#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

#include <iostream>

using namespace eprosima::xtypes;

int main()
{
    std::string idl_spec =
            R"(
        struct InnerType
        {
            uint32 im1;
            float im2;
        };
    )";

    idl::Context context = idl::parse(idl_spec);
    StructType inner = context.module().structure("InnerType");

    AliasType abool(primitive_type<bool>(), "bool");
    StructType outer("OuterType");
    outer.add_member(Member("om1", primitive_type<double>()).id(2));
    outer.add_member("om2", inner);
    outer.add_member("om3", StringType());
    outer.add_member("om4", WStringType(100));
    outer.add_member("om5", SequenceType(primitive_type<uint32_t>(), 5));
    outer.add_member("om6", SequenceType(inner));
    outer.add_member("om7", ArrayType(primitive_type<uint32_t>(), 4));
    outer.add_member("om8", ArrayType(inner, 4));
    outer.add_member("om9", SequenceType(SequenceType(primitive_type<uint32_t>(), 5), 3));
    outer.add_member("om10", ArrayType(ArrayType(primitive_type<uint32_t>(), 2), 3));
    outer.add_member("om11", abool);
    outer.add_member("om12", MapType(StringType(), inner));

    std::cout << idl::generate(inner) << std::endl;
    std::cout << idl::generate(outer) << std::endl;

    DynamicData data(outer);
    data["om1"] = 6.7;                                     //PrimitiveType<double>
    data["om2"]["im1"] = 42u;                              //PrimitiveType<uint32_t>
    data["om2"]["im2"] = 35.8f;                            //PrimitiveType<float>
    data["om3"] = "This is a string";                      //...
    data["om4"] = L"Also support unicode! \u263A";         //WStringType
    data["om5"].push(12u);                                 //SequenceType(PrimitiveType<uint32_t>)
    data["om5"].push(31u);                                 //...
    data["om5"].push(50u);                                 //...
    data["om5"][1] = 100u;                                 //...
    data["om6"].push(data["om2"]);                         //SequenceType(inner)
    data["om6"][0] = data["om2"];                          //...
    data["om7"][1] = 123u;                                 //ArrayType(PrimitiveType<uint32_t>)
    data["om8"][1] = data["om2"];                          //ArrayType(inner)
    data["om11"] = true;                                   //AliasType(PrimitiveType<bool>))
    StringType str_type;
    DynamicData map_key(str_type);
    map_key = "first";
    data["om12"][map_key] = data["om2"];                   //MapType(StringType(), inner)
    map_key = "second";
    data["om12"][map_key]["im1"] = 43u;                    //...
    data["om12"][map_key]["im2"] = 35.9f;                  //...

    std::cout << data.to_string() << std::endl; //See to_string() implementation as an example of data instrospection
    std::cout << "Now, print the same using the << operator overload:" << std::endl;
    std::cout << data << std::endl;

    idl::Module root;
    idl::Module& submod_a = root.create_submodule("a");
    idl::Module& submod_b = root.create_submodule("b");
    idl::Module& submod_aa = submod_a.create_submodule("a");
    root.structure(inner);
    submod_aa.structure(outer);

    std::cout << std::boolalpha;
    std::cout << "Does a::a::OuterType exists?: " << root.has_structure("a::a::OuterType") << std::endl;
    std::cout << "Does ::InnerType exists?: " << root.has_structure("::InnerType") << std::endl;
    std::cout << "Does InnerType exists?: " << root.has_structure("InnerType") << std::endl;
    std::cout << "Does OuterType exists?: " << root.has_structure("OuterType") << std::endl;

    DynamicData module_data(root["a"]["a"].structure("OuterType")); // ::a::a::OuterType
    module_data["om3"] = "This is a string.";

    EnumerationType<uint32_t> my_enum("MyEnum");
    my_enum.add_enumerator("A", 0);
    my_enum.add_enumerator("B", 10);
    my_enum.add_enumerator("C");

    DynamicData enum_data(my_enum);
    enum_data = my_enum.value("C");

    uint32_t value = enum_data;
    DynamicData enum_data2 = enum_data;
    uint32_t value2 = enum_data2;

    // uint64_t die = enum_data2; // This will assert

    std::cout << "Enumeration::C: " << value << std::endl;
    std::cout << "Enumeration2::C: " << value2 << std::endl;

    std::cout << "-----------------------" << std::endl;
    submod_b.enum_32(std::move(my_enum));
    root.create_constant("MyConstEnum", enum_data);
    DynamicData my_const(primitive_type<uint64_t>());
    my_const = UINT64_C(555);
    root.create_constant("MyConst", my_const);
    root.add_alias(abool);
    std::cout << idl::generate(root) << std::endl;

    // EnumerationType<uint64_t> my_long_enum("MyLongEnum"); // Static assert, uint64_t isn't allowed
    // enum_data2 = static_cast<uint32_t>(2); // Asserts because 2 isn't a valid value (0, 10 and 11).

    UnionType union_type("MyUnion", my_enum); // New UnionType using an enumeration as discriminator type
    std::vector<std::string> string_label = {"A"};
    union_type.add_case_member(string_label, Member("um1", inner));
    std::vector<uint32_t> enum_label = {my_enum.value("B"), my_enum.value("C")};
    union_type.add_case_member<uint32_t>(enum_label, Member("um2", primitive_type<float>()));
    union_type.add_case_member<uint32_t>({}, Member("um3", abool), true);

    DynamicData union_data(union_type);
    std::cout << "Uninitialized 'union::um3' data: " << union_data.get_member("um3").value<bool>() << std::endl;
    union_data["um2"] = 3.14159265f;
    std::cout << "union::um2 = PI: " << union_data["um2"].value<float>() << std::endl;
    union_data["um1"] = data["om2"];
    std::cout << "union::um1::im2: " << union_data["um1"]["im2"].value<float>() << std::endl;

    // Operators example
    DynamicData op_data(primitive_type<uint32_t>());
    op_data.value(5u);
    std::cout << "-----------------------" << std::endl;
    std::cout << "'op_data': " << op_data.value<uint32_t>();
    op_data++;
    std::cout << ", 'op_data++': " << op_data.value<uint32_t>() << std::endl;

    // Inheritance example
    std::cout << "-----------------------" << std::endl;
    StructType parent("ParentStruct");
    parent.add_member(Member("parent_uint32", primitive_type<uint32_t>()));
    parent.add_member(Member("parent_string", StringType()));
    StructType child("ChildStruct", &parent);
    child.add_member(Member("child_string", StringType()));
    StructType grand_child("GrandChildStruct", &child);
    grand_child.add_member(Member("grand_child_float", primitive_type<float>()));
    grand_child.add_member(Member("grand_child_double", primitive_type<double>()));
    std::cout << idl::generate(parent) << std::endl;
    std::cout << idl::generate(child) << std::endl;
    std::cout << idl::generate(grand_child) << std::endl;
    DynamicData grand_child_data(grand_child);
    grand_child_data["parent_string"] = "I'm the grand child!";
    grand_child_data["child_string"] = "I'm the grand child again!";
    grand_child_data["grand_child_float"] = 55.5f;
    std::cout << "grand_child_data[\"parent_string\"] = " << grand_child_data["parent_string"].value<std::string>()
              << std::endl;
    std::cout << "grand_child_data[\"child_string\"] = " << grand_child_data["child_string"].value<std::string>()
              << std::endl;
    std::cout << "grand_child_data[\"grand_child_float\"] = " << grand_child_data["grand_child_float"].value<float>()
              << std::endl;

    return 0;
}
