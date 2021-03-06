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
            //jvmopt[0].optionString = R"(-Djava.class.path=C:\Program Files\Java\jdk1.8.0_121\jre\lib\charsets.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\deploy.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\access-bridge-64.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\cldrdata.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\dnsns.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\jaccess.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\jfxrt.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\localedata.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\nashorn.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\sunec.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\sunjce_provider.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\sunmscapi.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\sunpkcs11.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\ext\zipfs.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\javaws.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\jce.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\jfr.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\jfxswt.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\jsse.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\management-agent.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\plugin.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\resources.jar;C:\Program Files\Java\jdk1.8.0_121\jre\lib\rt.jar;C:\my\jni_project\out;C:\my\jni_project\lib\kotlin-runtime.jar;C:\my\jni_project\lib\kotlin-reflect.jar)";

            fs::path root_path = "..";
            fs::path jdk_path = std::getenv("JDK_PATH");

            vector<fs::path> class_paths = {
                root_path / "out",

//                jdk_path / "jre/lib/charsets.jar"            ,
//                jdk_path / "jre/lib/deploy.jar"              ,
//                jdk_path / "jre/lib/ext/access-bridge-64.jar",
//                jdk_path / "jre/lib/ext/cldrdata.jar"        ,
//                jdk_path / "jre/lib/ext/dnsns.jar"           ,
//                jdk_path / "jre/lib/ext/jaccess.jar"         ,
//                jdk_path / "jre/lib/ext/jfxrt.jar"           ,
//                jdk_path / "jre/lib/ext/localedata.jar"      ,
//                jdk_path / "jre/lib/ext/nashorn.jar"         ,
//                jdk_path / "jre/lib/ext/sunec.jar"           ,
//                jdk_path / "jre/lib/ext/sunjce_provider.jar" ,
//                jdk_path / "jre/lib/ext/sunmscapi.jar"       ,
//                jdk_path / "jre/lib/ext/sunpkcs11.jar"       ,
//                jdk_path / "jre/lib/ext/zipfs.jar"           ,
//                jdk_path / "jre/lib/javaws.jar"              ,
//                jdk_path / "jre/lib/jce.jar"                 ,
//                jdk_path / "jre/lib/jfr.jar"                 ,
//                jdk_path / "jre/lib/jfxswt.jar"              ,
//                jdk_path / "jre/lib/jsse.jar"                ,
//                jdk_path / "jre/lib/management-agent.jar"    ,
//                jdk_path / "jre/lib/plugin.jar"              ,
//                jdk_path / "jre/lib/resources.jar"           ,
//                jdk_path / "jre/lib/rt.jar"                  ,
                
//                root_path / "lib/kotlin-reflect-1.1.3.jar",
                root_path / "lib/kotlin-runtime-1.1.3.jar",
//                root_path / "lib/kotlin-stdlib-1.1.3.jar",
            };

            vector<string> class_paths_str;
            std::transform(class_paths.begin(), class_paths.end(), std::back_inserter(class_paths_str),
                [](fs::path const &p)
            {
                return p.string();
            });

            string joined_class_paths = "-Djava.class.path=" + boost::algorithm::join(class_paths_str, ";");

            jvmopt[0].optionString = const_cast<char*>(joined_class_paths.c_str());
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

jobject unwrap(jobject_ptr const &p)
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
	
    void check_struct_jvm2cpp(jobject_ptr src, struct_runtime_type_desc_ptr runtime_desc)
    {
        if (!src)
        {
            std::stringstream ss;
            ss << "Trying to convert null '" << runtime_desc->java_name() << "' to non-optional cpp struct";
            throw jvm_interop_error(ss.str());
        }
    }

    void jvm2cpp_impl(jobject_ptr src, string &dst)
    {
	    auto env = env_instance();
        
        check_struct_jvm2cpp(src, get_type_desc<string>());

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
        

        jclass clazz = env->GetObjectClass(e->get_p());
        jmethodID getMessage = env->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");
        env->ExceptionDescribe();
        
        env->ExceptionClear();
        auto str_obj = env->CallObjectMethod(e->get_p(), getMessage);

        auto str = jvm2cpp<string>(wrap(str_obj));

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