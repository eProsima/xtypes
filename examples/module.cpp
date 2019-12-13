#include <xtypes/xtypes.hpp>
#include <xtypes/Module.hpp>

#include <iostream>

using namespace eprosima::xtypes;

int main()
{
    std::string idl_spec = R"(
        struct InnerType
        {
            uint32 im1;
            float im2;
        };
    )";

    std::map<std::string, DynamicType::Ptr> from_idl = idl::parse(idl_spec).get_all_types();
    const StructType& inner = static_cast<const StructType&>(*from_idl.at("InnerType"));

    StructType outer("OuterType");
    outer.add_member("om1", StringType());

    Module root;
    Module& submod_a = root.create_submodule("a");
    Module& submod_b = root.create_submodule("b");
    Module& submod_aa = submod_a.create_submodule("a");
    root.structure(inner);
    submod_aa.structure(outer);

    std::cout << std::boolalpha;
    std::cout << "Does a::a::OuterType exists?: " << root.has_structure("a::a::OuterType") << std::endl;
    std::cout << "Does ::InnerType exists?: " << root.has_structure("::InnerType") << std::endl;
    std::cout << "Does InnerType exists?: " << root.has_structure("InnerType") << std::endl;
    std::cout << "Does OuterType exists?: " << root.has_structure("OuterType") << std::endl;

    DynamicData module_data(root["a"]["a"].structure("OuterType")); // ::a::a::OuterType
    module_data["om1"] = "This is a string.";

    return 0;
}
