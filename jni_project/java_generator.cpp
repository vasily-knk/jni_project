#include "stdafx.h"
#include "java_generator.h"

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

} // namespace


string make_getter_name(char const *name)
{
    return "get" + fix_field_name(name);
}

string make_setter_name(char const *name)
{
    return "set" + fix_field_name(name);
}

namespace
{
    string indent(int num = 1)
    {
        std::stringstream ss;
        for (int i = 0; i < num; ++i)
            ss << "    ";

        return ss.str();
    }

    void generate_java_code(struct_fields_t const &st, std::ostream &out)
    {
        auto split_package_name = st.rttd->split_name();

        const string class_name = split_package_name.back();
        split_package_name.pop_back();

        if (!split_package_name.empty())
        {
            string package_name = boost::algorithm::join(split_package_name, ".");
            out << "package " << package_name << ";\n\n";
        }

        out << "public class " << class_name << " {\n";

        // fields
        
        for (auto const &field : st.fields)
        {
            out << indent() << "private " << field.rttd->java_name() << " " << field.name << ";\n";
        }

        out << "\n";

        // getters, setters
        for (auto const &field : st.fields)
        {
            auto const &java_name = field.rttd->java_name();
            
            out << indent() << "public " << java_name << " " << make_getter_name(field.name.c_str()) << "() {\n";
            out << indent(2) << "return this." << field.name << ";\n";
            out << indent() << "}\n\n";


            out << indent() << "public void " << make_setter_name(field.name.c_str()) << "(" << java_name << " val) {\n";
            out << indent(2) << "this." << field.name << " = val;\n";
            out << indent() << "}\n\n";
        }


        out << "}\n";
    }


} // namespace

void generate_java_structs(struct_fields_map_t const &structs, string const &path_str)
{
    fs::path root_path(path_str);

    for (auto const &r : structs)
    {
        struct_fields_t const &st = r.second;

        string const &lookup_name = st.rttd->lookup_name();

        const fs::path file_path = root_path / (lookup_name + ".java");
        const fs::path dir_path = file_path.parent_path();

        fs::create_directories(dir_path);

        std::ofstream out(file_path.string());
        generate_java_code(st, out);

    }
}


} // namespace jvm_interop
