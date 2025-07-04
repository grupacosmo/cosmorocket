package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.dataformat.csv.CsvFactory;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fasterxml.jackson.dataformat.csv.CsvSchema;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

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
            1625158800L,
            new SensorPacket.BMP(
                25.5f,
                60.0f,
                1013.25f
            ),
            new SensorPacket.GPS(
                new SensorPacket.GPS.Time(18, 00, 25),
                37.7749f,
                -122.4194f
            ),
            new SensorPacket.MPU(
                10,
                7,
                10,
                7,
                35
            ),
            3
        );

        CsvMapper mapper = new CsvMapper();

        CsvSchema schema = mapper.schemaFor(SensorPacket.class).withoutHeader();

        String csv = mapper.writerFor(SensorPacket.class).with(schema).writeValueAsString(packet);

        System.out.println(csv);

        SensorPacket resultSensorPacket = mapper.readerFor(SensorPacket.class).with(schema)
            .readValue(csv, SensorPacket.class);

        assertEquals(resultSensorPacket, packet);

    }

    @Test
    void dataBaseSaveTest() throws IOException, InterruptedException, ExecutionException {

        SensorPacket packet = new SensorPacket(
            1,
            1625158800L,
            new SensorPacket.BMP(
                25.5f,
                60.0f,
                1013.25f
            ),
            new SensorPacket.GPS(
                new SensorPacket.GPS.Time(18, 00, 25),
                37.7749f,
                -122.4194f
            ),
            new SensorPacket.MPU(
                10,
                7,
                10,
                7,
                35
            ),
            3
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
