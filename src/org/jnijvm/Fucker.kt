package org.jnijvm

import org.vasya.Bar

/**
 * Created by vasya on 20.06.2017.
 */

class Fucker {
    companion object {
        @JvmStatic
        fun fuckBar(src: Bar): Bar {
            println("Fucking bar!")

            var dst = Bar()

            dst.i = src.i * 2
            dst.str = src.str + " year right"
            dst.of = if (src.of == null) null else src.of * 0.5f
            dst.ostr = if (src.ostr == null) null else "No!" + src.ostr

            dst.strings = arrayOf("A", "B");


            return dst
        }
    }
}

