package org.vasya;

public class Vec2d extends Vec2dBase {
    public Vec2d(float x, float y) {
        setX(x);
        setY(y);
    }

    public float length() {
        return (float)Math.sqrt(x * x + y * y);
    }
}
