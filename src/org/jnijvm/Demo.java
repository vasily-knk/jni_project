package org.jnijvm;

public class Demo {
    public static void greet(String[] names, int int_arg, float float_arg) {
        foo(73);
        System.out.print("Hi: " );

        for (String name : names)
            System.out.print(name + " ");

        System.out.println();

        System.out.println("Int: " + int_arg);
        System.out.println("Float: " + float_arg);

    }

    public static native void foo(int val);
}