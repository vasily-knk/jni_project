package org.jnijvm;

public class Demo {
    public static void greet(String name) {
        foo(73);
        System.out.println("Hi! " + name);
    }

    public static native void foo(int val);
}