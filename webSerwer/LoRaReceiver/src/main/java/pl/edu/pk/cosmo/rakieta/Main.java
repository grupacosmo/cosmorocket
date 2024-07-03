package pl.edu.pk.cosmo.rakieta;


import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fazecast.jSerialComm.SerialPort;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import pl.edu.pk.cosmo.rakieta.entity.SensorPacket;
import pl.edu.pk.cosmo.rakieta.service.EspRead;
import pl.edu.pk.cosmo.rakieta.service.FireBaseService;

import java.io.*;

import java.io.IOException;

public class Main {

    public static void main(String[] args) throws IOException {
        ObjectMapper objectMapper = new ObjectMapper();
        FireBaseService fireBaseService = new FireBaseService();
        FirebaseDatabase database = fireBaseService.getDb();
        try (EspRead data = new EspRead()) {
            data.choosePort();
            System.out.println("Port setup completed");
            DatabaseReference ref = database.getReference("LoRa");
            mainLoop(data, objectMapper, ref);
        } catch (Exception er) {
            System.err.println("Port setup failed");
            er.printStackTrace();
            System.exit(1);
        }
    }

    private static void mainLoop(EspRead data, ObjectMapper objectMapper, DatabaseReference ref) {
        while (true) {
            try {
                SensorPacket readdata = data.readdata();
                if (readdata != null){
                    readAndSend(readdata, objectMapper, ref);
                    writeToFile(readdata, objectMapper);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private static void readAndSend(SensorPacket sensorPacket, ObjectMapper objectMapper, DatabaseReference ref) throws IOException {
        String dataLoRaJson = objectMapper.writeValueAsString(sensorPacket);
        ref.setValue(dataLoRaJson, (databaseError, databaseReference) -> {
            if (databaseError != null) {
                System.out.println("Data could not be saved. " + databaseError.getMessage());
            } else {
                System.out.println("Data saved successfully.");
            }
        });
    }
    private static void writeToFile(SensorPacket data, ObjectMapper objectMapper) throws IOException {
        String saveFile = objectMapper.writeValueAsString(data);
        String fileName = java.time.LocalDate.now().toString() + ".txt";
        File file = new File(fileName);
        if (file.exists() && !file.isDirectory()) {
            FileWriter fw = new FileWriter(fileName, true);
            fw.write("\n");
            fw.write(saveFile);
            fw.close();
        } else {
            FileWriter fw = new FileWriter(fileName);
            fw.write(saveFile);
            fw.close();
        }
    }
}

