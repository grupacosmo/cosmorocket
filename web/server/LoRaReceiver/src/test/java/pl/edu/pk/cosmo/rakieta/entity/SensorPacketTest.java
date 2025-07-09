package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvFactory;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fasterxml.jackson.dataformat.csv.CsvSchema;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import pl.edu.pk.cosmo.rakieta.Vector3;
import pl.edu.pk.cosmo.rakieta.Vector4;
import pl.edu.pk.cosmo.rakieta.service.FireBaseService;

import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class SensorPacketTest {

    ObjectMapper objectMapper;
    ObjectWriter objectWriter;
    FireBaseService fireBaseService;

    @BeforeEach
    void setup() {

        CsvFactory jf = new CsvFactory();
        jf.configure(JsonGenerator.Feature.IGNORE_UNKNOWN, true);
        objectMapper = new ObjectMapper(jf);
        objectWriter = new ObjectMapper().writer().withDefaultPrettyPrinter();

    }

    @Test
    void assertEqualLocal() throws IOException {

        SensorPacket packet = new SensorPacket(
            1,
            3,
            new SensorPacket.BMP(
                25.5f,
                60.0f,
                1013.25f
            ),
            new SensorPacket.MPU(
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector4<>(
                    25,
                    43,
                    54,
                    12
                )
            ),
            new SensorPacket.GPS(
                new SensorPacket.GPS.Time(18, 00, 25),
                37.7749f,
                -122.4194f
            )
        );

        CsvMapper mapper = new CsvMapper();

        CsvSchema schema = mapper.schemaFor(SensorPacket.class).withoutHeader();

        String csv = mapper.writerFor(SensorPacket.class).with(schema).writeValueAsString(packet);

        SensorPacket resultSensorPacket = mapper.readerFor(SensorPacket.class).with(schema)
            .readValue(csv, SensorPacket.class);

        assertEquals(resultSensorPacket, packet);

    }

    @Test
    void csvHeaderTest() {

        String targetHeader = "n,hours,minutes,seconds,status,temperature,pressure,altitude,acceleration_max_x,acceleration_max_y,acceleration_max_z,acceleration_average_x,acceleration_average_y,acceleration_average_z,gyroscope_max_x,gyroscope_max_y,gyroscope_max_z,gyroscope_average_x,gyroscope_average_y,gyroscope_average_z,rotation_average_x,rotation_average_y,rotation_average_z,rotation_average_w,latitude,longitude";
        String header = SensorPacket.SCHEMA.getColumnNames().stream().reduce((a, b) -> a + "," + b).orElse("");

        assertEquals(targetHeader, header);

    }

    @Test
    void csvReadTest() throws JsonProcessingException {

        String csv = "1,18,00,25,3,25.5000,60.0000,1013.2500,25,43,54,25,43,54,25,43,54,25,43,54,25,43,54,12,37.7749,-122.4194";

        SensorPacket packet = new SensorPacket(
            1,
            3,
            new SensorPacket.BMP(
                25.5f,
                60.0f,
                1013.25f
            ),
            new SensorPacket.MPU(
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector4<>(
                    25,
                    43,
                    54,
                    12
                )
            ),
            new SensorPacket.GPS(
                new SensorPacket.GPS.Time(18, 00, 25),
                37.7749f,
                -122.4194f
            )
        );

        SensorPacket readPacket = new CsvMapper().readerFor(SensorPacket.class).with(SensorPacket.SCHEMA).readValue(csv);

        assertEquals(packet, readPacket);

    }

    @Test
    void dataBaseSaveTest() throws IOException, InterruptedException, ExecutionException {

        SensorPacket packet = new SensorPacket(
            1,
            3,
            new SensorPacket.BMP(
                25.5f,
                60.0f,
                1013.25f
            ),
            new SensorPacket.MPU(
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector3<>(
                    25,
                    43,
                    54
                ),
                new Vector4<>(
                    25,
                    43,
                    54,
                    12
                )
            ),
            new SensorPacket.GPS(
                new SensorPacket.GPS.Time(18, 00, 25),
                37.7749f,
                -122.4194f
            )
        );

        fireBaseService = new FireBaseService();
        FirebaseDatabase database = fireBaseService.getDb();
        DatabaseReference ref = database.getReference("LoRa-test");

        CompletableFuture<Void> result = new CompletableFuture<>();

        Runnable trySave = new Runnable() {

            @Override
            public void run() {

                ref.push().setValue(packet, (error, reference) -> {

                    if(error != null) {
                        System.out.println("Data could not be saved. " + error.getMessage());
                        run();
                        return;
                    }

                    System.out.println("Data saved succesfully.");
                    result.complete(null);

                });

            }

        };

        trySave.run();

        try {

            result.get(10, TimeUnit.SECONDS);

        } catch(TimeoutException e) {

            assertTrue(false);

        }

        assertTrue(true);

    }

}
