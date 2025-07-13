package pl.edu.pk.cosmo.rakieta;

import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fazecast.jSerialComm.SerialPort;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

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
import java.util.List;
import java.util.Scanner;
import java.util.regex.Pattern;

public class Main {

    private static final ObjectWriter csvWriter = new CsvMapper().writerFor(SensorPacket.class).with(SensorPacket.SCHEMA);
    private static DatabaseReference reference;
    private static Pattern portsPattern = Pattern.compile("^\\d+(\\s*,\\s*\\d+)*+$");
    private static Pattern splitPattern = Pattern.compile("\\s*?,");
    private static FileWriter fileWriter;

    public static void main(String[] args) throws IOException {

        FireBaseService fireBaseService = new FireBaseService();
        FirebaseDatabase database = fireBaseService.getDb();

        String fileName = "LoRa-" + LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyy-MM-dd'T'HH:mm:ss"));

        reference = database.getReference(fileName);

        File file = new File(fileName.replace(':', '-') + ".txt");

        if(file.exists()) {

            fileWriter = new FileWriter(file, true);

        } else {

            fileWriter = new FileWriter(file);
            fileWriter.write(SensorPacket.SCHEMA.getColumnNames().stream().reduce((a, b) -> a + "," + b).orElse(""));
            fileWriter.write('\n');
            fileWriter.flush();

        }

        try(Scanner scanner = new Scanner(System.in)) {

            List<SerialPort> ports = choosePorts(scanner);

            List<LoRa> loras = new ArrayList<>(ports.size());

            for(SerialPort port : ports) {

                LoRa lora = new LoRa();
                lora.choosePort(port);
                loras.add(lora);

            }

            mainLoop(loras);

        } catch(Exception e) {

            System.err.println("Port setup failed");
            e.printStackTrace();
            System.exit(1);

        }

    }

    private static List<SerialPort> choosePorts(Scanner scanner) {

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

            return Arrays.stream(splitPattern.split(line))
                .map(Integer::parseInt)
                .map(e -> ports[e - 1])
                .toList();

        }

    }

    private static void mainLoop(List<LoRa> loras) {

        List<Thread> threads = loras.stream().map(lora -> new Thread(() -> {

            System.out.println("Lora running");

            while(true) {

                try {

                    SensorPacket data = lora.readData().getPacket();

                    readAndSend(data);
                    writeToFile(data);

                } catch(Exception e) {

                    e.printStackTrace();

                }

            }

        }))
            .toList();

        threads.forEach(Thread::start);

        threads.forEach(t -> {

            try {

                t.join();

            } catch(InterruptedException e) {

                t.interrupt();
                e.printStackTrace();

            }

        });

    }

    private static void readAndSend(SensorPacket sensorPacket) {

        reference.push().setValue(sensorPacket, (databaseError, databaseReference) -> {

            if(databaseError != null) {

                System.out.println("Data could not be saved. " + databaseError.getMessage());

            } else {

                System.out.println("Data saved successfully.");

            }

        });

    }

    private static synchronized void writeToFile(SensorPacket data) throws IOException {

        String csvToSave = csvWriter.writeValueAsString(data);

        fileWriter.write(csvToSave);
        fileWriter.flush();

    }

}
