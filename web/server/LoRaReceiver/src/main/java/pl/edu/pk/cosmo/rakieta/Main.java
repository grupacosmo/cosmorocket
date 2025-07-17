package pl.edu.pk.cosmo.rakieta;

import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fazecast.jSerialComm.SerialPort;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;
import picocli.CommandLine.Command;
import pl.edu.pk.cosmo.rakieta.entity.InfoWithPacket;
import pl.edu.pk.cosmo.rakieta.entity.SensorPacket;
import pl.edu.pk.cosmo.rakieta.service.LoRa;
import pl.edu.pk.cosmo.rakieta.service.FireBaseService;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Scanner;
import java.util.regex.Pattern;

@Command(name = "LoRaReceiver", version = "1.0")
public class Main implements Runnable {

    private final ObjectWriter csvWriter = new CsvMapper().writerFor(SensorPacket.class)
        .with(SensorPacket.SCHEMA);
    private DatabaseReference reference;
    private Pattern portsPattern = Pattern.compile("^\\d+(\\s*,\\s*\\d+)*+$");
    private Pattern splitPattern = Pattern.compile("\\s*?,");
    @Option(names = { "-o", "--output_location" })
    private String outputFileLocation = ".";
    @Option(names = { "-r", "--raw_output_location" })
    private String rawOutputFileLocation = ".";
    @Option(names = { "-c", "--credentials" })
    private String sdkCredentials = "firebase-credentials.json";
    @Parameters(index = "0")
    private String url = "";
    private String fileName;
    private static final Scanner scanner = new Scanner(System.in);

    @Override
    public void run() {

        File credentials = new File(sdkCredentials);

        if(!credentials.exists()) {

            System.err.println("You have to provide credentials file!");
            System.exit(1);

        }

        if(url.isEmpty()) {

            CommandLine.usage(Main.class, System.err);
            System.exit(1);

        }

        try {

            FireBaseService fireBaseService = new FireBaseService(url, credentials);
            FirebaseDatabase database = fireBaseService.getDb();

            String receiptName = "LoRa-" + LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss"));
            fileName = receiptName.replace(':', '-');

            reference = database.getReference(receiptName);

        } catch(IOException e) {

            e.printStackTrace();
            System.exit(1);

        }


        List<SerialPort> ports = choosePorts();

        if(ports.isEmpty()) {

            System.err.println("Couldn't detect LoRa's, trying different method.");

            ports = choosePortsls();

        }

        if(ports.isEmpty()) {

            System.err.println("Couldn't detect LoRa's, manual input required!");

            ports = choosePorts(scanner);


        }

        if(ports.isEmpty()) {

            System.err.println("Ports not selected!");
            System.exit(1);

        }

        try {

            List<LoRa> loras = new ArrayList<>();

            for(SerialPort port : ports) {

                loras.add(new LoRa(port));

            }

            mainLoop(loras);

        } catch(Exception e) {

            System.err.println("Port setup failed");
            e.printStackTrace();
            System.exit(2);

        }
    }

    public static void main(String[] args) {

        if(args.length == 0) {

            CommandLine.usage(Main.class, System.err);
            return;

        }

        int exitCode = new CommandLine(Main.class).execute(args);
        System.exit(exitCode);

    }

    private List<SerialPort> choosePorts() {

        return Arrays.stream(SerialPort.getCommPorts())
            .filter(e -> e.getVendorID() == 0x10c4 && e.getProductID() == 0xea60).toList();

    }

    private List<SerialPort> choosePortsls() {

        File lorasLocation = new File("/dev/cosmolora");

        if(!lorasLocation.exists() && !lorasLocation.isDirectory()) return List.of();

        return Arrays.stream(lorasLocation.listFiles()).map(File::getAbsolutePath)
            .filter(path -> path.contains("cosmolora/LORA")).map(SerialPort::getCommPort).toList();

    }

    private List<SerialPort> choosePorts(Scanner scanner) {

        SerialPort[] ports = SerialPort.getCommPorts();

        System.out.println("Select ports (ex. 1,3):");

        for(int i = 0; i < ports.length; i++) {

            SerialPort port = ports[i];

            System.out.println(i + 1 + ": " + port.getSystemPortPath() + " " + port.getDescriptivePortName());

        }

        for(;;) {

            System.out.print("Ports: ");

            String line = scanner.nextLine().trim();

            if(!portsPattern.matcher(line).matches()) continue;

            return Arrays.stream(splitPattern.split(line)).map(Integer::parseInt).map(e -> ports[e - 1]).toList();

        }

    }

    private void mainLoop(List<LoRa> loras) {

        List<SensorPacket> history = Collections.synchronizedList(new RingBuffer<>(10));

        List<Thread> threads = new ArrayList<>(loras.size());

        for(int i = 0; i < loras.size(); i++) {

            int index = i;

            threads.add(new Thread(() -> {

                LoRa lora = loras.get(index);

                File rawOutputFile = new File(rawOutputFileLocation, fileName + "=RAW_LORA_" + index + ".txt");

                if(!rawOutputFile.exists()) {

                    rawOutputFile.getParentFile().mkdirs();

                }

                File outputFile = new File(outputFileLocation, fileName + "=LORA_" + index + ".txt");

                if(!outputFile.exists()) {

                    outputFile.getParentFile().mkdirs();

                }

                try(FileWriter rawOutputFileWriter = new FileWriter(rawOutputFile, rawOutputFile.exists());
                    FileWriter outputFileWriter = new FileWriter(outputFile, outputFile.exists())) {

                    if(!outputFile.exists()) {

                        outputFileWriter.write(SensorPacket.SCHEMA.getColumnNames().stream().reduce((a, b) -> a + "," + b).orElse(""));
                        outputFileWriter.write('\n');
                        outputFileWriter.flush();

                    }

                    lora.choosePort();

                    while(true) {

                        try {

                            List<String> data = lora.readData();

                            writeToFile(rawOutputFileWriter, data);

                            InfoWithPacket infoPacket = LoRa.parseData(data);

                            SensorPacket packet = infoPacket.getPacket();

                            synchronized(history) {

                                if(!history.contains(packet)) {

                                    history.add(packet);

                                    readAndSend(packet);

                                }

                            }

                            writeToFile(outputFileWriter, packet);

                        } catch(Exception e) {

                            e.printStackTrace();

                        }

                    }

                } catch(IOException e) {

                    e.printStackTrace();

                }

            }));

        }


        threads.forEach(Thread::start);

        try {

            while(scanner.hasNextLine()) {

                String line = scanner.nextLine();
                loras.get(0).send(line);

            }

        } catch(IOException e) {

            e.printStackTrace();

        }

        threads.forEach(thread -> {

            try {

                thread.join();

            } catch(InterruptedException e) {

                thread.interrupt();
                e.printStackTrace();

            }

        });

    }

    private void readAndSend(SensorPacket sensorPacket) {

        reference.push().setValue(sensorPacket, (databaseError, _) -> {

            if(databaseError != null) {

                System.out.println("Data could not be saved. " + databaseError.getMessage());

            } else {

                System.out.println("Packet nr " + sensorPacket.getN() + " saved successfully.");

            }

        });

    }

    private void writeToFile(FileWriter fileWriter, List<String> data) throws IOException {

        for(String line : data) {

            fileWriter.write(line);
            fileWriter.write('\n');

        }

        fileWriter.flush();

    }

    private void writeToFile(FileWriter fileWriter, SensorPacket data) throws IOException {

        String csvToSave = csvWriter.writeValueAsString(data);

        fileWriter.write(csvToSave);
        fileWriter.flush();

    }

}
