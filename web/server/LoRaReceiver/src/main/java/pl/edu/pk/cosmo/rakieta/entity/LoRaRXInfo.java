package pl.edu.pk.cosmo.rakieta.entity;

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
public class LoRaRXInfo {

    private int length;
    private int rssi;
    private int snr;

}
