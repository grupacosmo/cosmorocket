package pl.edu.pk.cosmo.rakieta.entity;

import lombok.Setter;

@Setter
public class SensorPacket {
    public long time;
    public SensorPacketBME bme;
    public SensorPacketGPS gps;
    public SensorPacketMPU mpu;
    public int relayFlags;
}
