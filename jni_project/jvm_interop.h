#pragma once

namespace jvm_interop
{
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
	virtual jobject create() = 0;
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
	typedef jobject jvm_type;
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

template<typename T>
T cpp2jvm(T src, std::enable_if_t<jvm_type_traits<T>::is_primitive> * = nullptr)
{
	return src;
}

template<typename T>
jobject cpp2jvm(T const &src, std::enable_if_t<!jvm_type_traits<T>::is_primitive> * = nullptr)
{
	typedef jvm_type_traits<T> traits;

	struct_runtime_type_desc_ptr runtime_desc = traits::get_runtime_desc();
	jobject obj = runtime_desc->create();

	return obj;
}

jstring cpp2jvm(string const &src);

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
		static T process(jobject src)
		{
			return T();
		}
	};

	template<>
	struct jvm2cpp_t<string>
	{
		static string process(jobject src);
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
		static type call(jobject obj, jmethodID method, Args&&... args)			    		   \
		{																					   \
			type result = env_instance()->Call ## Type ## Method(obj, method, std::forward<Args>(args)...);  \
            process_jvm_exceptions(); \
            return result; \
		}																					   \
	};

JVM_INTEROP_DEFINE_METHOD_CALLER(jboolean, Boolean)
JVM_INTEROP_DEFINE_METHOD_CALLER(jbyte   , Byte)
JVM_INTEROP_DEFINE_METHOD_CALLER(jchar   , Char)
JVM_INTEROP_DEFINE_METHOD_CALLER(jdouble , Double)
JVM_INTEROP_DEFINE_METHOD_CALLER(jfloat  , Float)
JVM_INTEROP_DEFINE_METHOD_CALLER(jint    , Int)
JVM_INTEROP_DEFINE_METHOD_CALLER(jlong   , Long)
JVM_INTEROP_DEFINE_METHOD_CALLER(jshort  , Short)
JVM_INTEROP_DEFINE_METHOD_CALLER(void    , Void)
JVM_INTEROP_DEFINE_METHOD_CALLER(jobject , Object)

#undef JVM_INTEROP_DEFINE_METHOD_CALLER


} // namespace detail


template<typename T, typename... Args>
T call_method(jobject obj, jmethodID method, Args&&... args)
{
	return detail::method_caller<T>::call(obj, method, std::forward<Args>(args)...);
}

} // namespace jvm_interop
