// jni_project.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <jni.h>
#include <iostream>
#include <string>
using namespace std;


struct java_vm_t
{
	java_vm_t() = default;
	
	java_vm_t(java_vm_t const &) = delete;
	java_vm_t &operator=(java_vm_t const &) = delete;
	
	~java_vm_t()
	{
		if (vm)
		{
			vm->DestroyJavaVM();
			vm = nullptr;
		}
	}
	
	JavaVM *vm = nullptr;
};

void JNICALL foo(JNIEnv *env, jobject obj, int32_t val)
{
	auto ptr = reinterpret_cast<uint8_t *>(&val);
	cout << "FOOOING: " << val << "\n";
}

int main(int argc, char **argv)
{

	JavaVMOption jvmopt[1];
	jvmopt[0].optionString = "-Djava.class.path=../out";
	jvmopt[0].extraInfo = nullptr;

	JavaVMInitArgs vmArgs;
	vmArgs.version = JNI_VERSION_1_8;
	vmArgs.nOptions = 1;
	vmArgs.options = jvmopt;
	vmArgs.ignoreUnrecognized = JNI_TRUE;

	java_vm_t java_vm;

	// Create the JVM
	JNIEnv *env;
	long flag = JNI_CreateJavaVM(&java_vm.vm, reinterpret_cast<void**>(&env), &vmArgs);

	if (flag == JNI_ERR) 
	{
		cout << "Error creating VM. Exiting...\n";
		return 1;
	}

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
		cerr << "method not found\n";
		return 1;
	}
	auto string_class = env->FindClass("java/lang/String");
	if (!string_class)
	{
		cerr << "string class not found\n";
		return -1;
	}

	vector<string> strings = {
		"aaa", "bbb", "ccc",
	};

	auto arr = env->NewObjectArray(strings.size(), string_class, nullptr);
	if (!arr)
	{
		cerr << "Array creation failed\n";
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

	return 0;
}