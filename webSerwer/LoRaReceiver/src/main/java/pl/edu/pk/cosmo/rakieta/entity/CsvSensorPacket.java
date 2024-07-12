package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.annotation.JsonPropertyOrder;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

@EqualsAndHashCode
@Getter
@Setter
@JsonPropertyOrder({"time", "temperature", "humidity", "pressure", "latitude", "longitude", "altitude", "accelerationX", "accelerationY", "accelerationZ", "rotationX", "rotationY", "rotationZ", "angularVelocityX", "angularVelocityY", "angularVelocityZ", "relayFlags"})
public class CsvSensorPacket {
    public long time;
    public float temperature;
    public float humidity;
    public float pressure;
    public float latitude;
    public float longitude;
    public float altitude;
    public float accelerationX;
    public float accelerationY;
    public float accelerationZ;
    public float rotationX;
    public float rotationY;
    public float rotationZ;
    public float angularVelocityX;
    public float angularVelocityY;
    public float angularVelocityZ;
    public int relayFlags;
}
