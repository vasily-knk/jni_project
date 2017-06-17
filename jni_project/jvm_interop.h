#pragma once

namespace jvm_interop
{

void free_local_ref(jobject p);

struct jobject_wrapper
{
    explicit jobject_wrapper(jobject p)
        : p_(p)
    {}

    jobject_wrapper(jobject_wrapper const &) = delete;
    jobject_wrapper &operator=(jobject_wrapper const &) = delete;

    ~jobject_wrapper()
    {
        if (p_)
            free_local_ref(p_);
    }

    jobject get_p() const                                                      
    {
        return p_;
    }

private:
    jobject p_;
}; 

struct jobject_wrapper;
typedef shared_ptr<jobject_wrapper> jobject_ptr;
jobject_ptr wrap(jobject p);
    
struct runtime_type_desc	   ;
struct struct_runtime_type_desc;


typedef shared_ptr<runtime_type_desc> runtime_type_desc_ptr;
typedef shared_ptr<struct_runtime_type_desc> struct_runtime_type_desc_ptr;

struct runtime_type_desc
{
	virtual ~runtime_type_desc() = default;

	virtual string const &signature() = 0;
	virtual string const &java_name() = 0;
};

struct struct_runtime_type_desc
	: runtime_type_desc
{
	virtual string const &lookup_name() = 0;
	virtual jobject_ptr create() = 0;
	virtual jclass get_class() = 0;

	virtual jmethodID getter(char const *field_name, runtime_type_desc_ptr runtime_type_desc) = 0;
	virtual jmethodID setter(char const *field_name, runtime_type_desc_ptr runtime_type_desc) = 0;
};


runtime_type_desc_ptr create_primitive_runtime_type_desc(char const *java_name, char sig);
struct_runtime_type_desc_ptr create_struct_runtime_type_desc(char const *dot_separated_name);

template<typename T>
struct jvm_type_traits;


template<typename T>
struct jvm_primitive_type_traits
{
	typedef T jvm_type;
	static const bool is_primitive = true;
};

template<typename T>
struct jvm_struct_type_traits
{
	typedef jobject_ptr jvm_type;
	static const bool is_primitive = false;
};


#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(cpp_type, java_name, sig) \
	template<> \
	struct jvm_interop::jvm_type_traits<cpp_type> \
		: jvm_interop::jvm_primitive_type_traits<cpp_type> \
	{ \
		static jvm_interop::runtime_type_desc_ptr get_runtime_desc() \
		{ \
			static const auto p = jvm_interop::create_primitive_runtime_type_desc((java_name), (sig)); \
			return p; \
		} \
	};

#define JVM_INTEROP_DECLARE_STRUCT_TYPE(cpp_type, dot_separated_name) \
	template<> \
	struct jvm_interop::jvm_type_traits<cpp_type> \
		: jvm_interop::jvm_struct_type_traits<cpp_type> \
	{ \
		static jvm_interop::struct_runtime_type_desc_ptr get_runtime_desc() \
		{ \
			static const auto p = jvm_interop::create_struct_runtime_type_desc(dot_separated_name); \
			return p; \
		} \
	};






JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jboolean, "boolean", 'Z') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jbyte   , "byte"   , 'B') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jchar   , "char"   , 'C') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jdouble , "double" , 'D') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jfloat  , "float"  , 'F') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jint    , "int"    , 'I') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jlong   , "long"   , 'J') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jshort  , "short"  , 'S') 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(void    , "void"   , 'V') 

JVM_INTEROP_DECLARE_STRUCT_TYPE(string, "java.lang.String")

template<typename T>
runtime_type_desc_ptr get_runtime_type_desc()
{
	return jvm_type_traits<T>::get_runtime_desc();
}



template<typename T>
using jvm_type_t = typename jvm_type_traits<T>::jvm_type;

template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args);

namespace detail
{
    struct cpp2jvm_processor_t
    {
        explicit cpp2jvm_processor_t(struct_runtime_type_desc_ptr desc, jobject_ptr dst)
            : desc_(desc)
            , dst_(dst)
        {}

        template<typename T>
        void operator()(T const &field_src, char const *name) 
        {
            auto field_dst = cpp2jvm(field_src);

            auto field_desc = jvm_type_traits<T>::get_runtime_desc();
            jmethodID setter = desc_->setter(name, field_desc);

            call_method<void>(dst_, setter, field_dst);
        }

