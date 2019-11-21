#include <xtypes/xtypes.hpp>

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

    std::map<std::string, DynamicType::Ptr> from_idl = idl::parse(idl_spec).structs;
    const StructType& inner = static_cast<const StructType&>(*from_idl.at("InnerType"));

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

    std::cout << data.to_string() << std::endl; //See to_string() implementation as an example of data instrospection

    return 0;
}
