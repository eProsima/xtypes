/**
 * This example shows how to make use of the builtin assert and exception system.
 */

#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>
#include <iostream>

// Auxiliar define to manage both assert (debug only) and exceptions (both)
#if defined(XTYPES_EXCEPTIONS)

// If compiled with PREPROCESSOR flag XTYPES_EXCEPTIONS use this snniped.
#define ASSERT_OR_EXCEPTION(exp, msg)                                                                       \
    {                                                                                                           \
        try                                                                                                     \
        {                                                                                                       \
            exp;                                                                                                \
            std::cout << "Exception wasn't throw!" << std::endl;                                                \
        }                                                                                                       \
        catch (const std::runtime_error& exc)                                                                    \
        {                                                                                                       \
            if (std::string(exc.what()).find(msg) == std::string::npos)                                         \
            {                                                                                                   \
                std::cout << "Unexpected exception: " << exc.what() << std::endl;                               \
                std::cout << "Expected exception should contain: " << msg << std::endl;                         \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                std::cout << "Catched expected exception: " << exc.what() << std::endl;                         \
            }                                                                                                   \
        }                                                                                                       \
    }

#else
#if !defined(NDEBUG)

// If compiled without PREPROCESSOR flag XTYPES_EXCEPTIONS but debug (without NDEBUG defined) an assert will be raised.
#define ASSERT_OR_EXCEPTION(exp, msg)                                                                       \
    {                                                                                                           \
        std::cout << "Expected to raise assert: " << msg << std::endl;                                          \
        {                                                                                                       \
            exp;                                                                                                \
        }                                                                                                       \
        std::cout << "Failed to raise assert: " << msg << std::endl;                                            \
    }

#else

// If compiled without PREPROCESSOR flag XTYPES_EXCEPTIONS and release (with NDEBUG defined) the behavior is undefined.
// In most cases, it will not fail, but maybe there exists invalid accesses or lost of data.
#define ASSERT_OR_EXCEPTION(exp, msg) { std::cout << "Unckeched code: " << #exp << " with msg: " << msg << std::endl; }

#endif
#endif

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
    const StructType& inner = context.module().structure("InnerType");

    DynamicData data(inner);

    // The following sentences, will cause an exception if "XTYPES_EXCEPTION" is defined. Else an assertion if
    // compiled with debug settings and unchecked code message if compiled with release.
    ASSERT_OR_EXCEPTION(data["im3"], "im3"); // Accesing a missing member by name
    ASSERT_OR_EXCEPTION(data[2], "out of bounds"); // Accesing a missing member by index
    ASSERT_OR_EXCEPTION(data["im1"] = 5.5f, "uint32"); // Incompatible data types

    return 0;
}
