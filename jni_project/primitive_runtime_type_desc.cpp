#include "stdafx.h"

#include "jvm_interop.h"

namespace jvm_interop
{

struct primitive_runtime_type_desc_impl
	: runtime_type_desc
{
	primitive_runtime_type_desc_impl(char const *java_name, char sig)
		: java_name_(java_name)
		, signature_(string() + sig)
	{
		
	}

	string const &signature() override
	{
		return signature_;
	}

	string const &java_name() override
	{
		return java_name_;
	}

private:
	string java_name_;
	string signature_;
};


runtime_type_desc_ptr create_primitive_runtime_type_desc(char const *java_name, char sig)
{
	return make_shared<primitive_runtime_type_desc_impl>(java_name, sig);
}

	
} // namespace jvm_interop