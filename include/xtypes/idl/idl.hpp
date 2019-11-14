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

inline std::map<std::string, DynamicType::Ptr> parse(
        const std::string& idl)
{
    std::map<std::string, DynamicType::Ptr> result;
    static Parser parser;
    if (parser.parse(idl.c_str()))
    {
        parser.get_all_types(result);
    }
    return result;
}

inline std::map<std::string, DynamicType::Ptr> parse_file(
        const std::string& idl_file)
{
    std::map<std::string, DynamicType::Ptr> result;
    static Parser parser;
    if (parser.parse_file(idl_file.c_str()))
    {
        parser.get_all_types(result);
    }
    return result;
}


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
