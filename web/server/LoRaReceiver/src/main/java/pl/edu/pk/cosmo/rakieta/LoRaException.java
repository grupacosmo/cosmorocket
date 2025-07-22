package pl.edu.pk.cosmo.rakieta;

public class LoRaException extends RuntimeException {

    public LoRaException(String message) {

        super(message);

    }

    public LoRaException(String message, Throwable cause) {

        super(message, cause);

    }

    public LoRaException(Throwable cause) {

        super(cause);

    }

}
