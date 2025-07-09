package pl.edu.pk.cosmo.rakieta.service;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.auth.oauth2.GoogleCredentials;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseOptions;
import com.google.firebase.database.FirebaseDatabase;

import pl.edu.pk.cosmo.rakieta.FirebaseCredentials;

public class FireBaseService {

    public static final String URL = "https://cosmopklora-default-rtdb.europe-west1.firebasedatabase.app/";
    public static final String SDK_JSON = "firebase-credentials.json";
    public static final String FIREBASE_PRIVATE_KEY = "FIREBASE_PRIVATE_KEY";
    public static final String FIREBASE_PRIVATE_KEY_ID = "FIREBASE_PRIVATE_KEY_ID";
    private FirebaseDatabase db;

    public FireBaseService() throws IOException {

        File credentialsFile = new File(SDK_JSON);

        try(InputStream credentialsStream = credentialsFile.exists() ? new FileInputStream(credentialsFile) : getClass()
            .getResourceAsStream(SDK_JSON)) {

            if(credentialsStream.available() <= 0) {

                throw new RuntimeException("No data in firebase credentials file!");

            }

            ObjectMapper objectMapper = new ObjectMapper();
            FirebaseCredentials credentials = objectMapper.readValue(credentialsStream, FirebaseCredentials.class);

            String privateKeyEnv = System.getenv(FIREBASE_PRIVATE_KEY);

            if(privateKeyEnv != null && !privateKeyEnv.isEmpty()) {

                credentials.setPrivateKey(privateKeyEnv);

            }

            String privateKeyIdEnv = System.getenv(FIREBASE_PRIVATE_KEY);

            if(privateKeyIdEnv != null && !privateKeyIdEnv.isEmpty()) {

                credentials.setPrivateKey(privateKeyIdEnv);

            }

            FirebaseOptions options = new FirebaseOptions.Builder()
                .setCredentials(GoogleCredentials.fromStream(new ByteArrayInputStream(objectMapper.writeValueAsBytes(credentials))))
                .setDatabaseUrl(URL)
                .build();

            FirebaseApp.initializeApp(options);

        }

        db = FirebaseDatabase.getInstance();

    }

    public FirebaseDatabase getDb() {

        return db;

    }

}
