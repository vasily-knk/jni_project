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
struct non_primitive_type_desc;
struct struct_runtime_type_desc;


typedef shared_ptr<runtime_type_desc> runtime_type_desc_ptr;
typedef shared_ptr<non_primitive_type_desc> non_primitive_type_desc_ptr;
typedef shared_ptr<struct_runtime_type_desc> struct_runtime_type_desc_ptr;

struct runtime_type_desc
{
	virtual ~runtime_type_desc() = default;

	virtual string const &signature() = 0;
	virtual string const &java_name() = 0;
};

struct non_primitive_type_desc
    : runtime_type_desc
{
    virtual string const &lookup_name() = 0;
    virtual jclass get_class() = 0;
};

struct struct_runtime_type_desc
	: non_primitive_type_desc
{
    virtual jobject_ptr create() = 0;

	virtual jmethodID getter(char const *field_name, runtime_type_desc_ptr runtime_type_desc) = 0;
	virtual jmethodID setter(char const *field_name, runtime_type_desc_ptr runtime_type_desc) = 0;

    virtual jmethodID find_method(string const &method_name, string const &sig) = 0;

    virtual vector<string> const &split_name() = 0;
};


runtime_type_desc_ptr create_primitive_runtime_type_desc(char const *java_name, char sig);
non_primitive_type_desc_ptr create_array_runtime_type_desc(runtime_type_desc_ptr item_desc);
struct_runtime_type_desc_ptr create_struct_runtime_type_desc(char const *dot_separated_name);

template<typename T, typename Enable = void>
struct jvm_type_traits;


template<typename T>
struct jvm_primitive_type_traits
{
	typedef T jvm_type;
    typedef T unwrapped_jvm_type;
    static const bool is_primitive = true;
    static const bool needs_generation = false;

    typedef runtime_type_desc_ptr rttd_type;
};

struct jvm_struct_type_traits
{
    typedef jobject_ptr jvm_type;
    typedef jobject unwrapped_jvm_type;
    static const bool is_primitive = false;

    typedef struct_runtime_type_desc_ptr rttd_type;
};

struct jvm_builtin_type_traits
    : jvm_struct_type_traits
{
    static const bool needs_generation = false;
};

struct jvm_user_type_traits
    : jvm_struct_type_traits
{
    static const bool needs_generation = true;
};

template<typename T>
typename jvm_type_traits<T>::rttd_type get_type_desc()
{
    return jvm_type_traits<T>::get_explicit_type_desc();
}

template<typename T>
std::enable_if_t<jvm_type_traits<T>::needs_generation, struct_runtime_type_desc_ptr>
get_generated_type_desc()
{
    return jvm_type_traits<T>::get_explicit_generated_type_desc();
}

template<typename T>
using enable_if_primitive_t = std::enable_if_t<jvm_type_traits<T>::is_primitive>;

template<typename T>
using enable_if_not_primitive_t = std::enable_if_t<!jvm_type_traits<T>::is_primitive>;

template<typename T>
using rttd_t = typename jvm_type_traits<T>::rttd_type;

template<typename T>
rttd_t<T> get_rttd();



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


#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_TRAITS(cpp_type) \
    template<> struct jvm_interop::jvm_type_traits<cpp_type> : jvm_interop::jvm_primitive_type_traits<cpp_type> {};

#define JVM_INTEROP_DECLARE_BUILTIN_TYPE_TRAITS(cpp_type) \
    template<> struct jvm_interop::jvm_type_traits<cpp_type> : jvm_interop::jvm_builtin_type_traits {};

#define JVM_INTEROP_DECLARE_USER_TYPE_TRAITS(cpp_type) \
    template<> struct jvm_interop::jvm_type_traits<cpp_type> : jvm_interop::jvm_user_type_traits {};



#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_DESC(cpp_type, java_name, sig) \
    template<> \
    inline jvm_interop::runtime_type_desc_ptr jvm_interop::get_type_desc<cpp_type>() \
	{ \
		static const auto p = jvm_interop::create_primitive_runtime_type_desc(java_name, sig); \
		return p; \
	} 

