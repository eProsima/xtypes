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

#ifndef EPROSIMA_XTYPES_IDL_PARSER_HPP_
#define EPROSIMA_XTYPES_IDL_PARSER_HPP_

#include <peglib.h>

#include <xtypes/ArrayType.hpp>
#include <xtypes/StringType.hpp>
#include <xtypes/StructType.hpp>
#include <xtypes/SequenceType.hpp>

#include <xtypes/idl/grammar.hpp>

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
        : parser_(idl_grammar())
    {
        parser_.enable_ast();
    }

    bool parse(
            const char* idl_string,
            bool ignore_case = false)
    {
        std::shared_ptr<peg::Ast> ast;
        ignore_case_ = ignore_case;
        if (!parser_.parse(idl_string, ast))
        {
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast)->fill_all_types(types_map_);
        return true;
    }

    bool parse_file(
            const char* idl_file,
            bool ignore_case = false)
    {
        std::vector<char> source;
        std::shared_ptr<peg::Ast> ast;
        ignore_case_ = ignore_case;
        if (!(read_file(idl_file, source) && parser_.parse_n(source.data(), source.size(), ast, idl_file)))
        {
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast)->fill_all_types(types_map_);
        return true;
    }

    void get_all_types(
            std::map<std::string, DynamicType::Ptr>& types_map)
    {
        types_map = types_map_;
    }

    class exception// : public std::exception
    {
    private:
        std::string message_;
        std::shared_ptr<peg::Ast> ast_;
    public:
        exception(
                const std::string& message,
                const std::shared_ptr<peg::Ast> ast)
            : message_(message)
            , ast_(ast)
        {}

        const std::string what() const noexcept
        {
            std::string output;
            output = "Parser exception (" + ast_->path + ":" + std::to_string(ast_->line)
                     + ":" + std::to_string(ast_->column) + "): " + message_;
            return output;
        }
    };

