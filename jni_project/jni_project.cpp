// jni_project.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "jvm_interop.h"
#include "java_generator.h"

struct baz
{
    jint hhh;

    template<typename Proc>
    friend void reflect(Proc &proc, baz const &const_val)
    {
        auto &val = const_cast<baz &>(const_val);
        proc(val.hhh, "hhh");
    }
};

struct bar
{
	jint i;
	string str;
    optional<float> of;
    optional<string> ostr;

    //baz bz;
    //optional<baz> obz;
    vector<baz> i_arr;
                      
    template<typename Proc>
    friend void reflect(Proc &proc, bar const &const_val)
    {
        auto &val = const_cast<bar &>(const_val);

        proc(val.i, "i");
        proc(val.str, "str");
        proc(val.of, "of");
        proc(val.ostr, "ostr");
        //proc(val.bz, "bz");
        //proc(val.obz, "obz");
        //proc(val.i_arr, "i_arr");
    }
};

struct vec2d_t
{
    float x = 0.f;
    float y = 0.f;

    template<typename Proc>
    friend void reflect(Proc &proc, vec2d_t const &const_val)
    {
        auto &val = const_cast<vec2d_t &>(const_val);

        proc(val.x, "x");
        proc(val.y, "y");
    }
};

JVM_INTEROP_DECLARE_USER_TYPE(bar, "org.vasya.Bar")
JVM_INTEROP_DECLARE_USER_TYPE(baz, "org.vasya.Baz")
JVM_INTEROP_DECLARE_USER_TYPE_EXT(vec2d_t, "org.vasya.Vec2d", "org.vasya.Vec2dBase")


//void JNICALL foo(JNIEnv *env, jobject obj, int32_t val)
//{
//	auto ptr = reinterpret_cast<uint8_t *>(&val);
//	std::cout << "FOOOING: " << val << "\n";
//}


void JNICALL onBaz_c(JNIEnv *env, jobject self_raw, jobject b_raw)
{
    using namespace jvm_interop;

    auto b = jvm2cpp<baz>(wrap(b_raw));

    int aaa = 5;
}


int amain(int argc, char **argv)
{


//	jclass jcls = env->FindClass("org/jnijvm/Demo");
//	if (!jcls) 
//	{
//		env->ExceptionDescribe();
//		env->ExceptionClear();
//		return 1;
//	}
//
//	vector<JNINativeMethod> methods = 
//	{
//		{"foo", "(I)V", &foo},
//	};
//
//	if (auto ret = env->RegisterNatives(jcls, methods.data(), methods.size()) != JNI_OK)
//	{
//		env->ExceptionDescribe();
//		env->ExceptionClear();
//		return -1;
//	}
//
//
//
//	jmethodID methodId = env->GetStaticMethodID(jcls, "greet", "([Ljava/lang/String;IF)V");
//	if (!methodId)
//	{
//		std::cerr << "method not found\n";
//		return 1;
//	}
//	auto string_class = env->FindClass("java/lang/String");
//	if (!string_class)
//	{
//		std::cerr << "string class not found\n";
//		return -1;
//	}
//
//	vector<string> strings = {
//		"aaa", "bbb", "ccc",
//	};
//
//	auto arr = env->NewObjectArray(strings.size(), string_class, nullptr);
//	if (!arr)
//	{
//		std::cerr << "Array creation failed\n";
//		return -1;
//	}
//	for (jsize i = 0; i < strings.size(); ++i)
//	{
//		env->SetObjectArrayElement(arr, i, env->NewStringUTF(strings.at(i).c_str()));
//	}
//	
//	jint int_arg = 117;
//	jfloat float_arg = 3.14f;
//
//	env->CallStaticVoidMethod(jcls, methodId, arr, int_arg, float_arg);
//	
//	if (env->ExceptionCheck()) 
//	{
//		env->ExceptionDescribe();
//		env->ExceptionClear();
//	}

    using namespace jvm_interop;

    bar b;
    b.i = 10002;
    b.str = "Heya!";
    b.of = 3.14f;
    b.ostr = "HAAA";

    //b.obz = baz();

    bar b2;


	try
	{
        struct_fields_map_t fields_map;
	    append_struct_fields<bar>(fields_map);
        append_struct_fields<vec2d_t>(fields_map);

        generate_java_structs(fields_map, "../src");
//
        jobject_ptr jb = cpp2jvm(b);
        b2 = jvm2cpp<bar>(jb);


        //
//        auto m = get_method<void, double>(bar_jclass, "doSomething");
//
//        m(jb)(256.4);
//
//
//        jclass demo_jclass = find_class("org/jnijvm/Demo");
//        auto sm = get_static_method<void, string, jint, jfloat>(demo_jclass, "greet");
//
//        sm("Vasya", 32, 116.99f);

	} 
	catch (jvm_interop_error const &e)
	{
		std::cerr << e.what() << std::endl;
	}


                                 
    return 0;
}

void generate()
{
    using namespace jvm_interop;

    struct_fields_map_t fields_map;
    append_struct_fields<bar>(fields_map);
    append_struct_fields<vec2d_t>(fields_map);

    generate_java_structs(fields_map, "../src");

}

void interop()
{
    using namespace jvm_interop;

    bar b;
    b.i = 10002;
    b.str = "Heya!";
    b.of = 3.14f;
    b.ostr = "HAAA";

    //b.obz = baz();

    bar b2;


    try
    {
        JNIEnv *env = jvm_interop::env_instance();

        jobject_ptr jb = cpp2jvm(b);
        
        jclass demo_jclass = find_class("org/jnijvm/Demo");
        auto sm = get_static_method<bar, bar>(demo_jclass, "modifyBar");

        b2 = sm(b);


    }
    catch (jvm_interop_error const &e)
    {
        std::cerr << e.what() << std::endl;
    }



}

int main(int argc, char **argv)
{
    generate();
    //interop();
    return 0;
}
