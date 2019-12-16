#ifndef EPROSIMA_TESTS_UTILS_HPP_
#define EPROSIMA_TESTS_UTILS_HPP_

#if defined(XTYPES_EXCEPTIONS)
#define ASSERT_OR_EXCEPTION(exp, msg)                                                                       \
{                                                                                                           \
            try                                                                                             \
            {                                                                                               \
                { exp }                                                                                     \
                FAIL() << "Exception wasn't throw!";                                                        \
            }                                                                                               \
            catch(const std::runtime_error& exc)                                                            \
            {                                                                                               \
                if (std::string(exc.what()).find(msg) == std::string::npos)                                 \
                {                                                                                           \
                    FAIL() << "Unexpected exception: " << exc.what();                                       \
                }                                                                                           \
            }                                                                                               \
}
#else
#if !defined(NDEBUG)
#define ASSERT_OR_EXCEPTION(exp, msg)                                                                       \
{                                                                                                           \
        ASSERT_DEATH(                                                                                       \
            {                                                                                               \
                exp                                                                                         \
            },                                                                                              \
            msg                                                                                             \
        );                                                                                                  \
}
#else
#define ASSERT_OR_EXCEPTION(exp, msg)
#endif
#endif

#endif // EPROSIMA_TESTS_UTILS_HPP_
