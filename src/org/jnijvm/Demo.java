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

    public static Bar modifyBar(Bar src) {
        return Fucker.fuckBar(src);
    }

    public static void printStrings(String[][] strss) {
        for (String[] strs : strss) {
            System.out.println();
            System.out.println("-- Pack:");
            for (String str : strs)
                System.out.println(str);
        }
    }

    public static void printInts(int[] ints) {
        for (int i : ints)
            System.out.println("Int: " + i);
    }



    public static void main(String[] args)
    {
        Bar b = new Bar();
        modifyBar(b);
    }
}