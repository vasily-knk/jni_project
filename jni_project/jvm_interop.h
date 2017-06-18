#pragma once

namespace jvm_interop
{

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

    virtual jmethodID find_method(char const *field_name, string const &sig) = 0;
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

template<typename T>
using enable_if_primitive_t = std::enable_if_t<jvm_type_traits<T>::is_primitive>;

template<typename T>
using enable_if_not_primitive_t = std::enable_if_t<!jvm_type_traits<T>::is_primitive>;


jobject_ptr wrap(jobject p);
jobject_ptr wrap_null();

template<typename T>
T wrap(T val, enable_if_primitive_t<T> * = nullptr)
{
    return val;
}

jobject unwrap(jobject_ptr p);

template<typename T>
T unwrap(T val, enable_if_primitive_t<T> * = nullptr)
{
    return val;
}

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


#define JVM_INTEROP_DECLARE_STRUCT_TYPE_NO_OPTIONAL(cpp_type, dot_separated_name) \
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

    
#define JVM_INTEROP_DECLARE_STRUCT_TYPE(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_STRUCT_TYPE_NO_OPTIONAL(cpp_type, dot_separated_name) \
	template<> \
	struct jvm_interop::jvm_type_traits<optional<cpp_type>> \
        : jvm_interop::jvm_type_traits<cpp_type> {};






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

JVM_INTEROP_DECLARE_STRUCT_TYPE_NO_OPTIONAL(optional<float>, "java.lang.Float")

template<typename T>
runtime_type_desc_ptr get_runtime_type_desc()
{
	return jvm_type_traits<T>::get_runtime_desc();
}



template<typename T>
using jvm_type_t = typename jvm_type_traits<T>::jvm_type;

template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args);

string make_method_signature(runtime_type_desc_ptr ret, vector<runtime_type_desc_ptr> args);

template<typename Ret, typename... Args>
string make_method_signature();
void process_jvm_exceptions();


struct exceptions_handler
{
    exceptions_handler() = default;
    ~exceptions_handler()
    {
        process_jvm_exceptions();
    }

    exceptions_handler(exceptions_handler const &) = delete;
    exceptions_handler &operator=(exceptions_handler const &) = delete;
};



JNIEnv *env_instance();


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


    struct jvm2cpp_processor_t
    {
        explicit jvm2cpp_processor_t(struct_runtime_type_desc_ptr desc, jobject_ptr src)
            : desc_(desc)
            , src_(src)
        {}

        template<typename T>
        void operator()(T &field_dst, char const *name)
        {
            auto field_desc = jvm_type_traits<T>::get_runtime_desc();
            jmethodID getter = desc_->getter(name, field_desc);

            typedef typename jvm_type_traits<T>::jvm_type jvm_type;
            
            auto interm = call_method<jvm_type>(src_, getter);

            field_dst = jvm2cpp<T>(interm);
        }

    private:
        struct_runtime_type_desc_ptr desc_;
        jobject_ptr src_;
    };

} // namespace detail

template<typename T>
T cpp2jvm(T src, std::enable_if_t<jvm_type_traits<T>::is_primitive> * = nullptr)
{
	return src;
}

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

template<typename T>
jobject_ptr cpp2jvm(optional<T> const &src, enable_if_primitive_t<T> * = nullptr)
{
    if (!src)
        return wrap_null();

    struct_runtime_type_desc_ptr runtime_desc = jvm_type_traits<optional<T>>::get_runtime_desc();
    //return runtime_desc->create();

    auto sig = make_method_signature<void, T>();
    auto ctor = runtime_desc->find_method("<init>", sig);
    auto obj = env_instance()->NewObject(runtime_desc->get_class(), ctor, *src);
    process_jvm_exceptions();
    return wrap(obj);
}
    

template<typename T>
jobject_ptr cpp2jvm(optional<T> const &src, enable_if_not_primitive_t<T> * = nullptr)
{
    if (!src)
        return wrap_null();

    return cpp2jvm(*src);
}

jobject_ptr cpp2jvm(string const &src);

template<typename T>
T jvm2cpp(typename jvm_type_traits<T>::jvm_type src);

namespace detail
{
    template<typename T>
    void jvm2cpp_impl(T src, T& dst, enable_if_primitive_t<T> * = nullptr)
	{
        dst = src;
	}

    template<typename T>
    void jvm2cpp_impl(jobject_ptr src, T &dst, enable_if_not_primitive_t<T> * = nullptr)
	{
        typedef jvm_type_traits<T> traits;
        struct_runtime_type_desc_ptr runtime_desc = traits::get_runtime_desc();

        if (!src)
        {
            std::stringstream ss;
            ss << "Trying to convert null '" << runtime_desc->java_name() << "' to non-optional cpp struct";
            throw jvm_interop_error(ss.str());
        }
		    

        detail::jvm2cpp_processor_t proc(runtime_desc, src);
        reflect(proc, dst);
    }
    
    template<typename T>
    void jvm2cpp_impl(jobject_ptr src, optional<T> &dst, enable_if_primitive_t<T> * = nullptr)
    {
        if (!src)
        {
            dst = boost::none;
            return;
        }

        struct_runtime_type_desc_ptr runtime_desc = jvm_type_traits<optional<T>>::get_runtime_desc();

        auto sig = make_method_signature<T>();
        auto m = runtime_desc->find_method("getValue", sig);

        dst = call_method<T>(src, m);
    }

    template<typename T>
    void jvm2cpp_impl(jobject_ptr src, optional<T> &dst, enable_if_not_primitive_t<T> * = nullptr)
    {
        if (!src)
        {
            dst = boost::none;
            return;
        }

        dst = jvm2cpp<T>(src);
    }

	
    void jvm2cpp_impl(jobject_ptr src, string &dst);
} // namespace detail

template<typename T>
T jvm2cpp(typename jvm_type_traits<T>::jvm_type src)
{
    T dst;
    detail::jvm2cpp_impl(src, dst);
    return dst;
}






template<typename Ret, typename... Args>
string make_method_signature()
{
    return make_method_signature(
        jvm_type_traits<Ret>::get_runtime_desc(),
        { jvm_type_traits<Args>::get_runtime_desc()... }
    );
}

namespace detail
{
	template<typename T>
    struct method_caller;

#define JVM_INTEROP_DEFINE_METHOD_CALLER(type, Type) \
	template<>																				   \
	struct method_caller<type>									     						   \
	{																						   \
		template<typename... Args>															   \
		static type call(jobject_ptr obj, jmethodID method, Args&&... args)			    	   \
		{																					   \
			exceptions_handler h;                                                              \
            return wrap(env_instance()->Call ## Type ## Method(obj->get_p(), method, unwrap(args)...)); \
		}																					   \
	};

template<>																				   
struct method_caller<void>									     						   
{																						   
    template<typename... Args>															   
    static void call(jobject_ptr obj, jmethodID method, Args&&... args)			    	   
    {																					   
        exceptions_handler h;                                                              
        env_instance()->CallVoidMethod(obj->get_p(), method, unwrap(args)...); 
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
JVM_INTEROP_DEFINE_METHOD_CALLER(jobject_ptr, Object)

#undef JVM_INTEROP_DEFINE_METHOD_CALLER


} // namespace detail


template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args)
{
	return detail::method_caller<T>::call(obj, method, std::forward<Args>(args)...);
}

} // namespace jvm_interop
