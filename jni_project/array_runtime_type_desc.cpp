#include "stdafx.h"

#include "jvm_interop.h"

namespace jvm_interop
{

struct array_runtime_type_desc_impl
    : runtime_type_desc
{
    explicit array_runtime_type_desc_impl(runtime_type_desc_ptr item_desc)
        : signature_("[" + item_desc->signature())
        , java_name_(item_desc->java_name() + "[]")
    {
    }
    
    
    string const& signature() override
    {
        return signature_;
    }


    string const& java_name() override
    {
        return java_name_;
    }

private:
    string signature_;
    string java_name_;
};

runtime_type_desc_ptr create_array_runtime_type_desc(runtime_type_desc_ptr item_desc)
{
    return make_shared<array_runtime_type_desc_impl>(item_desc);
}


} // namespace jvm_interop