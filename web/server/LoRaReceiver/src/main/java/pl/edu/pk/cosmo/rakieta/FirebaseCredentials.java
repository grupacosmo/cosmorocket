package pl.edu.pk.cosmo.rakieta;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Setter
@Getter
@NoArgsConstructor
public class FirebaseCredentials {

    @JsonProperty("private_key_id")
    private String privateKeyId;

    @JsonProperty("private_key")
    private String privateKey;

    @JsonProperty("type")
    private String type;

    @JsonProperty("project_id")
    private String projectId;

    @JsonProperty("client_email")
    private String clientEmail;

    @JsonProperty("client_id")
    private String clientId;

    @JsonProperty("auth_uri")
    private String authUri;

    @JsonProperty("token_uri")
    private String tokenUri;

    @JsonProperty("auth_provider_x509_cert_url")
    private String authProviderX509CertUrl;

    @JsonProperty("client_x509_cert_url")
    private String clientX509CertUrl;

    @JsonProperty("universe_domain")
    private String universeDomain;

}
