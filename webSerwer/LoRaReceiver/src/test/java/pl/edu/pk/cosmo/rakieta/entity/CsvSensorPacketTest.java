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

import static org.junit.jupiter.api.Assertions.*;

class CsvSensorPacketTest {
    ObjectMapper objectMapper;
    ObjectWriter objectWriter;
    FireBaseService fireBaseService;

    @BeforeEach
    void setup() throws IOException {
        CsvFactory jf = new CsvFactory();
        jf.configure(JsonGenerator.Feature.IGNORE_UNKNOWN, true);
        objectMapper = new ObjectMapper(jf);
        objectWriter = new ObjectMapper().writer().withDefaultPrettyPrinter();
    }

    @Test
    void assertEqualLocal() throws Exception {
        Float3 acceleration = new Float3();
        acceleration.x = 0.1f;
        acceleration.y = 0.2f;
        acceleration.z = 0.3f;

        Float3 rotation = new Float3();
        rotation.x = 1.1f;
        rotation.y = 1.2f;
        rotation.z = 1.3f;

        Float3 angularVelocity = new Float3();
        angularVelocity.x = 2.1f;
        angularVelocity.y = 2.2f;
        angularVelocity.z = 2.3f;

        CsvSensorPacket packet = new CsvSensorPacket();
        packet.n = 1;
        packet.sys_time = 1625158800L;
        packet.bmp_temp = 25.5f;
        packet.bmp_humidity = 60.0f;
        packet.bmp_pressure = 1013.25f;
        packet.bmp_altitude = 39.43f;
        packet.gps_latitude = 37.7749f;
        packet.gps_longitude = -122.4194f;
        packet.gps_altitude = 30.0f;
        packet.mpu_avg_x = 1;
        packet.mpu_avg_y = 1;
        packet.mpu_avg_z = 1;
        packet.mpu_max_x = 1;
        packet.mpu_max_y = 1;
        packet.mpu_max_z = 1;
        packet.mpu_rot_x = 1;
        packet.mpu_rot_y = 1;
        packet.mpu_rot_z = 1;
        packet.status = 3;
        packet.errors = "Something went wrong";
        CsvSchema schema = new CsvSchema.Builder()
                .addColumn("n")
                .addColumn("sys_time")
                .addColumn("bmp_temp")
                .addColumn("bmp_humidity")
                .addColumn("bmp_pressure")
                .addColumn("bmp_altitude")
                .addColumn("gps_time")
                .addColumn("gps_latitude")
                .addColumn("gps_longitude")
                .addColumn("gps_altitude")
                .addColumn("mpu_avg_x")
                .addColumn("mpu_avg_y")
                .addColumn("mpu_avg_z")
                .addColumn("mpu_max_x")
                .addColumn("mpu_max_y")
                .addColumn("mpu_max_z")
                .addColumn("mpu_rot_x")
                .addColumn("mpu_rot_y")
                .addColumn("mpu_rot_z")
                .addColumn("status")
                .addColumn("errors")
                .build();

        CsvMapper mapper = new CsvMapper();
        String s1 = mapper.writerFor(CsvSensorPacket.class)
                .with(schema)
                .writeValueAsString(packet);
        System.out.println(s1);
        CsvSensorPacket csvSensorPacket1 = mapper.readerFor(CsvSensorPacket.class)
                .with(schema)
                .readValue(s1, CsvSensorPacket.class);

        assertEquals(csvSensorPacket1, packet);

    }

    @Test
    void dataBaseSaveTest() throws IOException {
        SensorPacket packet = new SensorPacket();
        packet.setN(2);
        packet.setSys_time(1625158800L);
        packet.setBme(new SensorPacketBME(
                25.5f,
                60.0f,
                1013.25f,
                39.43f
        ));
        packet.setGps(new SensorPacketGPS(
                37.7749f,
                -122.4194f,
                30.0f,
                46.5f

        ));

        packet.setMpu(new SensorPacketMPU(
                new Float3(1, 1, 1),
                new Float3(1, 1, 1),
                new Float3(1, 1, 1)
        ));
        packet.setStatus(3);
        packet.setErrors("Something went wrong");
        fireBaseService = new FireBaseService();
        FirebaseDatabase database = fireBaseService.getDb();
        DatabaseReference ref = database.getReference("LoRa");
        String dataLoRaJson = objectWriter.writeValueAsString(packet);
        while (true) {
            ref.setValue(packet, (databaseError, databaseReference) -> {
                if (databaseError != null) {
                    System.out.println("Data could not be saved. " + databaseError.getMessage());
                } else {
                    System.out.println("Data saved successfully.");
                }
            });
        }

    }
}