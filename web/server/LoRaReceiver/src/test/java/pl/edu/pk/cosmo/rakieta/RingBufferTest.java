package pl.edu.pk.cosmo.rakieta;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.List;

import org.junit.jupiter.api.Test;

class RingBufferTest {

    @Test
    void addTest() {

        RingBuffer<Integer> buffer = new RingBuffer<>(10);

        buffer.addAll(List.of(1,2,3,4,5,6,7,8,9,10,11));

        assertEquals(List.of(2,3,4,5,6,7,8,9,10,11), buffer);

    }

    @Test
    void containsTest() {

        RingBuffer<Integer> buffer = new RingBuffer<>(10);

        buffer.addAll(List.of(1,2,3,4,5,6,7,8,9,10));

        assertTrue(buffer.contains(5));

    }

}
