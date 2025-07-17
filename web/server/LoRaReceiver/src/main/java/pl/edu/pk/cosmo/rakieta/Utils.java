package pl.edu.pk.cosmo.rakieta;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Optional;

public class Utils {

    private Utils() {}

    public static Optional<String> getEnv(String env) {

        String value = System.getenv(env);

        if(value != null && !value.isEmpty()) {

            return Optional.of(value);

        }

        return Optional.empty();

    }

}
