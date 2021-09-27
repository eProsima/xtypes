#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

#include <iostream>

using namespace eprosima::xtypes;

int main()
{

    // Loading idl
    try
    {
        std::cout << "LOADING" << std::endl;
        std::cout << "-------" << std::endl;

        idl::Context context = idl::parse_file("resources/example.idl");

        if (context.success)
        {
            std::cout << "Context loaded correctly" << std::endl;
        }
        else
        {
            std::cout << "Error loading idl file" << std::endl;
            return 1;
        }

        // Getting a map with all the structures
        std::map<std::string, DynamicType::Ptr> types_map = context.module().get_all_types();

        std::cout << std::endl << "Types loaded: " << types_map.size() << std::endl;
        for (auto type_name : types_map)
        {
            std::cout << "  " << type_name.first << std::endl;
        }

        std::cout << std::endl << "INTROSPECTION" << std::endl;
        std::cout << "-------------" << std::endl;

        // Introspect types
        for (auto struct_type : types_map)
        {
            if (context.module().has_structure(struct_type.first))
            {
                const StructType& type = context.module().structure(struct_type.first);
                std::cout << struct_type.first << ": " << type << std::endl;
            }
            else if (context.module().has_enum_32(struct_type.first))
            {
                const EnumerationType<uint32_t>& enum_type = context.module().enum_32(struct_type.first);
                std::cout << struct_type.first << ": " << enum_type << std::endl;
            }
            else if (context.module().has_submodule(struct_type.first))
            {
                const std::shared_ptr<idl::Module> submodule_type = context.module().submodule(struct_type.first);
                std::cout << struct_type.first << ": " << submodule_type << std::endl;
            }
            else
            {
                std::cout << "Unkown type: " << struct_type.first << std::endl;
            }
        }

        std::cout << "ACCESS" << std::endl;
        std::cout << "------" << std::endl;

        const StructType& my_complex_struct = context.module().structure("MyComplexType");
        DynamicData data(my_complex_struct);

        /////
        // Access primitive type

        // set value
        data["myPrimitiveType"]["my_float"] = 3.1415f; // Important: the 'f' to set as float and not double

        // get value
        std::cout << "Value inside complex struct: "
                  << data["myPrimitiveType"]["my_float"].value<float>() << std::endl;

        /////
        // Access complex type in collection

        // Types required
        const StructType& my_string_struct = context.module().structure("MyStringType");
        StringType string_type;

        // set value
        DynamicData string_key(string_type);
        string_key = "key_on_map";

        DynamicData string_data(my_string_struct);
        string_data["my_string"] = "value_on_map";

        data["myCollectionType"]["my_map"][string_key] = string_data;

        // get value
        DynamicData new_string_data(my_string_struct);
        new_string_data = data["myCollectionType"]["my_map"][string_key];
        std::cout << "Value inside collection inside complex struct: '"
                  << new_string_data["my_string"].value<std::string>() << "'" << std::endl;

        /////
        // Access primitive type

        // Types required
        const EnumerationType<uint32_t>& my_enum_type = context.module().enum_32("MyEnumerationType");

        // set value
        DynamicData my_enum_data(my_enum_type);
        my_enum_data = my_enum_type.value("BLUE");

        data["mySubmoduleType"] = my_enum_data;

        // get value
        std::cout << "Value inside submodule struct: "
                  << data["mySubmoduleType"]["my_enum"].value<uint32_t>() << std::endl;
    }
    catch(const std::runtime_error& e)
    {
        std::cout << "Exception catched: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
