#pragma once

#include "jvm_interop.h"

namespace jvm_interop
{
    struct field_desc_t
    {
        field_desc_t(string const &name, runtime_type_desc_ptr rttd)
            : name(name)
            , rttd(rttd)
        {}

        string name;
        runtime_type_desc_ptr rttd;
    };

    struct struct_fields_t
    {
        struct_runtime_type_desc_ptr rttd;
        vector<field_desc_t> fields;
    };

    typedef std::map<string, struct_fields_t> struct_fields_map_t;
    
    template<typename T>
    void append_struct_fields(struct_fields_map_t &dst);

    struct append_struct_fields_processor
    {
        explicit append_struct_fields_processor(struct_runtime_type_desc_ptr rttd, struct_fields_map_t &dst)
            : dst_(dst)
            , struct_dst_(dst[rttd->java_name()])
        {
            struct_dst_.rttd = rttd;
        }

        template<typename T>
        void operator()(T const &field, char const *name) const
        {
            append_field(field, name);
            process_type<T>();
        }
        
    private:
        template<typename T>
        void process_type(optional<T> const &)
        {
            process_type<T>();
        }

        template<typename T>
        void process_type(enable_if_primitive_t<T> * = nullptr) const
        {
        }


        template<typename T>
        void process_type(enable_if_not_primitive_t<T> * = nullptr) const
        {
            process_not_primitive_type<T>();
        }
    
    private:
        template<typename T>
        void process_not_primitive_type(std::enable_if_t<is_array<T>::value> * = nullptr) const
        {
            process_type<T::value_type>();
        }

        template<typename T>
        void process_not_primitive_type(std::enable_if_t<!is_array<T>::value> * = nullptr) const
        {
            process_non_array_type<T>();
        }

    private:
        template<typename T>
        void process_non_array_type(std::enable_if_t<!jvm_type_traits<T>::needs_generation> * = nullptr) const
        {
        }

        template<typename T>
        void process_non_array_type(std::enable_if_t<jvm_type_traits<T>::needs_generation> * = nullptr) const
        {
            append_struct_fields<T>(dst_);
        }


    private:
        template<typename T>
        void append_field(T const & /*field*/, char const *name) const
        {
            auto desc = get_type_desc<T>();
            struct_dst_.fields.emplace_back(name, desc);
        }

    private:
        struct_fields_map_t &dst_;
        struct_fields_t &struct_dst_;
    };


    template<typename T>
    void append_struct_fields(struct_fields_map_t &dst)
    {
        auto desc = get_generated_type_desc<T>();
        string const &java_name = desc->java_name();
        if (dst.count(java_name) != 0)
            return;

        append_struct_fields_processor proc(desc, dst);

        T s;
        reflect(proc, s);
    }


    void generate_java_structs(struct_fields_map_t const &structs, string const &path);

    string make_getter_name(char const *name);
    string make_setter_name(char const *name);


    
} // namespace jvm_interop