private:
    peg::parser parser_;
    std::map<std::string, DynamicType::Ptr> types_map_;
    bool ignore_case_ = false;

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

    class SymbolScope
    {
    public:
        SymbolScope(
                std::shared_ptr<SymbolScope> outer)
            : outer(outer)
        {}

        std::string scope()
        {
            if (outer != nullptr && !outer->scope().empty())
            {
                return outer->scope() + "::" + name;
            }
            return name;
        }

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

        DynamicType::Ptr get_type(
                const std::string& name) const
        {
            // Solve scope
            if (name.find("::") != std::string::npos) // It is an scoped name
            {
                if (name.find("::") == 0) // Looking for root
                {
                    if (outer == nullptr) // We are the root, now go down.
                    {
                        return get_type(name.substr(2));
                    }
                    else // We are not the root, go up.
                    {
                        return outer->get_type(name);
                    }
                }
                else // not looking for root
                {
                    std::string inner_scope = name.substr(0, name.find("::"));
                    if (inner.count(inner_scope) > 0) // We have a inner scope that matches.
                    {
                        std::string inner_name = name.substr(name.find("::") + 2);
                        const auto& it = inner.find(inner_scope);
                        return it->second->get_type(inner_name);
                    }
                }
            }
            // No scope, or scope resolution failed: Try in upwards
            auto it = structs.find(name);
            if (it != structs.end())
            {
                return it->second;
            }
            if (nullptr != outer)
            {
                return outer->get_type(name);
            }
            return DynamicType::Ptr();
        }

        std::map<std::string, DynamicType::Ptr> get_all_types() const
        {
            std::map<std::string, DynamicType::Ptr> result;
            fill_all_types(result);
            return result;
        }

        void fill_all_types(
                std::map<std::string, DynamicType::Ptr>& map) const
        {
            map.insert(structs.begin(), structs.end());
            for (const auto& pair : inner)
            {
                pair.second->fill_all_types(map);
            }
        }

        //std::map<std::string, std::shared_ptr<Constants>> constants;
        //std::map<std::string, std::shared_ptr<Module>> modules;
        //std::map<std::string, DynamicType::Ptr> types;
        //std::map<std::string, std::shared_ptr<StructType>> structs;
        std::map<std::string, DynamicType::Ptr> structs;
        //std::map<std::string, std::shared_ptr<AnnotationType>> annotations;
        std::shared_ptr<SymbolScope> outer;
        std::map<std::string, std::shared_ptr<SymbolScope>> inner;
        std::string name;
    };

    std::shared_ptr<SymbolScope> build_on_ast(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> scope = nullptr)
    {
        using namespace peg::udl;
        if (scope == nullptr)
        {
            types_map_.clear();
            scope = std::make_shared<SymbolScope>(nullptr);
        }
        switch (ast->tag){
            case "MODULE_DCL"_:
                module_dcl(ast, scope);
                break;
            case "CONST_DCL"_:
                const_dcl(ast, scope);
                break;
            case "STRUCT_DEF"_:
                struct_def(ast, scope);
                break;
            case "STRUCT_FORWARD_DCL"_:
                struct_fw_dcl(ast, scope);
                break;
            case "UNION_DEF"_:
                union_def(ast, scope);
                break;
            case "UNION_FORWARD_DCL"_:
                union_fw_dcl(ast, scope);
                break;
            case "ENUM_DCL"_:
                enum_dcl(ast, scope);
                break;
            case "ANNOTATION_DCL"_:
            //    annotation_dcl(ast, scope);
                break;
            case "BITSET_DCL"_:
                // TODO bitset_dcl(ast, scope);
                std::cout << "Bitset unsupported" << std::endl;
                break;
            case "BITMASK_DCL"_:
                // TODO bitmask_dcl(ast, scope);
                std::cout << "Bitmask unsupported" << std::endl;
                break;
            case "TYPE_DECLARATOR"_:
                alias_dcl(ast, scope);
                break;
            default:
                for (auto node : ast->nodes)
                {
                    build_on_ast(node, scope);
                }
                break;
        }
        return scope;
    }

    std::string resolve_identifier(
            const std::shared_ptr<peg::Ast> ast,
            const std::string& identifier,
            std::shared_ptr<SymbolScope> scope,
            bool ignore_already_used = false)
    {
        if (identifier.find("_") == 0)
        {
            return identifier.substr(1); // If the identifier starts with "_", remove the underscode and return.
        }

        if (is_token(identifier))
        {
            throw exception("The identifier \"" + identifier + "\" is a reserved word.", ast);
        }

        if (!ignore_already_used && scope->has_symbol(identifier))
        {
            throw exception("The identifier \"" + identifier + "\" is already used.", ast);
        }

        return identifier;
    }

    void to_lower(
            std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c)
                       {
                           return std::tolower(c);
                       });
    }

    bool is_token(
        const std::string& identifier)
    {
        std::string aux_id = identifier;

        if (!ignore_case_)
        {
            to_lower(aux_id);
        }

        for (const std::string& name : parser_.get_rule_names())
        {
            if (name.find("KW_") == 0) // If it starts with "KW_", is a reserved word. You are welcome.
            {
                if (parser_[name.c_str()].parse(aux_id.c_str()).ret)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void module_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::shared_ptr<SymbolScope> scope;
        for (auto& node : ast->nodes)
        {
            switch (node->tag){
                case "IDENTIFIER"_:
                {
                    std::string name = resolve_identifier(node, node->token, outer, true);
                    if (outer->inner.count(name) == 0)
                    {
                        // New scope
                        scope = std::make_shared<SymbolScope>(outer);
                        scope->name = name;
                        outer->inner.emplace(name, scope);
                    }
                    else
                    {
                        // Adding to an already defined scope
                        scope = outer->inner[name];
                    }
                    break;
                }
                default:
                    build_on_ast(node, scope);
                    break;
            }
        }
    }

    void alias_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        std::string name;
        std::vector<size_t> dimensions;

        if (ast->nodes[1]->tag == "IDENTIFIER"_)
        {
            name = resolve_identifier(ast, ast->nodes[1]->token, outer);
            //name = ast->nodes[1]->token;
        }
        else if (ast->nodes[1]->tag == "ARRAY_DECLARATOR"_)
        {
            auto& node = ast->nodes[1];
            name = resolve_identifier(ast, ast->nodes[0]->token, outer);
            //name = node->nodes[0]->token;
            for (size_t idx = 1; idx < node->nodes.size(); ++idx)
            {
                dimensions.push_back(std::atoi(node->nodes[idx]->token.c_str()));
            }
            type = get_array_type(dimensions, type);
        }

        std::cout << "Found typedef " << name << " for type " << type->name()
                  << " but typedefs aren't supported. Ignoring." << std::endl;
    }

    void const_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        std::string identifier = ast->nodes[1]->token;
        //std::string expr = solve_expr(ast->nodes[2]->token);

        std::cout << "Found \"const " << type->name() << " " << identifier /*<< " = " << expr*/ << "\" "
                  << "but const aren't supported. Ignoring." << std::endl;
    }

    void enum_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;

        std::string name = ast->nodes[0]->token;
        // std::vector<std::string> value[i] = ast->nodes[i]->token;

        std::cout << "Found \"enum " << name << "\" but enumerations aren't supported. Ignoring." << std::endl;
    }

    void struct_fw_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        DynamicType::Ptr exists = outer->get_type(resolve_identifier(ast, ast->token, outer));
        if (exists.get() != nullptr)
        {
            throw exception("Struct " + ast->token + " was already declared.", ast);
        }

        StructType result(ast->token);
        outer->structs.emplace(ast->token, std::move(result));
    }

    void union_fw_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        DynamicType::Ptr exists = outer->get_type(resolve_identifier(ast, ast->token, outer));
        if (exists.get() != nullptr)
        {
            throw exception("Union " + ast->token + " was already declared.", ast);
        }

        // TODO Replace by Unions. Kept as Struct to allow name solving.
        StructType result(ast->token);
        outer->structs.emplace(ast->token, std::move(result));
    }

    void struct_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::string name;
        std::map<std::string, Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    //name = node->token;
                    StructType result(name);
                    outer->structs.emplace(name, std::move(result));
                    break;
                }
                case "INHERITANCE"_:
                    // parent = outer.structs[node->token]; // TODO Check if it doesn't exists
                    break;
                case "MEMBER"_:
                    member_def(node, outer, member_list);
                    break;
            }
        }

        DynamicType::Ptr result = outer->get_type(name);
        StructType* struct_type = static_cast<StructType*>(const_cast<DynamicType*>(result.get()));
        if (!struct_type->members().empty())
        {
            throw exception("Struct " + name + " redefinition.", ast);
        }
        for (auto& member : member_list)
        {
            struct_type->add_member(std::move(member.second));
        }
        // Replace
        outer->structs[name] = DynamicType::Ptr(*struct_type);
    }

    void union_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::map<std::string, Member> member_list;
        DynamicType::Ptr type;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    //name = node->token;
                    break;
                }
                case "SWITCH_TYPE_SPEC"_:
                case "SCOPED_NAME"_:
                {
                    type = type_spec(node, outer);
                    //UnionType result(name, type);
                    //outer->unions.emplace(name, std::move(result));
                    break;
                }
                case "SWITCH_BODY"_:
                    //switch_body(node, outer, member_list, type);
                    // TODO:
                    //     + SWITCH_BODY
                    //          + CASE
                    //              - CASE_LABEL/0 (0)
                    //              + ELEMENT_SPEC
                    //                  + TYPE_SPEC/0[SIGNED_LONG_INT]
                    //                  - DECLARATOR/1[IDENTIFIER] (my_int32)
                    //          + CASE
                    //              - CASE_LABEL/0 (1)
                    //              + ELEMENT_SPEC
                    //                  + TYPE_SPEC/0[UNSIGNED_LONGLONG_INT]
                    //                  - DECLARATOR/1[IDENTIFIER] (my_uint64)
                    //          + CASE
                    //              + CASE_LABEL/1 (default)
                    //              + ELEMENT_SPEC
                    //                  + TYPE_SPEC/0[STRING_TYPE]
                    //                  - DECLARATOR/1[IDENTIFIER] (my_string)
                    break;
            }
        }

        std::cout << "Found \"union " << name << "\" with discriminator of type " << type->name()
                  << " but unions aren't supported. Ignoring." << std::endl;
        /* TODO
        DynamicType::Ptr result = outer->get_type(name);
        UnionType* union_type = static_cast<UnionType*>(const_cast<DynamicType*>(result.get()));
        if (!union_type->members().empty())
        {
            throw exception("Union " + name + " redefinition.", ast);
        }
        for (auto& member : member_list)
        {
            union_type->add_member(std::move(member.second));
        }
        // Replace
        outer->unions[name] = DynamicType::Ptr(*struct_type);
        */
    }

    void member_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer,
            std::map<std::string, Member>& result)
    {
        using namespace peg::udl;
        DynamicType::Ptr type;

        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "TYPE_SPEC"_:
                    type = type_spec(node, outer);
                    break;
                case "DECLARATORS"_:
                    if (type.get() != nullptr)
                    {
                        members(node, outer, type, result);
                    }
                    break;
            }
        }
    }

    DynamicType::Ptr type_spec(
            const std::shared_ptr<peg::Ast> node, //ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        //for (const auto& node : ast->nodes)
        //{
            switch (node->tag)
            {
                case "SCOPED_NAME"_: // Scoped name
                case "IDENTIFIER"_:
                {
                    DynamicType::Ptr type = outer->get_type(node->token);
                    if (type.get() == nullptr)
                    {
                        throw exception("Member type " + node->token + " is unknown", node);
                    }
                    return type;
                }
                case "BOOLEAN_TYPE"_:
                    return primitive_type<bool>();
                case "SIGNED_TINY_INT"_:
                    return primitive_type<char>();
                case "UNSIGNED_TINY_INT"_:
                case "OCTET_TYPE"_:
                    return primitive_type<uint8_t>();
                case "SIGNED_SHORT_INT"_:
                    return primitive_type<int16_t>();
                case "UNSIGNED_SHORT_INT"_:
                    return primitive_type<uint16_t>();
                case "SIGNED_LONG_INT"_:
                    return primitive_type<int32_t>();
                case "UNSIGNED_LONG_INT"_:
                    return primitive_type<uint32_t>();
                case "SIGNED_LONGLONG_INT"_:
                    return primitive_type<int64_t>();
                case "UNSIGNED_LONGLONG_INT"_:
                    return primitive_type<uint64_t>();
                case "FLOAT_TYPE"_:
                    return primitive_type<float>();
                case "DOUBLE_TYPE"_:
                    return primitive_type<double>();
                case "LONG_DOUBLE_TYPE"_:
                    return primitive_type<long double>();
                case "CHAR_TYPE"_:
                    return primitive_type<char>();
                case "WIDE_CHAR_TYPE"_:
                    return primitive_type<wchar_t>();
                case "STRING_TYPE"_:
                    return StringType();
                case "STRING_SIZE"_:
                    return StringType(std::atoi(node->token.c_str()));
                case "WIDE_STRING_TYPE"_:
                    return WStringType();
                case "WSTRING_SIZE"_:
                    return WStringType(std::atoi(node->token.c_str()));
                case "SEQUENCE_TYPE"_:
                {
                    DynamicType::Ptr inner_type = type_spec(node->nodes[0], outer);
                    size_t size = 0;
                    if (node->nodes.size() > 1)
                    {
                        size = std::atoi(node->nodes[1]->token.c_str());
                    }
                    return SequenceType(*inner_type, size);
                }
                case "MAP_TYPE"_:
                {
                    DynamicType::Ptr key_type = type_spec(node->nodes[0], outer);
                    DynamicType::Ptr inner_type = type_spec(node->nodes[1], outer);
                    size_t size = 0;
                    if (node->nodes.size() > 2)
                    {
                        size = std::atoi(node->nodes[2]->token.c_str());
                    }
                    std::cout << "Found \"map<" << key_type->name() << ", " << inner_type->name()
                              << ", " << size << ">\" "
                              << "but maps aren't supported. Ignoring." << std::endl;
                    break;
                    // return MapType(*key_type, *inner_type, size); // TODO, uncomment when maps are implemented.
                }
                default:
                    return type_spec(node, outer);
            }
        //}

        return DynamicType::Ptr();
    }

    ArrayType::Ptr get_array_type(
            const std::vector<size_t>& dimensions,
            const DynamicType::Ptr type)
    {
        size_t base_dim = dimensions.back();
        ArrayType array(*type, base_dim);
        ArrayType::Ptr next_array(array);

        if (dimensions.size() > 1)
        {
            for (int32_t idx = dimensions.size() - 2; idx >= 0; --idx)
            {
                size_t dim = dimensions.at(idx);
                ArrayType::Ptr next(ArrayType(*next_array, dim));
                next_array = next;
            }
        }

        return next_array;
    }

    void member_array(
            const std::string& name,
            const std::vector<size_t>& dimensions,
            const DynamicType::Ptr type,
            std::map<std::string, Member>& result)
    {
        result.emplace(name, Member(name, *get_array_type(dimensions, type)));
    }

    void members(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer,
            const DynamicType::Ptr type,
            std::map<std::string, Member>& result)
    {
        using namespace peg::udl;
        using id_pair = std::pair<std::string, std::vector<size_t>>;
        using id_pair_vector = std::vector<id_pair>;

        id_pair_vector pairs = identifier_list(ast, outer);
        for (id_pair& pair : pairs)
        {
            std::string name = pair.first;
            std::vector<size_t> dimensions = pair.second;
            if (result.count(name) > 0)
            {
                throw exception("Member identifier " + ast->token + " already defined", ast);
            }
            if (dimensions.empty())
            {
                result.emplace(name, Member(name, *type));
            }
            else
            {
                member_array(name, dimensions, type, result);
            }
        }
    }

    std::vector<std::pair<std::string, std::vector<size_t>>> identifier_list(
            const std::shared_ptr<peg::Ast> node,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::vector<std::pair<std::string, std::vector<size_t>>> result;

        if (node->tag == node->original_tag)
        {
            // Multiple declaration
            for (auto& subnode : node->nodes)
            {
                identifier(subnode, outer, result);
            }
        }
        else
        {
            // Only one declaration
            identifier(node, outer, result);
        }
        return result;
    }

    void identifier(
            const std::shared_ptr<peg::Ast> node,
            std::shared_ptr<SymbolScope> outer,
            std::vector<std::pair<std::string, std::vector<size_t>>>& list)
    {
        using namespace peg::udl;
        std::string name;
        std::vector<size_t> dimensions;
        switch (node->tag)
        {
            case "IDENTIFIER"_:
                name = resolve_identifier(node, node->token, outer);
                break;
            case "ARRAY_DECLARATOR"_:
            {
                for (auto& subnode : node->nodes)
                {
                    switch (subnode->tag)
                    {
                        case "IDENTIFIER"_:
                        {
                            name = resolve_identifier(subnode, subnode->token, outer);
                            break;
                        }
                        case "POSITIVE_INT_CONST"_:
                        {
                            dimensions.push_back(std::atoi(subnode->token.c_str()));
                            break;
                        }
                    }
                }
                break;
            }
        }
        list.emplace_back(name, std::move(dimensions));
    }
};

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDL_PARSER_HPP_
