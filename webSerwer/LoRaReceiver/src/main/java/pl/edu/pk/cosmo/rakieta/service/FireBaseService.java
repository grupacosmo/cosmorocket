package pl.edu.pk.cosmo.rakieta.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.auth.oauth2.AccessToken;
import com.google.auth.oauth2.GoogleCredentials;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseOptions;
import com.google.firebase.database.FirebaseDatabase;
import pl.edu.pk.cosmo.rakieta.FirebaseCredentials;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.IOException;

public class FireBaseService {

    public static final String URL = "https://cosmopklora-default-rtdb.europe-west1.firebasedatabase.app/";
    public static final String SDK_JSON = "cosmopklora-firebase-adminsdk-gnt9r-f4d247185a.json";
    FirebaseDatabase db;
    public static final String FIREBASE_PRIVATE_KEY = "FIREBASE_PRIVATE_KEY";
    public static final String FIREBASE_PRIVATE_KEY_ID = "FIREBASE_PRIVATE_KEY_ID";
    public FireBaseService() throws IOException {
        String privateKey = System.getenv(FIREBASE_PRIVATE_KEY).replace("\\n", "\n");
        String privateKeyId = System.getenv(FIREBASE_PRIVATE_KEY_ID);

        ObjectMapper objectMapper = new ObjectMapper();
        FileInputStream serviceAccount =
                new FileInputStream(getClass().getClassLoader().getResource(SDK_JSON).getFile());
        FirebaseCredentials credentials = objectMapper.readValue(serviceAccount, FirebaseCredentials.class);
        credentials.setPrivateKey(privateKey);
        credentials.setPrivateKeyId(privateKeyId);
        byte[] credecialsBytes = objectMapper.writeValueAsBytes(credentials);

        FirebaseOptions options = new FirebaseOptions.Builder()
                .setCredentials(GoogleCredentials.fromStream(new ByteArrayInputStream(credecialsBytes)))
                .setDatabaseUrl(URL)
                .build();

        FirebaseApp.initializeApp(options);

        db = FirebaseDatabase.getInstance();
    }

    public FirebaseDatabase getDb() {
        return db;
    }

}
