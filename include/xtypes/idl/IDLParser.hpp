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

#ifndef EPROSIMA_XTYPES_IDLPARSER_HPP_
#define EPROSIMA_XTYPES_IDLPARSER_HPP_

#include <peglib.h>

#include <xtypes/xtypes.hpp>
//#include <xtypes/DynamicType.hpp>
#include <xtypes/idl/IDLGrammar.hpp>

#include <map>
#include <vector>
#include <fstream>
#include <memory>
#include <exception>

namespace eprosima {
namespace xtypes {
namespace idl {

class Parser
{
public:
    Parser()
        : parser_(IDL_GRAMMAR)
    {
        parser_.enable_ast();
    }

    bool parse(
            const char* idl_string)
    {
        std::shared_ptr<peg::Ast> ast;
        if (!parser_.parse(idl_string, ast))
        {
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast);
        return true;
    }

    bool parse_file(
            const char* idl_file)
    {
        std::vector<char> source;
        std::shared_ptr<peg::Ast> ast;
        if (!(read_file(idl_file, source) && parser_.parse_n(source.data(), source.size(), ast, idl_file)))
        {
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast);
        return true;
    }

    void get_all_types(
            std::map<std::string, std::shared_ptr<DynamicType>>& types_map)
    {
        types_map = types_map_;
    }

private:
    peg::parser parser_;
    std::map<std::string, std::shared_ptr<DynamicType>> types_map_;

    bool read_file(
            const char* path,
            std::vector<char>& buff) const
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);

        if (ifs.fail())
        {
            return false;
        }

        buff.resize(static_cast<unsigned int>(ifs.seekg(0, std::ios::end).tellg()));

        if (!buff.empty())
        {
            ifs.seekg(0, std::ios::beg).read(&buff[0], static_cast<std::streamsize>(buff.size()));
        }
        return true;
    }

    class exception : public std::exception
    {
    private:
        std::string message_;
        std::shared_ptr<peg::Ast> ast;
    public:
        exception(
                const std::string& message,
                const std::shared_ptr<peg::Ast> ast)
            : message_(message)
        {}

        const char* what() const noexcept override
        {
            std::string output;
            output = "Parser exception (" + ast->path + ":" + std::to_string(ast->line)
                     + ":" + std::to_string(ast->column) + "): " + message_;
            return output.c_str();
        }
    };

    class SymbolScope
    {
    public:
        SymbolScope(
                std::shared_ptr<SymbolScope> outer)
            : outer(outer)
        {}

        /*
        std::string scope()
        {

        }
        */

        bool has_symbol(
                const std::string& ident,
                bool extend = true) const
        {
            size_t n_elems = structs.count(ident); // + constants.count(ident) + members.count(ident) + types.count(ident);
            if (n_elems > 0)
            {
                return true;
            }
            if (extend && outer != nullptr)
            {
                return outer->has_symbol(ident, extend);
            }
            return false;
        }

        std::shared_ptr<DynamicType> get_type(
                const std::string& name) const
        {
            auto it = structs.find(name);
            if (it != structs.end())
            {
                return it->second;
            }
            if (nullptr != outer)
            {
                return outer->get_type(name);
            }
            return std::shared_ptr<DynamicType>(nullptr);
        }

        //std::map<std::string, std::shared_ptr<Constants>> constants;
        //std::map<std::string, std::shared_ptr<Module>> modules;
        //std::map<std::string, std::shared_ptr<DynamicType>> types;
        //std::map<std::string, std::shared_ptr<StructType>> structs;
        std::map<std::string, std::shared_ptr<DynamicType>> structs;
        //std::map<std::string, std::shared_ptr<AnnotationType>> annotations;
        std::shared_ptr<SymbolScope> outer;
    };

    void build_on_ast(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> scope = nullptr)
    {
        using namespace peg::udl;
        switch (ast->tag){
            //case "MODULE_DCL"_:
            //    module_dcl(ast, scope);
            //    break;
            //case "CONST_DCL"_:
            //    const_dcl(ast, scope);
            //    break;
            case "STRUCT_DEF"_:
                struct_def(ast, scope);
                break;
            //case "ANNOTATION_DCL"_:
            //    annotation_dcl(ast, scope);
            //    break;
            default:
                for (auto node : ast->nodes)
                {
                    build_on_ast(node, scope);
                }
                break;
        }
        types_map_ = scope->structs;
    }

    void struct_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        auto scope = std::make_shared<SymbolScope>(outer);
        std::string name;
        std::map<std::string, Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "IDENTIFIER"_:
                    name = node->token;
                    break;
                case "INHERITANCE"_:
                    // parent = outer.structs[node->token]; // TODO Check if it doesn't exists
                    break;
                case "MEMBER"_:
                    member_def(node, scope, member_list);
                    break;
            }
        }
        StructType result(name);
        //std::shared_ptr<DynamicType> result = std::make_shared<StructType>(name);
        for (auto& member : member_list)
        {
            result.add_member(std::move(member.second));
        }
        scope->structs.emplace(name, std::make_shared<StructType>(std::move(result)));
    }

    void member_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer,
            std::map<std::string, Member>& result)
    {
        using namespace peg::udl;
        std::shared_ptr<DynamicType> type;

        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "TYPE_SPEC"_:
                    type = type_spec(node, outer);
                    break;
                case "DECLARATORS"_:
                    members(node, outer, type, result);
                    break;
            }
        }
    }

    std::shared_ptr<DynamicType> type_spec(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "IDENTIFIER"_: // Scoped name
                {
                    std::shared_ptr<DynamicType> type = outer->get_type(node->token);
                    if (nullptr == type)
                    {
                        throw exception("Member type " + node->token + " is unknown", node);
                    }
                    return type;
                }
                case "STRING_TYPE"_:
                    return std::make_shared<StringType>();
                case "FLOAT_TYPE"_:
                {
                    return std::make_shared<PrimitiveType<float>>(primitive_type<float>());
                }
                default:
                    return type_spec(node, outer);
            }
        }

        return std::shared_ptr<DynamicType>(nullptr);
    }

    void members(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer,
            const std::shared_ptr<DynamicType> type,
            std::map<std::string, Member>& result)
    {
        using namespace peg::udl;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "DECLARATOR"_:
                {
                    std::string name = identifier(node, outer);
                    if (result.count(name) > 0)
                    {
                        throw exception("Member identifier " + node->token + " already defined", node);
                    }
                    result.emplace(name, Member(name, *type));
                    break;
                }
            }
        }
    }

    std::string identifier(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "IDENTIFIER"_:
                    if (outer->has_symbol(node->token))
                    {
                        throw exception("Identifier " + node->token + " already defined", node);
                    }
                    return node->token;
                default:
                    return identifier(node, outer);
            }
        }

        return "";
    }

};

static std::map<std::string, std::shared_ptr<DynamicType>> parse(
        const std::string& idl)
{
    std::map<std::string, std::shared_ptr<DynamicType>> result;
    static Parser parser;
    if (parser.parse(idl.c_str()))
    {
        parser.get_all_types(result);
    }
    return result;
}

static std::map<std::string, std::shared_ptr<DynamicType>> parse_file(
        const std::string& idl_file)
{
    std::map<std::string, std::shared_ptr<DynamicType>> result;
    static Parser parser;
    if (parser.parse_file(idl_file.c_str()))
    {
        parser.get_all_types(result);
    }
    return result;
}

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDLPARSER_HPP_
