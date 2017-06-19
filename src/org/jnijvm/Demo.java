package org.jnijvm;

import org.vasya.Bar;

public class Demo {
    public static void greet(String name, int int_arg, float float_arg) {
        System.out.print("Hi: " );
        System.out.print(name);
        System.out.println();

        System.out.println("Int: " + int_arg);
        System.out.println("Float: " + float_arg);

    }

    public static Bar modifyBar(Bar src)
    {
        Bar dst = new Bar();

        dst.setI(src.getI() * 2);
        dst.setStr(src.getStr() + " year right");
        dst.setOf(src.getOf() == null ? null : src.getOf() * 0.5f);
        dst.setOstr(src.getOstr() == null ? null : src.getOstr());
        dst.setObz(src.getObz() == null ? null : src.getObz());

        return dst;
    }

    public static native void foo(int val);
}