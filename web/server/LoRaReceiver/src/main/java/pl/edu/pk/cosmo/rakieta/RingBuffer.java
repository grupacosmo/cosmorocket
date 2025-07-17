package pl.edu.pk.cosmo.rakieta;

import java.util.AbstractList;

import lombok.EqualsAndHashCode;

@EqualsAndHashCode(callSuper = false)
class RingBuffer<T> extends AbstractList<T> {

    private Object[] buffer;
    private int i = 0;

    public RingBuffer(int size) {

        buffer = new Object[size];

    }

    @Override
    public int size() {

        return buffer.length;

    }

    @SuppressWarnings("unchecked")
    @Override
    public T get(int index) {

        return (T) buffer[(i + index) % size()];

    }

    @Override
    public boolean add(T e) {

        buffer[i] = e;
        increment();
        return true;

    }

    private int increment() {

        i = (i + 1) % size();
        return i;

    }

}