#define JVM_INTEROP_DECLARE_STRUCT_TYPE_DESC(cpp_type, dot_separated_name) \
    template<> \
    inline jvm_interop::struct_runtime_type_desc_ptr jvm_interop::get_type_desc<cpp_type>() \
	{ \
		static const auto p = jvm_interop::create_struct_runtime_type_desc(dot_separated_name); \
		return p; \
	} 

#define JVM_INTEROP_DECLARE_GENERATED_TYPE_DESC(cpp_type, dot_separated_name) \
    template<> \
    inline jvm_interop::struct_runtime_type_desc_ptr jvm_interop::get_generated_type_desc<cpp_type>() \
	{ \
		static const auto p = jvm_interop::create_struct_runtime_type_desc(dot_separated_name); \
		return p; \
	} 

template<typename T>
char const *get_boxed_name();


#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_BOXED_NAME(cpp_type, boxed_name) \
    template<> \
    inline char const *jvm_interop::get_boxed_name<cpp_type>() \
	{ \
		static char const * const s = boxed_name; \
        return s; \
	} 

#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_BASE(cpp_type, java_name, sig) \
    JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_TRAITS(cpp_type) \
    JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_DESC(cpp_type, java_name, sig)

#define JVM_INTEROP_DECLARE_BUILTIN_TYPE_BASE(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_BUILTIN_TYPE_TRAITS(cpp_type) \
    JVM_INTEROP_DECLARE_STRUCT_TYPE_DESC(cpp_type, dot_separated_name)

#define JVM_INTEROP_DECLARE_USER_TYPE_BASE(cpp_type, dot_separated_name, generated_dot_separated_name) \
    JVM_INTEROP_DECLARE_USER_TYPE_TRAITS(cpp_type) \
    JVM_INTEROP_DECLARE_STRUCT_TYPE_DESC(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_GENERATED_TYPE_DESC(cpp_type, generated_dot_separated_name) 



#define JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(cpp_type, java_name, sig, boxed_name) \
    JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_BASE(cpp_type, java_name, sig) \
    JVM_INTEROP_DECLARE_PRIMITIVE_TYPE_BOXED_NAME(cpp_type, boxed_name)

#define JVM_INTEROP_DECLARE_BUILTIN_TYPE(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_BUILTIN_TYPE_BASE(cpp_type, dot_separated_name) 
    //JVM_INTEROP_DECLARE_BUILTIN_TYPE_BASE(optional<cpp_type>, dot_separated_name) 


#define JVM_INTEROP_DECLARE_USER_TYPE_EXT(cpp_type, dot_separated_name, generated_dot_separated_name) \
    JVM_INTEROP_DECLARE_USER_TYPE_BASE(cpp_type, dot_separated_name, generated_dot_separated_name) 
    //JVM_INTEROP_DECLARE_USER_TYPE_BASE(optional<cpp_type>, dot_separated_name, generated_dot_separated_name) 

#define JVM_INTEROP_DECLARE_USER_TYPE(cpp_type, dot_separated_name) \
    JVM_INTEROP_DECLARE_USER_TYPE_EXT(cpp_type, dot_separated_name, dot_separated_name)


// optional
template<typename T>
struct jvm_type_traits<optional<T>, enable_if_primitive_t<T>>
    : jvm_builtin_type_traits
{
    static struct_runtime_type_desc_ptr get_explicit_type_desc()
    {
        static const auto p = create_struct_runtime_type_desc(get_boxed_name<T>());
        return p;
    }
};

template<typename T>
struct jvm_type_traits<optional<T>, enable_if_not_primitive_t<T>>
    : jvm_type_traits<T>
{
    typedef typename jvm_type_traits<T>::rttd_type rttd_type;
    
    static rttd_type get_explicit_type_desc()
    {
        return get_type_desc<T>();
    }
};



template<typename T>
struct jvm_type_traits<vector<T>>
{
    typedef T value_type;

    typedef jobject_ptr jvm_type;
    typedef jobject unwrapped_jvm_type;
    static const bool is_primitive = false;
    static const bool needs_generation = jvm_type_traits<value_type>::needs_generation;

    typedef non_primitive_type_desc_ptr rttd_type;

    static rttd_type get_explicit_type_desc()
    {
        static rttd_type p = create_array_runtime_type_desc(get_type_desc<value_type>());
        return p;
    }

    static struct_runtime_type_desc_ptr get_explicit_generated_type_desc()
    {
        return get_generated_type_desc<value_type>();
    }
};


JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jboolean, "boolean", 'Z', "java.lang.Boolean")
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jbyte   , "byte"   , 'B', "java.lang.Byte")
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jchar   , "char"   , 'C', "java.lang.Char") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jdouble , "double" , 'D', "java.lang.Double") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jfloat  , "float"  , 'F', "java.lang.Float") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jint    , "int"    , 'I', "java.lang.Int") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jlong   , "long"   , 'J', "java.lang.Long") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(jshort  , "short"  , 'S', "java.lang.Short") 
JVM_INTEROP_DECLARE_PRIMITIVE_TYPE(void    , "void"   , 'V', "java.lang.Void") 

