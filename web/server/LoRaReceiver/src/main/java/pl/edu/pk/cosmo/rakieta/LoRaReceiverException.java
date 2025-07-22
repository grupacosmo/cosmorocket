package pl.edu.pk.cosmo.rakieta;

public class LoRaReceiverException extends RuntimeException {

    public LoRaReceiverException(String message) {

        super(message);

    }

    public LoRaReceiverException(String message, Throwable cause) {

        super(message, cause);

    }

    public LoRaReceiverException(Throwable cause) {

        super(cause);

    }

}
