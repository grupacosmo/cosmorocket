package pl.edu.pk.cosmo.rakieta.entity;

import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Setter
@AllArgsConstructor
@NoArgsConstructor
public class SensorPacketGPS {
    public float gps_time;
    public float gps_latitude;
    public float gps_longitude;
    public float gps_altitude;
}
