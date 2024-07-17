package pl.edu.pk.cosmo.rakieta.entity;

import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Setter
@AllArgsConstructor
@NoArgsConstructor
public class SensorPacketMPU {
    public Float3 mpu_avg;
    public Float3 mpu_max;
    public Float3 mpu_rot;
}
