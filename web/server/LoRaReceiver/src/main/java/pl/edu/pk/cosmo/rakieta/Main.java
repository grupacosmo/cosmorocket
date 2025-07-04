package pl.edu.pk.cosmo.rakieta;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import pl.edu.pk.cosmo.rakieta.entity.InfoWithPacket;
import pl.edu.pk.cosmo.rakieta.entity.SensorPacket;
import pl.edu.pk.cosmo.rakieta.service.EspRead;
import pl.edu.pk.cosmo.rakieta.service.FireBaseService;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class Main {

    private static final ObjectMapper mapper = new ObjectMapper();
    private static DatabaseReference reference;

    public static void main(String[] args) throws IOException {

        FireBaseService fireBaseService = new FireBaseService();
        FirebaseDatabase database = fireBaseService.getDb();
        reference = database.getReference("LoRa");

        try(EspRead lora1 = new EspRead(); EspRead lora2 = new EspRead()) {

            lora1.choosePort();
            lora2.choosePort();
            System.out.println("Port setup completed");
            mainLoop(lora1, lora2);

        } catch(Exception e) {

            System.err.println("Port setup failed");
            e.printStackTrace();
            System.exit(1);

        }

    }

    private static void mainLoop(EspRead lora1, EspRead lora2) {

        while(true) {

            try {

                InfoWithPacket readData1 = lora1.readdata();
                InfoWithPacket readData2 = lora2.readdata();

                if(readData1 != null && readData2 != null) {

                    SensorPacket packet = (readData1.getInfo().getRssi() < readData2.getInfo()
                        .getRssi() ? readData1 : readData2).getPacket();

                    readAndSend(packet);
                    writeToFile(packet);

                }

            } catch(Exception e) {

                e.printStackTrace();

            }

        }

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

    private static void writeToFile(SensorPacket data) throws IOException {

        String csvToSave = mapper.writeValueAsString(data);
        String fileName = java.time.LocalDate.now().toString() + ".txt";
        File file = new File(fileName);

        try(FileWriter fw = (file.exists() && !file.isDirectory()) ?
                new FileWriter(file, true) :
                new FileWriter(file)) {

            fw.write(csvToSave);
            fw.write("\n");

        }

    }

}
