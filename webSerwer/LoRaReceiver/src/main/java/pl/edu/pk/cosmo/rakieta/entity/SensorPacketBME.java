package pl.edu.pk.cosmo.rakieta.entity;

import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Setter
@AllArgsConstructor
@NoArgsConstructor
public class SensorPacketBME {
    public float bmp_temp;
    public float bmp_humidity;
    public float bmp_pressure;
    public float bmp_altitude;
}
