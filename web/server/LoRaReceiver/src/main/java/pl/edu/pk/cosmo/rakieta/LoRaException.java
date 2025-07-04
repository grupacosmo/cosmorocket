package pl.edu.pk.cosmo.rakieta;

public class LoRaException extends RuntimeException {

    public LoRaException(String message) {

        super(message);

    }

    LoRaException(String message, Throwable cause) {

        super(message, cause);

    }

    LoRaException(Throwable cause) {

        super(cause);

    }

}
