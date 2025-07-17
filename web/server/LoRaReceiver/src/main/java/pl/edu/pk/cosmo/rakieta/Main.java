package pl.edu.pk.cosmo.rakieta;

import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fazecast.jSerialComm.SerialPort;
import com.google.common.collect.Streams;
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
    private List<String> outputFileLocations = List.of(".");
    @Option(names = { "-r", "--raw_output_location" })
    private List<String> rawOutputFileLocations = List.of(".");
    @Option(names = { "-c", "--credentials" })
    private String sdkCredentials = "firebase-credentials.json";
    @Parameters(index = "0")
    private String url = "";
    private String filename;
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
            filename = receiptName.replace(':', '-');

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

                List<File> rawOutputFiles = rawOutputFileLocations.stream().map(location -> new File(location, filename + "=RAW_LORA_" + index + ".txt")).toList();

                rawOutputFiles.stream().filter(file -> !file.exists()).map(File::getParentFile).forEach(File::mkdirs);

                List<FileWriter> rawOutputFileWriters = rawOutputFiles.stream().map(file -> {

                    try {

                        return new FileWriter(file);

                    } catch(IOException e) {

                        e.printStackTrace();
                        Thread.currentThread().interrupt();

                    }

                    return null;

                })
                    .toList();

                List<File> outputFiles = outputFileLocations.stream().map(location -> new File(location, filename + "=LORA_" + index + ".txt")).toList();

                outputFiles.stream().filter(file -> !file.exists()).map(File::getParentFile).forEach(File::mkdirs);

                List<FileWriter> outputFileWriters = outputFiles.stream().map(file -> {

                    try {

                        return new FileWriter(file);

                    } catch(IOException e) {

                        e.printStackTrace();
                        Thread.currentThread().interrupt();

                    }

                    return null;

                })
                    .toList();

                Streams.zip(outputFiles.stream(), outputFileWriters.stream(), (file, writer) -> {

                    if(!file.exists()) return null;

                    try {

                        writer.write(SensorPacket.SCHEMA.getColumnNames().stream().reduce((a, b) -> a + "," + b).orElse(""));
                        writer.write(System.lineSeparator());
                        writer.flush();

                    } catch(IOException e) {

                        e.printStackTrace();
                        Thread.currentThread().interrupt();

                    }

                    return null;

                })
                    .close();

                try {

                    lora.choosePort();

                } catch(IOException e) {

                    e.printStackTrace();
                    return;

                }

                while(true) {

                    try {

                        List<String> data = lora.readData();

                        rawOutputFileWriters.forEach(writer -> {

                            try {

                                writeToFile(writer, data);

                            } catch(IOException e) {

                                e.printStackTrace();

                            }

                        });

                        InfoWithPacket infoPacket = LoRa.parseData(data);

                        SensorPacket packet = infoPacket.getPacket();

                        synchronized(history) {

                            if(!history.contains(packet)) {

                                history.add(packet);

                                readAndSend(packet);

                            }

                        }

                        outputFileWriters.forEach(writer -> {

                            try {

                                writeToFile(writer, packet);

                            } catch(IOException e) {

                                e.printStackTrace();

                            }

                        });

                    } catch(Exception e) {

                        e.printStackTrace();

                    }

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
