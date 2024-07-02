package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.csv.CsvFactory;
import com.fasterxml.jackson.dataformat.csv.CsvMapper;
import com.fasterxml.jackson.dataformat.csv.CsvSchema;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class CsvSensorPacketTest {
    ObjectMapper objectMapper;
    @BeforeEach
    void setup(){
        CsvFactory jf = new CsvFactory();
        jf.configure(JsonGenerator.Feature.IGNORE_UNKNOWN, true);
        objectMapper = new ObjectMapper(jf);
    }
    @Test
    void test() throws Exception{
        Float3 acceleration = new Float3();
        acceleration.x = 0.1f;// Create an instance of CsvSensorPacket with hardcoded data
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
        packet.time = 1625158800L;
        packet.temperature = 25.5f;
        packet.humidity = 60.0f;
        packet.pressure = 1013.25f;
        packet.latitude = 37.7749f;
        packet.longitude = -122.4194f;
        packet.altitude = 30.0f;
        packet.accelerationX = 1;
        packet.accelerationY = 1;
        packet.accelerationZ = 1;
        packet.rotationX = 1;
        packet.rotationY = 1;
        packet.rotationZ = 1;
        packet.angularVelocityX = 1;
        packet.angularVelocityY = 1;
        packet.angularVelocityZ = 1;
        packet.relayFlags = 3;

        CsvSchema schema = new CsvSchema.Builder()
                .addColumn("time")
                .addColumn("temperature")
                .addColumn("humidity")
                .addColumn("pressure")
                .addColumn("latitude")
                .addColumn("longitude")
                .addColumn("altitude")
                .addColumn("accelerationX")
                .addColumn("accelerationY")
                .addColumn("accelerationZ")
                .addColumn("rotationX")
                .addColumn("rotationY")
                .addColumn("rotationZ")
                .addColumn("angularVelocityX")
                .addColumn("angularVelocityY")
                .addColumn("angularVelocityZ")
                .addColumn("relayFlags")
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
}