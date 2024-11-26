package pl.edu.pk.cosmo.rakieta.entity;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonPropertyOrder;
import lombok.EqualsAndHashCode;

@JsonPropertyOrder({"x","y","z"})
@EqualsAndHashCode
public class Float3 {
    public Float3(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    Float3() {
    }

    @JsonProperty("x")
    public float x;
    @JsonProperty("y")
    public float y;
    @JsonProperty("z")
    public float z;
}