JVM_INTEROP_DECLARE_BUILTIN_TYPE(string, "java.lang.String")


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
            auto field_desc = get_type_desc<T>();
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
            auto field_desc = get_type_desc<T>();
            jmethodID getter = desc_->getter(name, field_desc);

            field_dst = call_method<T>(src_, getter);
        }

    private:
        struct_runtime_type_desc_ptr desc_;
        jobject_ptr src_;
    };


    template<typename T>
    struct jni_array_creator_traits;
//    void SetLongArrayRegion(jlongArray array, jsize start, jsize len,
//        const jlong *buf) {

#define JVM_INTEROP_DEFINE_ARRAY_CREATOR(type, Type)                            \
    template<>                                                                  \
    struct jni_array_creator_traits<type>                                       \
    {                                                                           \
        typedef type ret_type;                                                  \
        using array_type = type ## Array;                                       \
        using array_creator_fn = array_type(JNIEnv::*)(jsize);                  \
        using region_setter_fn = void(JNIEnv::*)(array_type, jsize, jsize, ret_type const *); \
        using region_getter_fn = void(JNIEnv::*)(array_type, jsize, jsize, ret_type *); \
                                                                                \
        static array_creator_fn get_array_creator()                             \
        {                                                                       \
            return &JNIEnv::New ## Type ## Array;                               \
        }                                                                       \
                                                                                \
        static region_setter_fn get_region_setter()                             \
        {                                                                       \
            return &JNIEnv::Set ## Type ## ArrayRegion;                         \
        }                                                                       \
                                                                                \
        static region_getter_fn get_region_getter()                             \
        {                                                                       \
            return &JNIEnv::Get ## Type ## ArrayRegion;                         \
        }                                                                       \
    };


    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jboolean, Boolean)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jbyte, Byte)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jchar, Char)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jdouble, Double)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jfloat, Float)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jint, Int)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jlong, Long)
    JVM_INTEROP_DEFINE_ARRAY_CREATOR(jshort, Short)


#undef JVM_INTEROP_DEFINE_ARRAY_CREATOR    

} // namespace detail






    
template<typename T>
T cpp2jvm(T src, std::enable_if_t<jvm_type_traits<T>::is_primitive> * = nullptr)
{
	return src;
}

