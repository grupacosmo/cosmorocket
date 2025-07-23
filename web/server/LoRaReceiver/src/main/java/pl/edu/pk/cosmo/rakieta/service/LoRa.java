package pl.edu.pk.cosmo.rakieta.service;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.List;
import java.util.Objects;
import java.util.Scanner;

import com.fazecast.jSerialComm.SerialPort;

import lombok.Getter;
import pl.edu.pk.cosmo.rakieta.LoRaException;

public class LoRa implements Closeable {

    public static final int LORA_BOUND_RATE = 9600;
    private BufferedReader serialInput;
    private OutputStreamWriter serialOutput;
    private SerialPort port;

    @Getter
    private String name;

    public LoRa(SerialPort port) {

        this.port = port;
        name = port.getSystemPortPath();

    }

    public void setupPort() throws IOException {

        Objects.requireNonNull(port);
        setupPort(port);

    }

    public void setupPort(SerialPort port) throws IOException {

        configurePort(port);
        openPort(port);

        handshake();

    }

    public void choosePort(Scanner scanner) throws IOException {

        SerialPort[] ports = SerialPort.getCommPorts();

        System.out.println("Select a port: ");

        for(int i = 0; i < ports.length; i++) {

            SerialPort port = ports[i];
            System.out.println(i + 1 + ": " + port.getDescriptivePortName());

        }

        SerialPort serialPort;

        int chosenPort = 0;

        do {

            chosenPort = scanner.nextInt();

        } while(chosenPort < 1 || chosenPort > ports.length);

        serialPort = ports[chosenPort - 1];

        scanner.nextLine();

        setupPort(serialPort);

    }

    private void configurePort(SerialPort serialPort) {

        serialPort.clearDTR();
        serialPort.clearRTS();
        serialPort.setNumDataBits(8);
        serialPort.setNumStopBits(SerialPort.ONE_STOP_BIT);
        serialPort.setParity(SerialPort.NO_PARITY);
        serialPort.setBaudRate(LORA_BOUND_RATE);
        serialPort.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER, 0, 0);

    }

    public void openPort(SerialPort serialPort) {

        if(!serialPort.openPort()) {

            final String UNABLE_TO_OPEN_THE_PORT = "Unable to open the port.";

            System.err.println(UNABLE_TO_OPEN_THE_PORT);
            throw new LoRaException(UNABLE_TO_OPEN_THE_PORT);

        }

        openSerialInput(port);
        openSerialOutput(port);

    }

    public void openSerialOutput() {

        openSerialOutput(port);

    }

    public void openSerialInput() {

        openSerialInput(port);

    }

    public void openSerialOutput(SerialPort serialPort) {

        serialOutput = new OutputStreamWriter(serialPort.getOutputStream());

    }

    public void openSerialInput(SerialPort serialPort) {

        serialInput = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));

    }

    public void closeSerialInput() throws IOException {

        serialInput.close();
        serialInput = null;

    }

    public void closeSerialOutput() throws IOException {

        serialOutput.close();
        serialOutput = null;

    }

    private void handshake() throws IOException {

        final String msg = "LORA SETUP";

        sendCommandAndWait("AT+MODE=TEST", "+MODE: TEST", msg);

        sendCommandAndWait("AT+TEST=RFCFG,868,SF7,250,12,15,14,ON,OFF,OFF", "+TEST", msg);

        sendCommandAndWait("AT+TEST=RXLRPKT", "+TEST", msg);

    }

    private void sendCommandAndWait(String command, String expectedResponse, String info) throws IOException {

        sendCommand(command);
        waitForOK(expectedResponse, info);

    }

    private void sendCommandAndWait(String command, List<String> expectedResponses, String info) throws IOException {

        sendCommand(command);
        waitForOK(expectedResponses, info);

    }

    public void sendString(String line) throws IOException {

        sendCommandAndWait("AT+TEST=TXLRSTR," + line, List.of("+TEST: TXLRSTR", "+TEST: TX DONE"), "sendString");

    }

    private void sendCommand(String command) throws IOException {

        serialOutput.write(command + "\n");
        serialOutput.flush();

    }

    private void waitForOK(String expectedResponse, String info) throws IOException {

        waitForOK(List.of(expectedResponse), info);

    }

    private void waitForOK(List<String> expectedResponses, String info) throws IOException {

        for(String expectedResponse : expectedResponses) {

            String message = serialInput.readLine();
            System.out.println("[DEBUG][LoraResponse] " + message);

            if(!message.startsWith(expectedResponse)) {

                throw new LoRaException("Not recived '" + expectedResponse + "' for " + info);

            }

        }

    }

    public String read() throws IOException {

        return serialInput.readLine();

    }

    public void close() throws IOException {

        serialInput.close();

    }

}
