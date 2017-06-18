package org.jnijvm;

public class Demo {
    public static void greet(String name, int int_arg, float float_arg) {
        System.out.print("Hi: " );
        System.out.print(name);
        System.out.println();

        System.out.println("Int: " + int_arg);
        System.out.println("Float: " + float_arg);

    }

    public static native void foo(int val);
}