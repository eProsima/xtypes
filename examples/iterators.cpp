#include <xtypes/xtypes.hpp>

#include <iostream>

using namespace eprosima::xtypes;

int main()
{
    std::cout << "--- Iterate string ---" << std::endl;
    {
        StringType string;
        DynamicData str1(string);

        str1.value<std::string>("Hello!");
        for (auto&& elem : str1) // Loop through each `ReadableDynamicDataRef`.
        {
            std::cout << elem.value<char>();
        }
        std::cout << std::endl;

        // Hello! -> Ifmmp"
        for (auto&& elem : str1) // Loop through each `WritableDynamicDataRef`.
        {
            elem = static_cast<char>(elem.value<char>() + 1);
        }

        for (auto&& elem : str1) // Loop through each `ReadableDynamicDataRef`.
        {
            std::cout << elem.value<char>();
        }
        std::cout << std::endl;
    }

    std::cout << "--- Iterate array ---" << std::endl;
    {
        ArrayType array_type(primitive_type<int32_t>(), 10);
        DynamicData array(array_type);

        for (int i = 0; i < 10; ++i)
        {
            array[i] = 5 * i;
        }

        int32_t check_sum = 0;
        for (auto&& elem : array) // Loop through each `ReadableDynamicDataRef`.
        {
            check_sum += elem.value<int32_t>();
        }

        for (auto&& elem : array) // Loop through each `WritableDynamicDataRef`.
        {
            elem = elem.value<int32_t>() * 2;
        }

        int32_t double_check_sum = 0;
        for (auto&& elem : array) // Loop through each `ReadableDynamicDataRef`.
        {
            double_check_sum += elem.value<int32_t>();
        }

        std::cout << "Array Check_sum is: " << check_sum << " and its double is: " << double_check_sum << std::endl;
    }

    std::cout << "--- Iterate sequence ---" << std::endl;
    {
        SequenceType seq_type(primitive_type<int32_t>());
        DynamicData seq(seq_type);

        for (int i = 0; i < 10; ++i)
        {
            seq.push(5 * i);
        }

        int32_t check_sum = 0;
        for (auto&& elem : seq) // Loop through each `ReadableDynamicDataRef`.
        {
            check_sum += elem.value<int32_t>();
        }

        for (auto&& elem : seq) // Loop through each `WritableDynamicDataRef`.
        {
            elem = elem.value<int32_t>() * 2;
        }

        int32_t double_check_sum = 0;
        for (auto&& elem : seq) // Loop through each `ReadableDynamicDataRef`.
        {
            double_check_sum += elem.value<int32_t>();
        }
        std::cout << "Sequence Check_sum is: " << check_sum << " and its double is: " << double_check_sum << std::endl;
    }

    std::cout << "--- Iterate map ---" << std::endl;
    {
        MapType map_type(primitive_type<uint32_t>(), primitive_type<int32_t>());
        DynamicData map(map_type);

        DynamicData key(primitive_type<uint32_t>());
        for (uint32_t i = 0; i < 10; ++i)
        {
            key = i;
            map[key] = int32_t(5 * i);
        }


        int32_t check_sum = 0;
        // The map returns an iterator to its pairs, which doesn't follow the insertion order!
        for (auto&& elem : map) // Loop through each `ReadableDynamicDataRef`.
        {
            check_sum += elem[1].value<int32_t>();
        }

        for (auto&& elem : map) // Loop through each `WritableDynamicDataRef`.
        {
            elem[1] = elem[1].value<int32_t>() * 2;
        }

        int32_t double_check_sum = 0;
        for (auto&& elem : map) // Loop through each `ReadableDynamicDataRef`.
        {
            double_check_sum += elem[1].value<int32_t>();
        }
        std::cout << "Map Check_sum is: " << check_sum << " and its double is: " << double_check_sum << std::endl;
    }

    std::cout << "--- Iterate struct data (wide) ---" << std::endl;
    {
        StructType my_struct("MyStruct");
        my_struct.add_member("my_int", primitive_type<int32_t>());
        my_struct.add_member("my_double", primitive_type<double>());

        DynamicData my_data(my_struct);
        my_data["my_int"] = 55;
        my_data["my_double"] = -23.44;

        //ReadableDynamicDataRef::MemberIterator it = my_data.citems().begin();
        [[maybe_unused]] auto it = my_data.citems().begin();
        [[maybe_unused]] auto wit = my_data.items().begin();

        for (auto&& elem : my_data.items()) // Loop through each `ReadableDynamicDataRef`.
        {
            switch (elem.kind())
            {
                case TypeKind::INT_32_TYPE:
                    std::cout << elem.member().name() << ": " << elem.data().value<int32_t>() << std::endl;
                    break;
                case TypeKind::FLOAT_64_TYPE:
                    std::cout << elem.member().name() << ": " << elem.data().value<double>() << std::endl;
                    break;
                default:
                    break;
            }
        }

        for (auto&& elem : my_data.items()) // Loop through each `WritableDynamicDataRef`.
        {
            switch (elem.kind())
            {
                case TypeKind::INT_32_TYPE:
                    elem.data() = elem.data().value<int32_t>() * 2;
                    break;
                case TypeKind::FLOAT_64_TYPE:
                    elem.data() = elem.data().value<double>() * 2;
                    break;
                default:
                    break;
            }
        }

        for (auto&& elem : my_data.items()) // Loop through each `ReadableDynamicDataRef`.
        {
            switch (elem.kind())
            {
                case TypeKind::INT_32_TYPE:
                    std::cout << elem.member().name() << " * 2: " << elem.data().value<int32_t>() << std::endl;
                    break;
                case TypeKind::FLOAT_64_TYPE:
                    std::cout << elem.member().name() << " * 2: " << elem.data().value<double>() << std::endl;
                    break;
                default:
                    break;
            }
        }
    }

    std::cout << "--- Iterate struct type (depth) ---" << std::endl;
    {
        StructType l2 = StructType("Level2")
                .add_member("l2m1", primitive_type<uint32_t>())
                .add_member("l2m2", primitive_type<float>())
                .add_member("l2m3", StringType())
                .add_member("l2m4", WStringType());

        StructType l1 = StructType("Level1")
                .add_member("l1m1", SequenceType(primitive_type<uint32_t>()))
                .add_member("l1m2", SequenceType(l2))
                .add_member("l1m3", ArrayType(primitive_type<uint32_t>(), 2))
                .add_member("l1m4", ArrayType(l2, 4))
                .add_member("l1m5", l2);

        StructType l0 = StructType("Level0")
                .add_member("l0m1", l1)
                .add_member("l0m2", l2)
                .add_member("l0m3", MapType(primitive_type<uint32_t>(), StringType()));

        l0.for_each([&](const DynamicType::TypeNode& node)
        {
            for (size_t tabs = 0; tabs < node.deep(); ++tabs)
            {
                std::cout << "  ";
            }
            std::cout << node.type().name();
            if (node.has_parent() && node.from_member() != nullptr)
            {
                std::cout << " " << node.from_member()->name();
            }
            std::cout << std::endl;
        });
    }

    std::cout << "--- Iterate struct data (depth) ---" << std::endl;
    {
        StructType l2 = StructType("Level2")
                .add_member("l2m1", primitive_type<uint32_t>())
                .add_member("l2m2", primitive_type<float>())
                .add_member("l2m3", StringType());

        StructType l1 = StructType("Level1")
                .add_member("l1m1", SequenceType(primitive_type<uint32_t>()))
                .add_member("l1m2", SequenceType(l2))
                .add_member("l1m3", ArrayType(primitive_type<uint32_t>(), 2))
                .add_member("l1m4", ArrayType(l2, 4))
                .add_member("l1m5", l2);

        StructType l0 = StructType("Level0")
                .add_member("l0m1", l1)
                .add_member("l0m2", l2)
                .add_member("l0m3", MapType(primitive_type<uint32_t>(), StringType()));

        uint32_t uint_value = 0;
        DynamicData data(l0);
        data["l0m1"]["l1m1"].push(uint_value++);    // [0] = 0
        data["l0m1"]["l1m1"].push(uint_value++);    // [1] = 1
        data["l0m1"]["l1m1"].push(uint_value++);    // [2] = 2
        {
            DynamicData l2data(l2);
            l2data["l2m1"] = uint_value++;          // 3
            l2data["l2m2"] = 12.345f;
            l2data["l2m3"] = "l2m3";
            data["l0m1"]["l1m2"].push(l2data);      // [0]
            data["l0m1"]["l1m2"].push(l2data);      // [1]
        }
        data["l0m1"]["l1m3"][0] = uint_value++;     // 4
        data["l0m1"]["l1m3"][1] = uint_value++;     // 5
        {
            DynamicData l2data(l2);
            l2data["l2m1"] = uint_value++;          // 6
            l2data["l2m2"] = 123.45f;
            l2data["l2m3"] = "l2m3_bis";
            data["l0m1"]["l1m4"][0] = l2data;       // [0]
            data["l0m1"]["l1m4"][1] = l2data;       // [1]
            data["l0m1"]["l1m4"][2] = l2data;       // [2]
            data["l0m1"]["l1m4"][3] = l2data;       // [3]
        }
        {
            DynamicData l2data(l2);
            l2data["l2m1"] = uint_value++;          // 7
            l2data["l2m2"] = 1234.5f;
            l2data["l2m3"] = "l2m3_bis_2";
            data["l0m1"]["l1m5"] = l2data;
        }
        data["l0m2"]["l2m1"] = uint_value++;        // 8
        data["l0m2"]["l2m2"] = 12345.f;
        data["l0m2"]["l2m3"] = "l2m3_bis_3";

        DynamicData key(primitive_type<uint32_t>());
        // Maps alter the internal storage of their elements, so they will be printed in a different order.
        for (uint32_t i = 0; i < 5; ++i)
        {
            key = i;
            data["l0m3"][key] = "Map element: " + std::to_string(i);
        }


        data.for_each([&](const DynamicData::ReadableNode& node)
        {
            if (!node.has_parent())
            {
                return;                     // Ignore the root
            }
            if (node.from_member() != nullptr)
            {
                for (size_t tabs = 0; tabs < node.deep(); ++tabs)
                {
                    std::cout << "  ";
                }
                std::cout << node.from_member()->name();
                if (node.type().is_primitive_type() || node.type().kind() == TypeKind::STRING_TYPE)
                {
                    std::cout << ": " << node.data().cast<std::string>();
                }
                std::cout << std::endl;
            }
        });
    }

    return 0;
}
