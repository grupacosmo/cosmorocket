package pl.edu.pk.cosmo.rakieta.entity;

import java.util.List;

import com.fasterxml.jackson.annotation.JsonUnwrapped;
import com.fasterxml.jackson.dataformat.csv.CsvSchema;
import com.fasterxml.jackson.dataformat.csv.CsvSchema.ColumnType;

import lombok.AllArgsConstructor;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import pl.edu.pk.cosmo.rakieta.Vector3;
import pl.edu.pk.cosmo.rakieta.Vector4;

@Setter
@Getter
@NoArgsConstructor
@AllArgsConstructor
@EqualsAndHashCode
public class SensorPacket {

    public static final CsvSchema SCHEMA = CsvSchema.builder().addColumns(
        List.of(
            "n",
                "hours", "minutes", "seconds",
                "status",

                "temperature", "pressure", "altitude",

                "acceleration_max_x", "acceleration_max_y", "acceleration_max_z",
                "acceleration_average_x", "acceleration_average_y", "acceleration_average_z",

                "gyroscope_max_x", "gyroscope_max_y", "gyroscope_max_z",
                "gyroscope_average_x", "gyroscope_average_y", "gyroscope_average_z",

                "rotation_average_x", "rotation_average_y", "rotation_average_z", "rotation_average_w",

                "latitude", "longitude"
        ), ColumnType.NUMBER_OR_STRING
    ).build();

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
    public static class MPU {

        @JsonUnwrapped(prefix = "acceleration_max_")
        private Vector3<Double> accelerationMax;
        @JsonUnwrapped(prefix = "acceleration_average_")
        private Vector3<Double> accelerationAverage;

        @JsonUnwrapped(prefix = "gyroscope_max_")
        private Vector3<Double> gyroscopeMax;
        @JsonUnwrapped(prefix = "gyroscope_average_")
        private Vector3<Double> gyroscopeAverage;

        @JsonUnwrapped(prefix = "rotation_average_")
        private Vector4<Double> rotationAverage;

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

    private int n;
    private int status;
    @JsonUnwrapped
    private BMP bmp;
    @JsonUnwrapped
    private MPU mpu;
    @JsonUnwrapped
    private GPS gps;

}
