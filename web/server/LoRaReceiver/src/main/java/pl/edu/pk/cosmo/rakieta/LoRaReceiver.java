package pl.edu.pk.cosmo.rakieta;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Paths;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Stream;

import com.fasterxml.jackson.databind.ObjectReader;
import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.google.firebase.database.DatabaseReference;

import lombok.Getter;
import pl.edu.pk.cosmo.rakieta.entity.InfoWithPacket;
import pl.edu.pk.cosmo.rakieta.entity.LoRaRXInfo;
import pl.edu.pk.cosmo.rakieta.entity.SensorPacket;
import pl.edu.pk.cosmo.rakieta.service.LoRa;

public class LoRaReceiver extends Thread {

    @Getter
    private LoRa lora;
    private DatabaseReference reference;
    private List<SensorPacket> history;
    private List<FileWriter> outputFileWriters;
    private List<FileWriter> rawOutputFileWriters;
    private static final ObjectWriter csvWriter = new CsvMapper().writerFor(SensorPacket.class).with(SensorPacket.SCHEMA);
    private static final Pattern infoPattern = Pattern.compile("LEN:(?<LEN>\\d*?), RSSI:(?<RSSI>-?\\d*?), SNR:(?<SNR>\\d*)");
    private static final ObjectReader reader = new CsvMapper().readerFor(SensorPacket.class).with(SensorPacket.SCHEMA);

    public LoRaReceiver(LoRa lora, DatabaseReference reference, List<SensorPacket> history) {

        this.lora = lora;
        this.reference = reference;
        this.history = history;

    }

    private FileWriter createWriter(File file) throws LoRaReceiverException {

        try {

            return new FileWriter(file);

        } catch (IOException e) {

            throw new LoRaReceiverException(e);

        }

    }

    public void setup(String folderName, List<String> outputFolders, List<String> rawOutputFolders)
            throws LoRaReceiverException, IOException {

        List<File> outputFiles = outputFolders.stream()
                .map(folder -> new File(Paths.get(folder, folderName).toString(), lora.getName().substring(1).replace('/', '-') + ".txt")).toList();

        List<File> rawOutputFiles = rawOutputFolders.stream()
                .map(folder -> new File(Paths.get(folder, folderName).toString(), lora.getName().substring(1).replace('/', '-') + ".txt")).toList();

        Stream.concat(outputFiles.stream(), rawOutputFiles.stream()).forEach(file -> file.getParentFile().mkdirs());

        outputFileWriters = outputFiles.stream().map(this::createWriter).toList();

        outputFileWriters.forEach(writer -> {

            try {

                writer.write(SensorPacket.SCHEMA.getColumnNames().stream().reduce((acc, e) -> acc + "," + e).orElse(""));
                writer.write(System.lineSeparator());
                writer.flush();

            } catch (IOException e) {

                throw new LoRaReceiverException(e);

            }

        });

        rawOutputFileWriters = rawOutputFiles.stream().map(this::createWriter).toList();

        lora.setupPort();

    }

    @Override
    public void run() {

        while(!interrupted()) {

            System.out.println("reading");

            try {

                String info = lora.read();
                String data = lora.read();

                for(FileWriter writer : rawOutputFileWriters) {

                    writeToFile(writer, List.of(info, data));

                }

                InfoWithPacket infoPacket = parseData(List.of(info, data));

                SensorPacket packet = infoPacket.getPacket();

                synchronized (history) {

                    if (!history.contains(packet)) {

                        history.add(packet);

                        saveToDatabase(reference, packet);

                    }

                }

                for(FileWriter writer : outputFileWriters) {

                    writeToFile(writer, packet);

                }

            } catch (Exception e) {

                e.printStackTrace();

            }

        }

        System.out.println("interrupted");

    }

    private void saveToDatabase(DatabaseReference reference, SensorPacket sensorPacket) {

        reference.push().setValue(sensorPacket, (databaseError, _) -> {

            if (databaseError != null) {

                System.out.println("Data could not be saved. " + databaseError.getMessage());

            } else {

                System.out.println("Packet nr " + sensorPacket.getN() + " saved successfully.");

            }

        });

    }

    private void writeToFile(FileWriter fileWriter, List<String> data) throws IOException {

        for (String line : data) {

            fileWriter.write(line);
            fileWriter.write(System.lineSeparator());

        }

        fileWriter.flush();

    }

    private void writeToFile(FileWriter fileWriter, SensorPacket data) throws IOException {

        String csvToSave = csvWriter.writeValueAsString(data);

        fileWriter.write(csvToSave);
        fileWriter.flush();

    }

    public static InfoWithPacket parseData(List<String> data) {

        String infoLine = data.get(0);
        String dataLine = data.get(1);

        if(infoLine == null || infoLine.equals("0") || !infoLine.startsWith("+TEST: LEN")) { return null; }
        if(dataLine == null || dataLine.equals("0") || !dataLine.startsWith("+TEST: RX")) { return null; }

        LoRaRXInfo info = parseInfo(infoLine);

        if(info == null) return null;

        String parsedData = parseRX(dataLine);

        return new InfoWithPacket(info, toPacket(parsedData));

    }

    private static LoRaRXInfo parseInfo(String line) {

        Matcher infoMatcher = infoPattern.matcher(line);

        if(!infoMatcher.find()) return null;

        return new LoRaRXInfo(
            Integer.parseInt(infoMatcher.group("LEN")),
            Integer.parseInt(infoMatcher.group("RSSI")),
            Integer.parseInt(infoMatcher.group("SNR"))
        );

    }

    private static String parseRX(String line) {

        String hex = line.split("\"")[1];
        byte[] data = new byte[hex.length() / 2];

        for(int i = 0; i < hex.length(); i += 2) {

            data[i / 2] = Byte.parseByte(hex.substring(i, i + 2), 16);

        }

        return new String(data);

    }

    private static SensorPacket toPacket(String csv) {

        try {

            return reader.readValue(csv, SensorPacket.class);

        } catch(Exception e) {

            e.printStackTrace(System.err);

        }

        return null;

    }

}
