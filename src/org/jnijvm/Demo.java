package org.jnijvm;

import org.vasya.Bar;

import java.util.ArrayList;


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

    public static String[] printStrings(String[][] strss) {
        ArrayList<String> arrs = new ArrayList<>();
        for (String[] strs : strss) {
            System.out.println();
            System.out.println("-- Pack:");
            for (String str : strs) {
                System.out.println(str);
                arrs.add(str);
            }
        }

        return arrs.toArray(new String[0]);
    }

    public static int[] printInts(int[] ints) {
        int[] result = new int[ints.length];
        for (int i = 0; i < ints.length; ++i) {
            System.out.println("Int: " + ints[i]);
            result[i] = ints[i] * 10;
        }

        return result;
    }



    public static void main(String[] args)
    {
        Bar b = new Bar();
        modifyBar(b);
    }
}