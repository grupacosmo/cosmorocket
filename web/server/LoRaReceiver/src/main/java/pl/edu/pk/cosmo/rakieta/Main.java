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
import pl.edu.pk.cosmo.rakieta.entity.SensorPacket;
import pl.edu.pk.cosmo.rakieta.service.LoRa;
import pl.edu.pk.cosmo.rakieta.service.FireBaseService;

import java.io.File;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Scanner;
import java.util.regex.Pattern;

@Command(name = "LoRaReceiver", version = "1.0")
public class Main implements Runnable {

    private DatabaseReference reference;
    private Pattern portsPattern = Pattern.compile("^\\d+(\\s*,\\s*\\d+)*+$");
    private Pattern splitPattern = Pattern.compile("\\s*?,");
    @Parameters(index = "0", description = "Firebase database url")
    private String url = "";
    private String folderName;
    @Option(names = { "-o", "--output_location" })
    private List<String> outputFolders = List.of(".");
    @Option(names = { "-r", "--raw_output_location" })
    private List<String> rawOutputFolders = List.of(".");
    @Option(names = { "-c", "--credentials" })
    private String sdkCredentials = "firebase-credentials.json";
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
            folderName = receiptName.replace(':', '-');

            reference = database.getReference(receiptName);

        } catch(IOException e) {

            e.printStackTrace();
            System.exit(1);

        }

        List<SerialPort> ports = detectPorts();

        List<LoRa> loras = ports.stream().map(LoRa::new).toList();

        List<SensorPacket> history = Collections.synchronizedList(new RingBuffer<>(10));

        List<LoRaReceiver> receivers = loras.stream().map(lora -> new LoRaReceiver(lora, reference, history)).toList();

        try {

			mainLoop(receivers);

		} catch (IOException e) {

			e.printStackTrace();
            System.exit(1);

		}
    }

    private void mainLoop(List<LoRaReceiver> receivers) throws IOException {

        for(LoRaReceiver receiver : receivers) {
            receiver.setup(folderName, outputFolders, rawOutputFolders);
            receiver.start();
        }

        LoRaReceiver mainReceiver = receivers.get(0);

        while(scanner.hasNextLine()) {

            String line = scanner.nextLine().trim();
            if (line.isEmpty()) continue;
            System.out.println("Console input: " + line);
            if (line.equals("_ CLOSE RECEIVER")) {
                System.out.println("Graceful shutdown...");

                receivers.forEach(receiver -> {
                    receiver.interrupt();
                    try {
                        receiver.getLora().close();
                    }catch(IOException e) {
                        e.printStackTrace();
                    }
                });
                break;
            }
            else {
                System.out.println("unknown command");
            }
//            mainReceiver.getLora().closeSerialInput();
//            mainReceiver.interrupt();
//            mainReceiver.getLora().openSerialInput();
//            mainReceiver.start();
//            System.out.println("restarted");

        }

        System.out.println("Exiting LoRaReceiver... (joining threads)");
        receivers.forEach(receiver -> {
            try {
                receiver.join();
            } catch(InterruptedException e) {
                if (Thread.interrupted()) {
                    receivers.forEach(Thread::stop);
                }
                e.printStackTrace();
            }
        });
    }

    public static void main(String[] args) {

        if(args.length == 0) {

            CommandLine.usage(Main.class, System.err);
            return;

        }

        System.exit(new CommandLine(Main.class).execute(args));

    }

    private List<SerialPort> detectPorts() {

        List<SerialPort> ports = detectLoRa();

        if(ports.isEmpty()) {

            System.err.println("Couldn't detect LoRa's, trying different method.");

            ports = detectUdevLoRa();

        }

        if(ports.isEmpty()) {

            System.err.println("Couldn't detect LoRa's, manual input required!");

            ports = manualPortSelection(scanner);

        }

        if(ports.isEmpty()) {

            System.err.println("Ports not selected!");
            System.exit(1);

        }

        return ports;

    }

    private List<SerialPort> detectLoRa() {

        return Arrays.stream(SerialPort.getCommPorts())
            .filter(e -> e.getVendorID() == 0x10c4 && e.getProductID() == 0xea60).toList();

    }

    private List<SerialPort> detectUdevLoRa() {

        File lorasLocation = new File("/dev/cosmolora");

        if(!lorasLocation.exists() && !lorasLocation.isDirectory()) return List.of();

        return Arrays.stream(lorasLocation.listFiles()).map(File::getAbsolutePath)
            .filter(path -> path.contains("cosmolora/LORA")).map(SerialPort::getCommPort).toList();

    }

    private List<SerialPort> manualPortSelection(Scanner scanner) {

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


}
