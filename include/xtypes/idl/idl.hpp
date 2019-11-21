/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef EPROSIMA_XTYPES_IDL_IDL_HPP_
#define EPROSIMA_XTYPES_IDL_IDL_HPP_

#include <xtypes/idl/parser.hpp>
#include <xtypes/idl/generator.hpp>

#include <sstream>

namespace eprosima {
namespace xtypes {
namespace idl {

/// \brief Generates the DynamicTypes related to an idl specification.
/// It supports IDL4.2
/// \param[in] idl A IDL specification to parse into DynamicType.
/// \return A map with the DynamicTypes parsed from the idl.
inline Context& parse(
        const std::string& idl,
        Context& context)
{
    Parser* parser = Parser::instance();
    parser->parse(idl.c_str(), context);
    return context;
}

inline Context parse(
        const std::string& idl)
{
    Parser* parser = Parser::instance();
    return parser->parse(idl);
}

/// \brief Same as parse() but it receives a path file where the IDL is located.
inline Context& parse_file(
        const std::string& idl_file,
        Context& context)
{
    Parser* parser = Parser::instance();
    parser->parse_file(idl_file.c_str(), context);
    return context;
}

inline Context parse_file(
        const std::string& idl_file)
{
    Parser* parser = Parser::instance();
    return parser->parse_file(idl_file);
}

/// \brief Generates the IDL that represents an StructType
/// \param[in] type Type to represent into IDL
/// \return An IDL that represents the StructType given.
inline std::string generate(const StructType& type)
{
    std::stringstream ss;
    ss << "struct " << type.name() << std::endl;
    ss << "{" << std::endl;

    for(const Member& member: type.members())
    {
        ss << std::string(4, ' ');
        if(member.type().kind() == TypeKind::ARRAY_TYPE)
        {
            ss << generator::array_member(member); //Spetial member syntax
        }
        else
        {
            ss << generator::type_name(member.type()) << " " << member.name() << ";";
        }
        ss << std::endl;
    }
    ss << "}" << std::endl;
    return ss.str();
}

} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_IDL_HPP_
