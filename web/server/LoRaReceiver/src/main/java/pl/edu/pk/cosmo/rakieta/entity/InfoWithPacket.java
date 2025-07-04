package pl.edu.pk.cosmo.rakieta.entity;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;

@Setter
@Getter
@AllArgsConstructor
public class InfoWithPacket  {

    private LoRaRXInfo info;
    private SensorPacket packet;

}
