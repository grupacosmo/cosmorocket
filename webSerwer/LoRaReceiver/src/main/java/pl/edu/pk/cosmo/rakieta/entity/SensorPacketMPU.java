package pl.edu.pk.cosmo.rakieta.entity;

import lombok.Setter;

@Setter
public class SensorPacketMPU {
    public Float3 acceleration;
    public Float3 rotation;
    public Float3 angularVelocity;
}
