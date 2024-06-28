package pl.edu.pk.cosmo.rakieta;


import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

public class FirebaseCredentials {

    private String privateKeyId;

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

    @JsonProperty("private_key_id")
    public String getPrivateKeyId() {
        return privateKeyId;
    }

    @JsonIgnore
    public  void setPrivateKeyId(String privateKeyId) {
        this.privateKeyId = privateKeyId;
    }

    @JsonProperty("private_key")
    public String getPrivateKey() {
        return privateKey;
    }

    @JsonIgnore
    public void setPrivateKey(String privateKey) {
        this.privateKey = privateKey;
    }


    @JsonProperty("type")
    public void setType(String type){ this.type = type; }
    @JsonProperty("type")
    public String getType(){ return this.type; }

    @JsonProperty("project_id")
    public void setProjectId(String projectId){ this.projectId = projectId; }
    @JsonProperty("project_id")
    public String getProjectId(){ return this.projectId; }

    @JsonProperty("client_email")
    public void setClientEmail(String clientEmail){ this.clientEmail = clientEmail; }
    @JsonProperty("client_email")
    public String getClientEmail(){ return this.clientEmail; }

    @JsonProperty("client_id")
    public void setClientId(String clientId){ this.clientId = clientId; }
    @JsonProperty("client_id")
    public String getClientId(){ return this.clientId; }

    @JsonProperty("auth_uri")
    public void setAuthUri(String authUri){ this.authUri = authUri; }
    @JsonProperty("auth_uri")
    public String getAuthUri(){ return this.authUri; }

    @JsonProperty("token_uri")
    public void setTokenUri(String tokenUri){ this.tokenUri = tokenUri; }
    @JsonProperty("token_uri")
    public String getTokenUri(){ return this.tokenUri; }

    @JsonProperty("auth_provider_x509_cert_url")
    public void setAuthProviderX509CertUrl(String authProviderX509CertUrl){ this.authProviderX509CertUrl = authProviderX509CertUrl; }
    @JsonProperty("auth_provider_x509_cert_url")
    public String getAuthProviderX509CertUrl(){ return this.authProviderX509CertUrl; }

    @JsonProperty("client_x509_cert_url")
    public void setClientX509CertUrl(String clientX509CertUrl){ this.clientX509CertUrl = clientX509CertUrl; }
    @JsonProperty("client_x509_cert_url")
    public String getClientX509CertUrl(){ return this.clientX509CertUrl; }

    @JsonProperty("universe_domain")
    public void setUniverseDomain(String universeDomain){ this.universeDomain = universeDomain; }
    @JsonProperty("universe_domain")
    public String getUniverseDomain(){ return this.universeDomain; }
}