    private:
        struct_runtime_type_desc_ptr desc_;
        jobject_ptr dst_;
    };

} // namespace detail

template<typename T>
T cpp2jvm(T src, std::enable_if_t<jvm_type_traits<T>::is_primitive> * = nullptr)
{
	return src;
}

jstring cpp2jvm(string const &src);


template<typename T>
jobject_ptr cpp2jvm(T const &src, std::enable_if_t<!jvm_type_traits<T>::is_primitive> * = nullptr)
{
	typedef jvm_type_traits<T> traits;

	struct_runtime_type_desc_ptr runtime_desc = traits::get_runtime_desc();
    jobject_ptr obj = runtime_desc->create();

    detail::cpp2jvm_processor_t proc(runtime_desc, obj);
    reflect(proc, src);

	return obj;
}



namespace detail
{
	template<typename T, typename Enable = void>
	struct jvm2cpp_t;

	template <typename T>
	struct jvm2cpp_t<T, std::enable_if_t<jvm_type_traits<T>::is_primitive>>
	{
		static T process(T src)
		{
			return src;
		}
	};

	template <typename T>
	struct jvm2cpp_t<T, std::enable_if_t<!jvm_type_traits<T>::is_primitive>>
	{
		static T process(jobject_ptr src)
		{
			return T();
		}
	};

	template<>
	struct jvm2cpp_t<string>
	{
		static string process(jobject_ptr src);
	};
} // namespace detail

template<typename T>
T jvm2cpp(typename jvm_type_traits<T>::jvm_type src)
{
	return detail::jvm2cpp_t<T>::process(src);
}




JNIEnv *env_instance();

struct jvm_interop_error
	: std::runtime_error
{
	explicit jvm_interop_error(const string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit jvm_interop_error(const char* _Message)
		: runtime_error(_Message)
	{
	}
};

string make_method_signature(runtime_type_desc_ptr ret, vector<runtime_type_desc_ptr> args);

template<typename Ret, typename... Args>
string make_method_signature()
{
    return make_method_signature(
        jvm_type_traits<Ret>::get_runtime_desc(),
        { jvm_type_traits<Args>::get_runtime_desc()... }
    );
}

void process_jvm_exceptions();

namespace detail
{
	template<typename T>
    struct method_caller;

#define JVM_INTEROP_DEFINE_METHOD_CALLER(type, Type) \
	template<>																				   \
	struct method_caller<type>									     						   \
	{																						   \
		template<typename... Args>															   \
		static type call(jobject_ptr obj, jmethodID method, Args&&... args)			    		   \
		{																					   \
			type result = env_instance()->Call ## Type ## Method(obj->get_p(), method, std::forward<Args>(args)...);  \
            process_jvm_exceptions(); \
            return result; \
		}																					   \
	};

    template<>
    struct method_caller<void>
    {
        template<typename... Args>
        static void call(jobject_ptr obj, jmethodID method, Args&&... args)
        {
            env_instance()->CallVoidMethod(obj->get_p(), method, std::forward<Args>(args)...);
            process_jvm_exceptions();
        }
    };
    
    template<> struct method_caller<jobject_ptr>
    {
        template<typename... Args>
        static jobject_ptr call(jobject_ptr obj, jmethodID method, Args&&... args)
        {
            jobject_ptr result = wrap(env_instance()->CallObjectMethod(obj->get_p(), method, std::forward<Args>(args)...)); 
            process_jvm_exceptions(); 
            return result;
        }
    };

JVM_INTEROP_DEFINE_METHOD_CALLER(jboolean, Boolean)
JVM_INTEROP_DEFINE_METHOD_CALLER(jbyte   , Byte)
JVM_INTEROP_DEFINE_METHOD_CALLER(jchar   , Char)
JVM_INTEROP_DEFINE_METHOD_CALLER(jdouble , Double)
JVM_INTEROP_DEFINE_METHOD_CALLER(jfloat  , Float)
JVM_INTEROP_DEFINE_METHOD_CALLER(jint    , Int)
JVM_INTEROP_DEFINE_METHOD_CALLER(jlong   , Long)
JVM_INTEROP_DEFINE_METHOD_CALLER(jshort  , Short)

#undef JVM_INTEROP_DEFINE_METHOD_CALLER


} // namespace detail


template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args)
{
	return detail::method_caller<T>::call(obj, method, std::forward<Args>(args)...);
}

} // namespace jvm_interop
