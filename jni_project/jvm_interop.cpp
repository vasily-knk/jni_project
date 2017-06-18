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


void free_local_ref(jobject p)
{
    env_instance()->DeleteLocalRef(p);
}

jobject_ptr wrap(jobject p)
{
    if (!p)
        return nullptr;
    
    return make_shared<jobject_wrapper>(p);
}

jobject_ptr wrap_null()
{
    return nullptr;
}

jobject unwrap(jobject_ptr p)
{
    if (!p)
        return nullptr;

    return p->get_p();
}





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

jobject_ptr cpp2jvm(string const &src)
{
	return wrap(env_instance()->NewStringUTF(src.c_str()));
}

namespace detail
{
	
    void jvm2cpp_impl(jobject_ptr src, string &dst)
    {
	    auto env = env_instance();

	    auto src_str = static_cast<jstring>(src->get_p());

	    char const *p = env->GetStringUTFChars(src_str, nullptr);
        dst = p;

        env->ReleaseStringUTFChars(src_str, p);
	}

} // namespace detail


void process_jvm_exceptions()
{
    JNIEnv* env = env_instance();

    if (env->ExceptionCheck())
    {
        auto e = wrap(env->ExceptionOccurred());
        
        env->ExceptionClear(); 

        jclass clazz = env->GetObjectClass(e->get_p());
        jmethodID getMessage = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
        
        auto str = string();// jvm2cpp<string>(call_method<jobject_ptr>(e, getMessage));

        env->ExceptionClear();

        std::stringstream ss;
        ss << "JVM Exception: " << str;
        throw jvm_interop_error(ss.str());
    }
}

jclass find_class(char const *name)
{
    JNIEnv* env = env_instance();

    jclass jcls = env->FindClass(name);

    if (!jcls)
    {
        std::stringstream ss;
        ss << "JVM class not found: " << name;
        throw jvm_interop_error(ss.str());
    }

    return jcls;

}

void register_native_impl(jclass clazz, char const *name, char const *sig, void *fn_ptr)
{
    JNIEnv* env = env_instance();

    JNINativeMethod m = { const_cast<char*>(name), const_cast<char*>(sig), fn_ptr };

    auto ret = env->RegisterNatives(clazz, &m, 1);
    if (ret != JNI_OK)
        process_jvm_exceptions();
}

jmethodID get_method(jclass clazz, char const *method_name, char const *sig)
{
    JNIEnv* env = env_instance();
    jmethodID id = env->GetMethodID(clazz, method_name, sig);

    if (!id)
    {
        std::stringstream ss;
        ss << "Method '" << method_name << "' with signature '" << sig << "' not found";
        throw jvm_interop_error(ss.str());
    }

    return id;

}

jmethodID get_static_method(jclass clazz, char const *method_name, char const *sig)
{
    JNIEnv* env = env_instance();
    jmethodID id = env->GetStaticMethodID(clazz, method_name, sig);

    if (!id)
    {
        std::stringstream ss;
        ss << "Static method '" << method_name << "' with signature '" << sig << "' not found";
        throw jvm_interop_error(ss.str());
    }

    return id;

}



    
} // namespace jvm_interop