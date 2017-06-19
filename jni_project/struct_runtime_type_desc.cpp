#include "stdafx.h"

#include "jvm_interop.h"
#include "java_generator.h"

namespace jvm_interop
{
	struct struct_runtime_type_desc_impl
		: struct_runtime_type_desc
	{
		explicit struct_runtime_type_desc_impl(char const *dot_separated_name, bool /*needs_generation*/)
		{
			boost::algorithm::split(split_name_, dot_separated_name, boost::is_any_of("."));

			lookup_name_ = boost::algorithm::join(split_name_, "/");
			signature_ = "L" + lookup_name_ + ";";

			java_name_ = boost::algorithm::join(split_name_, ".");

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

        jmethodID find_method(string const &method_name, string const &sig) override
        {
            jclass clazz = get_jclass();

            return get_method(clazz, method_name.c_str(), sig.c_str());
        }


        vector<string> const &split_name() override
        {
            return split_name_;
        }

    private:

		jclass get_jclass() const
		{
			if (clazz_)
				return clazz_;

            clazz_ = find_class(lookup_name_.c_str());

			return clazz_;
		}

	private:
        vector<string> split_name_;

        string signature_;
		string java_name_;
		string lookup_name_;

		mutable jclass clazz_ = nullptr;
	};

	struct_runtime_type_desc_ptr create_struct_runtime_type_desc(char const *dot_separated_name, bool needs_generation)
	{
		return make_shared<struct_runtime_type_desc_impl>(dot_separated_name, needs_generation);
	}

} // namespace jvm_interop