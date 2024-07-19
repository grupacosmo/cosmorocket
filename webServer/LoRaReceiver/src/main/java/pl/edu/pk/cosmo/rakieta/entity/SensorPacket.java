package pl.edu.pk.cosmo.rakieta.entity;

import lombok.Setter;

@Setter
public class SensorPacket {
    public int n;
    public long sys_time;
    public SensorPacketBME bme;
    public SensorPacketGPS gps;
    public SensorPacketMPU mpu;
    public int status;
    public String errors;
}
