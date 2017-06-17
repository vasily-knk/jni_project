// jni_project.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "jvm_interop.h"

struct bar
{
	int i;
	string str;
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

	jclass jcls = env->FindClass("org/jnijvm/Demo");
	if (!jcls) 
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
		return 1;
	}

	vector<JNINativeMethod> methods = 
	{
		{"foo", "(I)V", &foo},
	};

	if (auto ret = env->RegisterNatives(jcls, methods.data(), methods.size()) != JNI_OK)
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
		return -1;
	}



	jmethodID methodId = env->GetStaticMethodID(jcls, "greet", "([Ljava/lang/String;IF)V");
	if (!methodId)
	{
		std::cerr << "method not found\n";
		return 1;
	}
	auto string_class = env->FindClass("java/lang/String");
	if (!string_class)
	{
		std::cerr << "string class not found\n";
		return -1;
	}

	vector<string> strings = {
		"aaa", "bbb", "ccc",
	};

	auto arr = env->NewObjectArray(strings.size(), string_class, nullptr);
	if (!arr)
	{
		std::cerr << "Array creation failed\n";
		return -1;
	}
	for (jsize i = 0; i < strings.size(); ++i)
	{
		env->SetObjectArrayElement(arr, i, env->NewStringUTF(strings.at(i).c_str()));
	}
	
	jint int_arg = 117;
	jfloat float_arg = 3.14f;

	env->CallStaticVoidMethod(jcls, methodId, arr, int_arg, float_arg);
	
	if (env->ExceptionCheck()) 
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	bar b;
	jobject jb = nullptr;

	using namespace jvm_interop;

	try
	{
		jb = cpp2jvm(b);

		auto desc = jvm_type_traits<bar>::get_runtime_desc();

        typedef string ret_type;
        typedef jvm_type_traits<ret_type>::jvm_type jvm_ret_type;

		auto getter = desc->getter("secret_string", get_runtime_type_desc<ret_type>());

		auto f = call_method<jvm_ret_type>(jb, getter);

		std::cout << "Secret value: " << jvm2cpp<ret_type>(f) << std::endl;
	} 
	catch (jvm_interop_error const &e)
	{
		std::cerr << e.what() << std::endl;
	}

	jvm2cpp<jint>(5);

    auto s = make_method_signature<void, jint>();


	return 0;
}