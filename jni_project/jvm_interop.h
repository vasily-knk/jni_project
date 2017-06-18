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

    virtual jmethodID find_method(string const &method_name, string const &sig) = 0;
};


runtime_type_desc_ptr create_primitive_runtime_type_desc(char const *java_name, char sig);
struct_runtime_type_desc_ptr create_struct_runtime_type_desc(char const *dot_separated_name);

template<typename T>
struct jvm_type_traits;


template<typename T>
struct jvm_primitive_type_traits
{
	typedef T jvm_type;
    typedef T unwrapped_jvm_type;
    static const bool is_primitive = true;
};

template<typename T>
struct jvm_struct_type_traits
{
	typedef jobject_ptr jvm_type;
    typedef jobject unwrapped_jvm_type;
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

jobject unwrap(jobject_ptr const &p);

template<typename T>
T unwrap(T val, enable_if_primitive_t<T> * = nullptr)
{
    return val;
}

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

#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(cpp_type, java_name, sig, boxed_name) \
	template<> \
	struct jvm_interop::jvm_type_traits<cpp_type> \
		: jvm_interop::jvm_primitive_type_traits<cpp_type> \
	{ \
		static jvm_interop::runtime_type_desc_ptr get_runtime_desc() \
		{ \
			static const auto p = jvm_interop::create_primitive_runtime_type_desc((java_name), (sig)); \
			return p; \
		} \
	}; \
    JVM_INTEROP_DECLARE_STRUCT_TYPE_NO_OPTIONAL(optional<cpp_type>, boxed_name) 



    
#define JVM_INTEROP_DECLARE_STRUCT_TYPE(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_STRUCT_TYPE_NO_OPTIONAL(cpp_type, dot_separated_name) \
	template<> \
	struct jvm_interop::jvm_type_traits<optional<cpp_type>> \
        : jvm_interop::jvm_type_traits<cpp_type> {};






JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jboolean, "boolean", 'Z', "java.lang.Boolean") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jbyte   , "byte"   , 'B', "java.lang.Byte") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jchar   , "char"   , 'C', "java.lang.Char") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jdouble , "double" , 'D', "java.lang.Double") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jfloat  , "float"  , 'F', "java.lang.Float") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jint    , "int"    , 'I', "java.lang.Int") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jlong   , "long"   , 'J', "java.lang.Long") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jshort  , "short"  , 'S', "java.lang.Short") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(void    , "void"   , 'V', "java.lang.Void") 

JVM_INTEROP_DECLARE_STRUCT_TYPE(string, "java.lang.String")


template<typename T>
runtime_type_desc_ptr get_runtime_type_desc()
{
	return jvm_type_traits<T>::get_runtime_desc();
}



template<typename T>
using jvm_type_t = typename jvm_type_traits<T>::jvm_type;

template<typename T>
using unwrapped_jvm_type_t = typename jvm_type_traits<T>::unwrapped_jvm_type;

static_assert(std::is_same<jvm_interop::unwrapped_jvm_type_t<string>, jobject>::value, "AAA");
static_assert(std::is_same<jvm_interop::jvm_type_t<string>, jobject_ptr>::value, "AAA");

template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args);

string make_method_signature(runtime_type_desc_ptr ret, vector<runtime_type_desc_ptr> args);

template<typename Ret, typename... Args>
string make_method_signature();
void process_jvm_exceptions();



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
            auto field_desc = jvm_type_traits<T>::get_runtime_desc();
            jmethodID setter = desc_->setter(name, field_desc);

            call_method<void>(dst_, setter, field_src);
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

            field_dst = call_method<T>(src_, getter);
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
T jvm2cpp(jvm_type_t<T> src);


namespace detail
{
    void check_struct_jvm2cpp(jobject_ptr src, struct_runtime_type_desc_ptr);


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

        check_struct_jvm2cpp(src, runtime_desc);

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

        string name = jvm_type_traits<T>::get_runtime_desc()->java_name() + "Value";

