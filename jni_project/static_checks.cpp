#include "stdafx.h"
#include "jvm_interop.h"

namespace jvm_interop
{

    // jint
    
    static_assert(jvm_type_traits<jint>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<jint>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<optional<jint>>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<optional<jint>>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<vector<jint>>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<vector<jint>>::needs_generation, "AAA");

    // string

    static_assert(!jvm_type_traits<string>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<string>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<optional<string>>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<optional<string>>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<vector<string>>::is_primitive, "AAA");
    static_assert(!jvm_type_traits<vector<string>>::needs_generation, "AAA");

    // my_struct

    struct my_struct
    {
        
    };

    JVM_INTEROP_DECLARE_USER_TYPE(my_struct, "org.vasya.MyStruct")

    static_assert(!jvm_type_traits<my_struct>::is_primitive, "AAA");
    static_assert(jvm_type_traits<my_struct>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<optional<my_struct>>::is_primitive, "AAA");
    static_assert(jvm_type_traits<optional<my_struct>>::needs_generation, "AAA");

    static_assert(!jvm_type_traits<vector<my_struct>>::is_primitive, "AAA");
    static_assert(jvm_type_traits<vector<my_struct>>::needs_generation, "AAA");




} // namespace jvm_interop