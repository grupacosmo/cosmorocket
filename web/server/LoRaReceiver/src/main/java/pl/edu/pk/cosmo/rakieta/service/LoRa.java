package pl.edu.pk.cosmo.rakieta.service;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.List;
import java.util.Objects;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.fasterxml.jackson.databind.ObjectReader;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fazecast.jSerialComm.SerialPort;

import lombok.Getter;
import pl.edu.pk.cosmo.rakieta.LoRaException;
import pl.edu.pk.cosmo.rakieta.entity.*;

public class LoRa implements Closeable {

    public static final int LORA_BOUND_RATE = 9600;
    private static final Pattern infoPattern = Pattern.compile("LEN:(?<LEN>\\d*?), RSSI:(?<RSSI>-?\\d*?), SNR:(?<SNR>\\d*)");
    private BufferedReader serialInput;
    private OutputStreamWriter serialOutput;
    private static final ObjectReader reader = new CsvMapper().readerFor(SensorPacket.class).with(SensorPacket.SCHEMA);
    private SerialPort port;

    @Getter
    private String name;

    public LoRa(SerialPort port) {

        name = port.getSystemPortPath();
        this.port = port;

    }

    public void choosePort() throws IOException {

        Objects.requireNonNull(port);
        choosePort(port);

    }

    public void choosePort(SerialPort port) throws IOException {

        setupPort(port);

        serialInput = new BufferedReader(new InputStreamReader(port.getInputStream()));
        serialOutput = new OutputStreamWriter(port.getOutputStream());

        setupLora();

    }

    public void choosePort(Scanner s) throws IOException {

        SerialPort[] ports = SerialPort.getCommPorts();

        System.out.println("Select a port: ");

        for(int i = 0; i < ports.length; i++) {

            SerialPort port = ports[i];
            System.out.println(i + 1 + ": " + port.getDescriptivePortName());

        }

        SerialPort serialPort;

        int chosenPort = 0;

        do {

            chosenPort = s.nextInt();

        } while(chosenPort < 1 || chosenPort > ports.length);

        serialPort = ports[chosenPort - 1];

        s.nextLine();

        choosePort(serialPort);

    }

    private static void setupPort(SerialPort serialPort) {

        serialPort.clearDTR();
        serialPort.clearRTS();
        serialPort.setNumDataBits(8);
        serialPort.setNumStopBits(SerialPort.ONE_STOP_BIT);
        serialPort.setParity(SerialPort.NO_PARITY);
        serialPort.setBaudRate(LORA_BOUND_RATE);

        if(!serialPort.openPort()) {

            final String UNABLE_TO_OPEN_THE_PORT = "Unable to open the port.";

            System.err.println(UNABLE_TO_OPEN_THE_PORT);
            throw new LoRaException(UNABLE_TO_OPEN_THE_PORT);

        }

        System.out.println("Port opened successfully.");

        serialPort.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER, 0, 0);

    }

    private void setupLora() throws IOException {

        String msg = "LORA SETUP";
        System.out.println("lora setup");
        System.out.println("test mode");
        serialOutput.write("AT+MODE=TEST\r\n");
        serialOutput.flush();
        waitForOK("+MODE: TEST", msg);

        System.out.println("RFCFG");
        serialOutput.write("AT+TEST=RFCFG,868,SF12,125,12,15,14,ON,OFF,OFF\r\n");
        serialOutput.flush();
        waitForOK("+TEST", msg);

        System.out.println("RXLPRKT");
        serialOutput.write("AT+TEST=RXLRPKT\r\n");
        serialOutput.flush();
        waitForOK("+TEST", msg);

    }

    private void waitForOK(String expectedResponse, String info) throws IOException {

        String message = serialInput.readLine();
        System.out.println("[DEBUG][LoraResponse] " + message);

        if(!message.startsWith(expectedResponse)) {

            throw new LoRaException("Not recived '" + expectedResponse + "' for " + info);

        }

    }

    public List<String> readData() throws IOException {

        String infoLine = serialInput.readLine();
        String dataLine = serialInput.readLine();

        return List.of(infoLine, dataLine);

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

    public void close() throws IOException {

        serialInput.close();

    }

    private static SensorPacket toPacket(String split) {

        try {

            return reader.readValue(split, SensorPacket.class);

        } catch(Exception e) {

            e.printStackTrace(System.err);

        }

        return null;

    }

    public void send(String line) throws IOException {

        serialOutput.write("AT+TEST=TXLRSTR," + line + "\r\n");
        serialOutput.flush();

    }

}