template<typename T>
jobject_ptr cpp2jvm(T const &src, std::enable_if_t<!jvm_type_traits<T>::is_primitive> * = nullptr)
{
    struct_runtime_type_desc_ptr runtime_desc = get_type_desc<T>();

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

    struct_runtime_type_desc_ptr runtime_desc = get_type_desc<optional<T>>();
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

template<typename T>
jobject_ptr cpp2jvm_array(T const &src, enable_if_primitive_t<typename T::value_type> * = nullptr)
{
    typedef typename T::value_type value_type;
    
    auto creator = detail::jni_array_creator_traits<value_type>::get_array_creator();
    auto region_setter = detail::jni_array_creator_traits<value_type>::get_region_setter();

    jsize sz = jsize(src.size());

    auto env = env_instance();
    
    auto dst = std::invoke(creator, env, sz);
    std::invoke(region_setter, env, dst, 0, sz, src.data());
    
    return wrap(dst);
}

template<typename T>
jobject_ptr cpp2jvm_array(T const &src, enable_if_not_primitive_t<typename T::value_type> * = nullptr)
{
    auto desc = get_type_desc<typename T::value_type>();
    
    jsize sz(src.size());

    auto env = env_instance();
    auto dst = env->NewObjectArray(sz, desc->get_class(), nullptr);
    for (jsize i = 0; i < sz; ++i)
    {
        jobject_ptr elem = cpp2jvm(src.at(i));
        env->SetObjectArrayElement(dst, i, unwrap(elem));
    }

    return wrap(dst);
}

template<typename T>
jobject_ptr cpp2jvm(vector<T> const &src)
{
    return cpp2jvm_array(src);
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
    void jvm2cpp_impl(jobject_ptr src, T &dst, std::enable_if_t<!jvm_type_traits<T>::is_primitive> * = nullptr)
	{
        struct_runtime_type_desc_ptr runtime_desc = get_type_desc<T>();

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

        struct_runtime_type_desc_ptr runtime_desc = get_type_desc<optional<T>>();

        auto sig = make_method_signature<T>();

        string name = get_type_desc<T>()->java_name() + "Value";

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

    template<typename Arr>
    void jvm2cpp_array(jobject_ptr src, Arr &dst, enable_if_primitive_t<typename Arr::value_type> * = nullptr)
    {
        typedef typename Arr::value_type value_type;

        typedef typename detail::jni_array_creator_traits<value_type>::array_type array_type;
        
        auto src_arr = reinterpret_cast<array_type>(unwrap(src));
        auto env = env_instance();
        jsize len = env->GetArrayLength(src_arr);
        dst.resize(len);

        auto region_getter = detail::jni_array_creator_traits<value_type>::get_region_getter();

        std::invoke(region_getter, env, src_arr, 0, len, dst.data());
    }

    template<typename Arr>
    void jvm2cpp_array(jobject_ptr src, Arr &dst, enable_if_not_primitive_t<typename Arr::value_type> * = nullptr)
    {
        typedef typename Arr::value_type value_type;

        auto src_arr = reinterpret_cast<jobjectArray>(unwrap(src));
        auto env = env_instance();
        jsize len = env->GetArrayLength(src_arr);
        dst.resize(len);

        for (jsize i = 0; i < len; ++i)
        {
            auto src_elem = wrap(env->GetObjectArrayElement(src_arr, i));
            dst.at(i) = jvm2cpp<value_type>(src_elem);
        }
    }

    template<typename T>
    void jvm2cpp_impl(jobject_ptr src, vector<T> &dst)
    {
        if (!src)
        {
            std::stringstream ss;
            ss << "Trying to convert null '" << get_type_desc<T>()->java_name() << "' to non-optional cpp array";
            throw jvm_interop_error(ss.str());
        }

        jvm2cpp_array(src, dst);
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
        get_type_desc<Ret>(),
        { get_type_desc<Args>()... }
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
        JVM_INTEROP_DEFINE_METHOD_CALLER(jbyte, Byte)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jchar, Char)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jdouble, Double)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jfloat, Float)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jint, Int)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jlong, Long)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jshort, Short)
        JVM_INTEROP_DEFINE_METHOD_CALLER(jobject, Object)
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

        Ret operator()(Args const &... args) const
        {
            return call_method<Ret>(obj, id, args...);
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

    Ret operator()(Args const &... args) const
    {
        return call_static_method<Ret>(clazz, id, args...);
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
