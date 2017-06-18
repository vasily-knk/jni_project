// jni_project.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "jvm_interop.h"

struct bar
{
	jint i;
	string str;
    optional<float> of;

    template<typename Proc>
    friend void reflect(Proc &proc, bar const &const_val)
    {
        auto &val = const_cast<bar &>(const_val);

        proc(val.i, "i");
        proc(val.str, "str");
        proc(val.of, "of");
    }
};

JVM_INTEROP_DECLARE_STRUCT_TYPE(bar, "Bar")


void JNICALL foo(JNIEnv *env, jobject obj, int32_t val)
{
	auto ptr = reinterpret_cast<uint8_t *>(&val);
	std::cout << "FOOOING: " << val << "\n";
}

int main(int argc, char **argv)
{

	JNIEnv *env = jvm_interop::env_instance();

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

    bar b2;


	try
	{
        jobject_ptr jb = cpp2jvm(b);
        b2 = jvm2cpp<bar>(jb);
	} 
	catch (jvm_interop_error const &e)
	{
		std::cerr << e.what() << std::endl;
	}


	return 0;
}