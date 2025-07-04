package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.annotation.JsonUnwrapped;

import lombok.AllArgsConstructor;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Setter
@Getter
@NoArgsConstructor
@AllArgsConstructor
@EqualsAndHashCode
public class SensorPacket {

    @Setter
    @Getter
    @AllArgsConstructor
    @NoArgsConstructor
    @EqualsAndHashCode
    public static class BMP {

        private float temperature;
        private float pressure;
        private float altitude;

    }

    @Setter
    @Getter
    @AllArgsConstructor
    @NoArgsConstructor
    @EqualsAndHashCode
    public static class GPS {

        @Setter
        @Getter
        @AllArgsConstructor
        @NoArgsConstructor
        @EqualsAndHashCode
        public static class Time {

            private int hours;
            private int minutes;
            private int seconds;

        }

        @JsonUnwrapped
        private Time time;
        private float latitude;
        private float longitude;

    }

    @Setter
    @Getter
    @AllArgsConstructor
    @NoArgsConstructor
    @EqualsAndHashCode
    public static class MPU {

        private int accelerationMax;
        private int accelerationAverage;

        private int gyroscopeMax;
        private int gyroscopeAverage;

        private int rotationAverage;

    }

    private int n;
    private long systemTime;
    @JsonUnwrapped
    private BMP bmp;
    @JsonUnwrapped
    private GPS gps;
    @JsonUnwrapped
    private MPU mpu;
    private int status;

}
