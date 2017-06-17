#include "stdafx.h"

#include "jvm_interop.h"

namespace jvm_interop
{
    namespace
    {
        string fix_field_name(char const *name)
        {
            string result = name;

            if (!result.empty())
            {
                char &c = result.at(0);
                if (c >= 'a' && c <= 'z')
                    c += 'A' - 'a';
            }
            
            return result;
        }

        string make_getter_name(char const *name)
        {
            return "get" + fix_field_name(name);
        }

        string make_setter_name(char const *name)
        {
            return "set" + fix_field_name(name);
        }

    } // namespace

	struct struct_runtime_type_desc_impl
		: struct_runtime_type_desc
	{
		explicit struct_runtime_type_desc_impl(char const *dot_separated_name)
		{
			vector<string> s;
			boost::algorithm::split(s, dot_separated_name, boost::is_any_of("."));

			lookup_name_ = boost::algorithm::join(s, "/");
			signature_ = "L" + lookup_name_ + ";";

			java_name_ = boost::algorithm::join(s, ".");

		}
		
		
		string const& signature() override
		{
			return signature_;
		}

		string const& java_name() override
		{
			return java_name_;
		}

		string const& lookup_name() override
		{
			return lookup_name_;
		}

        jobject_ptr create() override
		{
			JNIEnv* env = env_instance();
			jclass clazz = get_jclass();
			jmethodID ctor = find_method("<init>", "()V");

            jobject_ptr obj = wrap(env->NewObject(clazz, ctor));

			process_jvm_exceptions();

			if (!obj)
			{
				std::stringstream ss;
				ss << "Constructor returned null for '" << lookup_name_ << "'";
				throw jvm_interop_error(ss.str());
			}

			return obj;
		}


		jclass get_class() override
		{
			return get_jclass();
		}

		jmethodID getter(char const* field_name, runtime_type_desc_ptr runtime_type_desc) override
		{
			auto sig = make_method_signature(runtime_type_desc, {  });

			return find_method(make_getter_name(field_name), sig);
		}

		jmethodID setter(char const* field_name, runtime_type_desc_ptr runtime_type_desc) override
		{
			auto sig = make_method_signature(get_runtime_type_desc<void>(), { runtime_type_desc });

			return find_method(make_setter_name(field_name), sig);
		}

	private:
		jmethodID find_method(string const &name, string const &sig) const
		{
			jclass clazz = get_jclass();

			JNIEnv* env = env_instance();
			jmethodID id = env->GetMethodID(clazz, name.c_str(), sig.c_str());

			if (!id)
			{
				std::stringstream ss;
				ss << "Method '" << name << "' with signature '" << sig << "' not found in '" << lookup_name_ << "'";
				throw jvm_interop_error(ss.str());
			}

			return id;
		}

		jclass get_jclass() const
		{
			if (clazz_)
				return clazz_;

			clazz_ = env_instance()->FindClass(lookup_name_.c_str());

			if (!clazz_)
			{
				std::stringstream ss;
				ss << "JVM class not found: '" << lookup_name_ << "'";
				throw jvm_interop_error(ss.str());
			}

			return clazz_;
		}

	private:
		string signature_;
		string java_name_;
		string lookup_name_;
		mutable jclass clazz_ = nullptr;
	};

	struct_runtime_type_desc_ptr create_struct_runtime_type_desc(char const *dot_separated_name)
	{
		return make_shared<struct_runtime_type_desc_impl>(dot_separated_name);
	}

} // namespace jvm_interop