        auto m = runtime_desc->find_method(name.c_str(), sig);

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
T jvm2cpp(jvm_type_t<T> src)
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



template<typename T>
struct jni_method_caller_traits;

#define JVM_INTEROP_DEFINE_METHOD_CALLER(type, Type)                            \
    template<>                                                                  \
    struct jni_method_caller_traits<type>                                       \
    {                                                                           \
        typedef type ret_type;                                                  \
                                                                                \
        using caller_fn = ret_type(JNIEnv::*)(jobject, jmethodID, ...);         \
        using static_caller_fn = ret_type(JNIEnv::*)(jclass, jmethodID, ...);   \
                                                                                \
        static caller_fn get_caller_fn()                                        \
        {                                                                       \
            return &JNIEnv::Call ## Type ## Method;                             \
        }                                                                       \
                                                                                \
        static static_caller_fn get_static_caller_fn()                          \
        {                                                                       \
            return &JNIEnv::CallStatic ## Type ## Method;                       \
        }                                                                       \
    };



JVM_INTEROP_DEFINE_METHOD_CALLER(jboolean, Boolean)
JVM_INTEROP_DEFINE_METHOD_CALLER(jbyte   , Byte)
JVM_INTEROP_DEFINE_METHOD_CALLER(jchar   , Char)
JVM_INTEROP_DEFINE_METHOD_CALLER(jdouble , Double)
JVM_INTEROP_DEFINE_METHOD_CALLER(jfloat  , Float)
JVM_INTEROP_DEFINE_METHOD_CALLER(jint    , Int)
JVM_INTEROP_DEFINE_METHOD_CALLER(jlong   , Long)
JVM_INTEROP_DEFINE_METHOD_CALLER(jshort  , Short)
JVM_INTEROP_DEFINE_METHOD_CALLER(jobject , Object)
JVM_INTEROP_DEFINE_METHOD_CALLER(void, Void)

#undef JVM_INTEROP_DEFINE_METHOD_CALLER
   

template<typename T, typename... Args>
T call_method_unwrapped_ret(jobject_ptr obj, jmethodID method, Args&&... args)
{
    typedef detail::jni_method_caller_traits<T> traits;
    auto fn = traits::get_caller_fn();

    return std::invoke(fn, env_instance(), unwrap(obj), method, unwrap(cpp2jvm(args))...);
}

template<typename T, typename... Args>
T call_static_method_unwrapped_ret(jclass clazz, jmethodID method, Args&&... args)
{
    typedef detail::jni_method_caller_traits<T> traits;
    auto fn = traits::get_static_caller_fn();

    return std::invoke(fn, env_instance(), clazz, method, unwrap(cpp2jvm(args))...);
}

template<typename T>
struct method_caller_t
{
    typedef unwrapped_jvm_type_t<T> unwrapped_type_t;
    typedef jvm_type_t<T> wrapped_type_t;

    template<typename... Args>
    static T call(jobject_ptr obj, jmethodID method, Args&&... args)
    {
        unwrapped_type_t ret = call_method_unwrapped_ret<unwrapped_type_t, Args...>(obj, method, std::forward<Args>(args)...);
        process_jvm_exceptions();

        return jvm2cpp<T>(wrap(ret));
    }
    
    template<typename... Args>
    static T call_static(jclass clazz, jmethodID method, Args&&... args)
    {
        unwrapped_type_t ret = call_static_method_unwrapped_ret<unwrapped_type_t, Args...>(clazz, method, std::forward<Args>(args)...);
        process_jvm_exceptions();

        return jvm2cpp<T>(wrap(ret));
    }
};

template<>
struct method_caller_t<void>
{
    template<typename... Args>
    static void call(jobject_ptr obj, jmethodID method, Args&&... args)
    {
        call_method_unwrapped_ret<void, Args...>(obj, method, std::forward<Args>(args)...);
        process_jvm_exceptions();
    }

    template<typename... Args>
    static void call_static(jclass clazz, jmethodID method, Args&&... args)
    {
        call_static_method_unwrapped_ret<void, Args...>(clazz, method, std::forward<Args>(args)...);
        process_jvm_exceptions();
    }

};


} // namespace detail


template<typename T, typename... Args>
T call_method(jobject_ptr obj, jmethodID method, Args&&... args)
{
    return detail::method_caller_t<T>::call(obj, method, std::forward<Args>(args)...);
}

template<typename T, typename... Args>
T call_static_method(jclass clazz, jmethodID method, Args&&... args)
{
    return detail::method_caller_t<T>::call_static(clazz, method, std::forward<Args>(args)...);
}

jclass find_class(char const *name);

void register_native_impl(jclass clazz, char const *name, char const *sig, void *fn_ptr);
    
template<typename T, typename... Args>
void register_native(jclass clazz, char const *name, void *fn_ptr)
{
    string sig = make_method_signature<T, Args...>();
    register_native_impl(clazz, name, sig.c_str(), fn_ptr);
}


template<typename Ret, typename... Args>
struct method_t
{
    struct caller
    {
        caller(const jobject_ptr& jobject_wrapper, jmethodID id)
            : obj(jobject_wrapper)
            , id(id)
        {}

        Ret operator()(Args&&...args) const
        {
            return call_method<Ret>(obj, id, std::forward<Args>(args)...);
        }

        jobject_ptr obj;
        jmethodID id;
    };

    explicit method_t(jmethodID id)
        : id(id)
    {}
    
    caller operator()(jobject_ptr obj) const
    {
        return caller(obj, id);
    }

private:
    jmethodID id;
};

template<typename Ret, typename... Args>
struct static_method_t
{
    explicit static_method_t(jclass clazz, jmethodID id)
        : clazz(clazz)
        , id(id)
    {}

    Ret operator()(Args&&...args) const
    {
        return call_static_method<Ret>(clazz, id, std::forward<Args>(args)...);
    }

private:
    jclass clazz;
    jmethodID id;
};

jmethodID get_method(jclass clazz, char const *name, char const *sig);
jmethodID get_static_method(jclass clazz, char const *method_name, char const *sig);

template<typename Ret, typename... Args>
method_t<Ret, Args...> get_method(jclass clazz, char const *name)
{
    const string sig = make_method_signature<Ret, Args...>();
    const jmethodID id = get_method(clazz, name, sig.c_str());
    return method_t<Ret, Args...>(id);
}

template<typename Ret, typename... Args>
static_method_t<Ret, Args...> get_static_method(jclass clazz, char const *name)
{
    const string sig = make_method_signature<Ret, Args...>();
    const jmethodID id = get_static_method(clazz, name, sig.c_str());
    return static_method_t<Ret, Args...>(clazz, id);
}


} // namespace jvm_interop
