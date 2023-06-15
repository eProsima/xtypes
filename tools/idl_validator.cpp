#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

#include <cstdlib>

using namespace eprosima::xtypes;

int main(
        int argc,
        char** argv)
{
    if (argc != 2)
    {
        std::cout << "The IDL must be passed as argument" << std::endl;
        return -1;
    }
    else
    {
        std::string idl_spec = argv[1];
        std::cout << "IDL: " << idl_spec << std::endl;
        idl::Context context;
        context.log_level(idl::log::LogLevel::xDEBUG);
        context.print_log(true);

        // Introduce current ros2 paths
        const char * distro = std::getenv("ROS_DISTRO");
        if (nullptr == distro)
        {
            std::cout << "There is no ROS2 overlay loaded or ros_environment package is missing" << std::endl;
        }
        else
        {
            context.include_paths.push_back("/opt/ros/" + std::string(distro) + "/share/");
        }

        context.ignore_redefinition = true;
        context.allow_keyword_identifiers = true;
        context = idl::parse(idl_spec, context);
        std::cout << "Parse Success: " << std::boolalpha << context.success << std::endl;

        for (auto [name, type] : context.get_all_scoped_types())
        {
            if (type->kind() == TypeKind::STRUCTURE_TYPE)
            {
                std::cout << "Struct Name:" << name << std::endl;
                auto members = static_cast<const StructType*>(type.get())->members();
                for (auto &m: members)
                {
                    if (m.type().kind() == TypeKind::ALIAS_TYPE)
                    {
                        auto alias = static_cast<const AliasType&>(m.type());
                        std::cout << "Struct Member:" << name << "[" << m.name() << "," << alias.rget().name() << "]" << std::endl;
                    }
                    else
                    {
                        std::cout << "Struct Member:" << name << "[" << m.name() << "," << m.type().name() << "]" << std::endl;
                    }
                }
            }
        }
        return context.success ? 0 : 1;
    }
}
