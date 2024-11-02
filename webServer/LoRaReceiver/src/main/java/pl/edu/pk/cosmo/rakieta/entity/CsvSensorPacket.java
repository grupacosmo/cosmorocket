package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.annotation.JsonPropertyOrder;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

@EqualsAndHashCode
@Getter
@Setter
@JsonPropertyOrder({"n","sys_time",
        "bmp_temp", "bmp_humidity", "bmp_pressure","bmp_altitude",
        "gps_time", "gps_latitude", "gps_longitude", "gps_altitude",
        "mpu_avg_x", "mpu_avg_y", "mpu_avg_z",
        "mpu_max_x", "mpu_max_y", "mpu_max_z",
        "mpu_rot_x", "mpu_rot_y", "mpu_rot_z",
        "status", "errors"})
public class CsvSensorPacket {
    public int n;
    public long sys_time;
    public float bmp_temp;
    public float bmp_humidity;
    public float bmp_pressure;
    public float bmp_altitude;
    public float gps_time;
    public float gps_latitude;
    public float gps_longitude;
    public float gps_altitude;
    public float mpu_avg_x;
    public float mpu_avg_y;
    public float mpu_avg_z;
    public float mpu_max_x;
    public float mpu_max_y;
    public float mpu_max_z;
    public float mpu_rot_x;
    public float mpu_rot_y;
    public float mpu_rot_z;
    public int status;
    public String errors;
}
