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
import pl.edu.pk.cosmo.rakieta.Utils;

public class FireBaseService {

    public static final String FIREBASE_PRIVATE_KEY = "FIREBASE_PRIVATE_KEY";
    public static final String FIREBASE_PRIVATE_KEY_ID = "FIREBASE_PRIVATE_KEY_ID";
    private FirebaseDatabase db;

    public FireBaseService(String url, File credentialsFile) throws IOException {

        try(InputStream credentialsStream = new FileInputStream(credentialsFile)) {

            if(credentialsStream.available() <= 0) {

                throw new RuntimeException("No data in firebase credentials file!");

            }

            ObjectMapper objectMapper = new ObjectMapper();
            FirebaseCredentials credentials = objectMapper.readValue(credentialsStream, FirebaseCredentials.class);

            Utils.getEnv(FIREBASE_PRIVATE_KEY).ifPresent(credentials::setPrivateKey);

            Utils.getEnv(FIREBASE_PRIVATE_KEY_ID).ifPresent(credentials::setPrivateKeyId);

            FirebaseOptions options = new FirebaseOptions.Builder()
                .setCredentials(GoogleCredentials.fromStream(new ByteArrayInputStream(objectMapper.writeValueAsBytes(credentials))))
                .setDatabaseUrl(url)
                .build();

            FirebaseApp.initializeApp(options);

        }

        db = FirebaseDatabase.getInstance();

    }

    public FirebaseDatabase getDb() {

        return db;

    }

}
