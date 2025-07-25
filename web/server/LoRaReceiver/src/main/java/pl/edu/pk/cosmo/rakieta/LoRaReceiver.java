package pl.edu.pk.cosmo.rakieta;

import java.io.*;
import java.nio.file.Paths;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;
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
    private final List<SensorPacket> history;
    private List<Writer> outputFileWriters;
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
                .map(folder -> new File(Paths.get(folder, folderName).toString(), lora.getName().substring(1).replace('/', '-') + ".log")).toList();

        List<File> rawOutputFiles = rawOutputFolders.stream()
                .map(folder -> new File(Paths.get(folder, folderName).toString(), lora.getName().substring(1).replace('/', '-') + "_raw.log")).toList();

        Stream.concat(outputFiles.stream(), rawOutputFiles.stream()).forEach(file -> file.getParentFile().mkdirs());

        outputFileWriters = outputFiles.stream().map(this::createWriter).collect(Collectors.toCollection(ArrayList::new));
//        outputFileWriters.add(new OutputStreamWriter(System.out));
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
        final int MAX_EXCEPTION_IN_ROW_BEFORE_CRASH = 20;
        final int REFRESH_DELAY = 1000;

        long lastRefresh = 0;
        int packagesSinceLastRefresh = 0;

        int exceptionInRowCounter = 0;
        clearScreen();
        while(!interrupted()) {
            if (exceptionInRowCounter > MAX_EXCEPTION_IN_ROW_BEFORE_CRASH) {
                return;
            }


            printHidden(".", false);
            try {
                String info = lora.read();
                String data = lora.read();

                // end of stream
                if (info == null || data == null) {
                    break;
                }

                for(FileWriter writer : rawOutputFileWriters) {
                    writeToFile(writer, List.of(info, data));
                }

                boolean refreshRequest = lastRefresh + REFRESH_DELAY < System.currentTimeMillis();
                if(refreshRequest) {
                    lastRefresh = System.currentTimeMillis();

                    moveCursorToPrintArea();
                    clearScreenFromCursor();


                    moveCursorToDataArea();
                    System.out.print("\033[35m");
                    System.out.print(info);
                    System.out.print("\033[0m");

                    System.out.print("\033[1;50H");
                    System.out.print("p: " + packagesSinceLastRefresh);
                    System.out.println();

                    moveCursorToPrintArea();

                    packagesSinceLastRefresh = 0;
                }

//                else {
//                    printHidden("lr:" + lastRefresh  +"; lr+RD:" +  REFRESH_DELAY  + " ctm:" + System.currentTimeMillis());
//                }


                InfoWithPacket infoPacket = parseData(List.of(info, data), refreshRequest);

                SensorPacket packet = infoPacket.getPacket();
                if (packet == null) {
                    if (refreshRequest) printHidden("packet is null (check exceptions above)");
                    // exceptionInRowCounter = 0; // ignore?
                    continue;
                }

                if (refreshRequest) {
                    printColoredPackage(packet);
                    printHidden("hex:" + data);
                }

                synchronized (history) {
                    if (!history.contains(packet)) {
                        history.add(packet);
                        saveToDatabase(reference, packet);
                    }
                }

                for(Writer writer : outputFileWriters) {
                    writeToFile(writer, packet);
                }

                packagesSinceLastRefresh++;
                exceptionInRowCounter = 0;
            } catch (Exception e) {
                exceptionInRowCounter++;

                e.printStackTrace();
            }

        }

        System.out.println("interrupted");

    }

    private void printColoredPackage(SensorPacket packet) {
        moveCursorToDataArea();
        System.out.println();
        clearScreenFromCursor();
        System.out.print("\033[32m");
        System.out.print(packet.getN());
        System.out.print("\033[0m");
        System.out.print(" | ");
        System.out.print("\033[32m");
        System.out.printf("%02d:%02d:%02d", packet.getGps().getTime().getHours(), packet.getGps().getTime().getMinutes(), packet.getGps().getTime().getSeconds());
        System.out.print("\033[0m");
        System.out.print(" | ");
        System.out.print("\033[93m");
        System.out.print(packet.getStatus());
        System.out.print("\033[0m");
        System.out.println();
//        System.out.println(" | ");
        System.out.print("\033[92m");
        System.out.printf("tep: % 5.2f   pre: %6.0f    alt: % 7.3f", packet.getBmp().getTemperature(), packet.getBmp().getPressure(), packet.getBmp().getAltitude());
        System.out.print("\033[0m");

        System.out.println();
//        System.out.print(" | ");
        System.out.print("\033[95m");
        Vector3<Double> accMax = packet.getMpu().getAccelerationMax();
        Vector3<Double> accAvg = packet.getMpu().getAccelerationAverage();
        System.out.printf("ACC max:   % 8.3f % 8.3f % 8.3f    avg:  % 8.3f % 8.3f % 8.3f",
                accMax.x, accMax.y, accMax.z,  accAvg.x, accAvg.y, accAvg.z);

        System.out.print("\033[0m");
        System.out.println();
//        System.out.print(" | ");
        System.out.print("\033[95m");
        Vector3<Double> gyrMax = packet.getMpu().getGyroscopeMax();
        Vector3<Double> gyrAvg = packet.getMpu().getGyroscopeAverage();
        System.out.printf("GYRO max:  % 8.3f % 8.3f % 8.3f    avg:  % 8.3f % 8.3f % 8.3f",
                gyrMax.x, gyrMax.y, gyrMax.z, gyrAvg.x, gyrAvg.y, gyrAvg.z);

        System.out.print("\033[0m");
        System.out.println();
//        System.out.print(" | ");
        System.out.print("\033[95m");
        Vector4<Double> rotQuaternion = packet.getMpu().getRotationAverage();
        Vector3<Double> rotEuler = toEulerAngles(rotQuaternion);
        System.out.printf("ROT avg:   % 3d % 3d % 3d    (% .3f % .3f % .3f % .3f)",
                rotEuler.x.intValue(), rotEuler.y.intValue(), rotEuler.z.intValue(),
                rotQuaternion.x, rotQuaternion.y, rotQuaternion.z, rotQuaternion.w);
        System.out.print("\033[0m");
        System.out.println();

        System.out.print("\033[95m");

        System.out.print(MessageFormat.format("lat: {0} lon: {1}",
                packet.getGps().getLatitude(), packet.getGps().getLongitude()));
        System.out.print("\033[0m");
        System.out.println();
        // horizontal line  -  \e[47m - white bg  -  \e[2K - clear entire line
        System.out.println("\033[47m\033[2K");
        System.out.print("\033[0m");

        moveCursorToPrintArea();
    }

    private void moveCursorToDataArea() {
        System.out.print("\033[1;1H");
    }

    private void moveCursorToPrintArea() {
        System.out.print("\033[8;1H");
        System.out.println();
    }

    private void clearScreen() {
        System.out.print("\033[2J");
    }

    private void clearScreenFromCursor() {
        System.out.print("\033[0J");
    }


    private void saveToDatabase(DatabaseReference reference, SensorPacket sensorPacket) {

        reference.push().setValue(sensorPacket, (databaseError, _) -> {

            if (databaseError != null) {
                System.out.println("\033[97;41m");
                System.out.println("Data could not be saved. " + databaseError.getMessage());
                System.out.println("\033[0m");

            } else {

                //System.out.println("Packet nr " + sensorPacket.getN() + " saved successfully.");

            }

        });

    }

    private void writeToFile(Writer writer, List<String> data) throws IOException {

        for (String line : data) {

            writer.write(line);
            writer.write(System.lineSeparator());

        }

        writer.flush();

    }

    private void writeToFile(Writer writer, SensorPacket data) throws IOException {

        String csvToSave = csvWriter.writeValueAsString(data);

        writer.write(csvToSave);
        writer.flush();

    }

    public static InfoWithPacket parseData(List<String> data, boolean print) {

        String infoLine = data.get(0);
        String dataLine = data.get(1);

        if(infoLine == null || infoLine.equals("0") || !infoLine.startsWith("+TEST: LEN")) { return null; }
        if(dataLine == null || dataLine.equals("0") || !dataLine.startsWith("+TEST: RX")) { return null; }

        LoRaRXInfo info = parseInfo(infoLine);

        if(info == null) return null;

        String parsedData = parseRX(dataLine);
        if (print) printHidden("decoded hex: " + parsedData);

        SensorPacket packet = toPacket(parsedData);
        recalculatePackage(packet);

        return new InfoWithPacket(info, packet);

    }

    private static void recalculatePackage(SensorPacket packet) {
        if (packet == null) return;
        final double ACC_MAGIC_NUMBER = 0.000244140625;
        final double GYRO_MAGIC_NUMBER = 0.0152587890625;
        SensorPacket.MPU mpu = packet.getMpu();
        mpu.getAccelerationMax().x *= ACC_MAGIC_NUMBER;
        mpu.getAccelerationMax().y *= ACC_MAGIC_NUMBER;
        mpu.getAccelerationMax().z *= ACC_MAGIC_NUMBER;
        mpu.getAccelerationAverage().x *= ACC_MAGIC_NUMBER;
        mpu.getAccelerationAverage().y *= ACC_MAGIC_NUMBER;
        mpu.getAccelerationAverage().z *= ACC_MAGIC_NUMBER;

        mpu.getGyroscopeMax().x *= GYRO_MAGIC_NUMBER;
        mpu.getGyroscopeMax().y *= GYRO_MAGIC_NUMBER;
        mpu.getGyroscopeMax().z *= GYRO_MAGIC_NUMBER;
        mpu.getGyroscopeAverage().x *= GYRO_MAGIC_NUMBER;
        mpu.getGyroscopeAverage().y *= GYRO_MAGIC_NUMBER;
        mpu.getGyroscopeAverage().z *= GYRO_MAGIC_NUMBER;
    }

    private static void printHidden(String text) {
        printHidden(text, true);
    }
    private static void printHidden(String text, boolean nl) {
        System.out.print("\033[90m");
        System.out.print(text);
        System.out.print("\033[0m");
        if (nl) System.out.println();
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
            return reader.forType(SensorPacket.class).readValue(csv);
        }
        catch (JsonMappingException e) {
            printHidden("Invalid sensor packet (mapping): " + e.getMessage());
        }
        catch (JsonProcessingException e) {
            printHidden("Invalid sensor packet (processing): " + e.getMessage());
        }
        catch(Exception e) {
            printHidden("Exception '" + e.getClass() + "' occurred while parsing sensor packet: " + e.getMessage());
        }

        return null;

    }

    private static Vector3<Double> toEulerAngles(Vector4<Double> quaternion) {
        return toEulerAngles(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
    }
    private static Vector3<Double> toEulerAngles(double x, double y, double z, double w) {
        double roll, pitch, yaw;

        // Roll (x-axis rotation)
        double sinr_cosp = 2 * (w * x + y * z);
        double cosr_cosp = 1 - 2 * (x * x + y * y);
        roll = Math.atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        double sinp = 2 * (w * y - z * x);
        if (Math.abs(sinp) >= 1)
            pitch = Math.copySign(Math.PI / 2, sinp); // use 90 degrees if out of range
        else
            pitch = Math.asin(sinp);

        // Yaw (z-axis rotation)
        double siny_cosp = 2 * (w * z + x * y);
        double cosy_cosp = 1 - 2 * (y * y + z * z);
        yaw = Math.atan2(siny_cosp, cosy_cosp);

        return new Vector3<>(
                Math.toDegrees(roll),
                Math.toDegrees(pitch),
                Math.toDegrees(yaw));
    }

}
