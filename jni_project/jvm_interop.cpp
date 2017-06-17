#include "stdafx.h"
#include "jvm_interop.h"

namespace jvm_interop
{

namespace
{
	struct java_vm_t
	{
		java_vm_t()
		{
			JavaVMOption jvmopt[1];
			jvmopt[0].optionString = "-Djava.class.path=../out";
			jvmopt[0].extraInfo = nullptr;

			JavaVMInitArgs vmArgs;
			vmArgs.version = JNI_VERSION_1_8;
			vmArgs.nOptions = 1;
			vmArgs.options = jvmopt;
			vmArgs.ignoreUnrecognized = JNI_TRUE;

			long flag = JNI_CreateJavaVM(&vm, reinterpret_cast<void**>(&env_), &vmArgs);

			if (flag == JNI_ERR)
			{
				std::stringstream ss;
				ss << "Failed to start JVM";
				throw jvm_interop_error(ss.str());
			}
		}

		~java_vm_t()
		{
			if (vm)
			{
				vm->DestroyJavaVM();
				vm = nullptr;
			}
		}

		java_vm_t(java_vm_t const &) = delete;
		java_vm_t &operator=(java_vm_t const &) = delete;

		JNIEnv *get_env() const
		{
			return env_;
		}

	private:
		JavaVM *vm = nullptr;
		JNIEnv *env_ = nullptr;
	};

	java_vm_t const &jvm_instance()
	{
		static java_vm_t jvm;
		return jvm;
	}

} // namespace



JNIEnv *env_instance()
{
	return jvm_instance().get_env();
}

string make_method_signature(runtime_type_desc_ptr ret, vector<runtime_type_desc_ptr> args)
{
	std::stringstream ss;
	ss << "(";

	for (auto arg : args)
		ss << arg->signature();

	ss << ")" << ret->signature();

	return ss.str();
}

jstring cpp2jvm(string const &src)
{
	return env_instance()->NewStringUTF(src.c_str());
}

namespace detail
{
	
	string jvm2cpp_t<string>::process(jobject src)
	{
		return env_instance()->GetStringUTFChars(static_cast<jstring>(src), nullptr);
	}

} // namespace detail


void process_jvm_exceptions()
{
    JNIEnv* env = env_instance();

    if (env->ExceptionCheck())
    {
        jthrowable e = env->ExceptionOccurred();
        env->ExceptionClear(); 

        jclass clazz = env->GetObjectClass(e);
        jmethodID getMessage = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");

        jstring message = static_cast<jstring>(env->CallObjectMethod(e, getMessage));
        const char *mstr = env->GetStringUTFChars(message, nullptr);

        env->ExceptionClear();

        std::stringstream ss;
        ss << "JVM Exception: " << mstr;
        throw jvm_interop_error(ss.str());
    }
}

    
} // namespace jvm_interop