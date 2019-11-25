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
#include <xtypes/DynamicData.hpp>

#include <xtypes/idl/grammar.hpp>

#include <map>
#include <vector>
#include <fstream>
#include <memory>
#include <exception>
#include <locale>
#include <regex>

namespace eprosima {
namespace xtypes {
namespace idl {

class Parser;

struct Context
{
    // Config
    bool ignore_case = false;
    bool clear = true;
    bool preprocess = true;
    bool allow_keyword_identifiers = false;
    std::string preprocessor_exec = "";
    std::vector<std::string> include_paths;

    // Results
    bool success = false;
    std::map<std::string, DynamicType::Ptr> structs;
    std::map<std::string, DynamicData> constants;
    //std::map<std::string, Modules> modules;

    ~Context()
    {
        clear_context();
    }

private:
    friend class Parser;
    Parser* instance_;

    inline void clear_context();
};

static const Context DEFAULT_CONTEXT = Context();

class Parser
{
public:
    static Parser* instance()
    {
        Parser* instance = get_instance();
        if (instance == nullptr)
        {
            instance = new Parser();
        }
        return instance;
    }

    static void destroy()
    {
        Parser* instance = get_instance();
        delete instance;
        instance = nullptr;
    }

    Parser()
        : parser_(idl_grammar())
    {
        parser_.enable_ast();
    }

    Context parse(
            const std::string& idl_string)
    {
        Context context = DEFAULT_CONTEXT;
        parse(idl_string, context);
        return context;
    }

    bool parse(
            const std::string& idl_string,
            Context& context)
    {
        context.instance_ = this;
        std::shared_ptr<peg::Ast> ast;
        ignore_case_ = context.ignore_case;
        clear_ = context.clear;
        allow_kw_ids_ = context.allow_keyword_identifiers;
        std::string idl_to_parse = idl_string;
        preprocessor(context.preprocessor_exec);
        include_paths_ = context.include_paths;
        if (context.preprocess)
        {
            idl_to_parse = preprocess_string(idl_to_parse);
        }
        if (!parser_.parse(idl_to_parse.c_str(), ast))
        {
            context.success = false;
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast);
        root_scope_->fill_all_types(context.structs);
        //TODO: root_scope->fill_context(context);
        context.success = true;
        return true;
    }

    Context parse_file(
            const std::string& idl_file)
    {
        Context context = DEFAULT_CONTEXT;
        parse_file(idl_file, context);
        return context;
    }

    bool parse_file(
            const std::string& idl_file,
            Context& context)
    {
        context.instance_ = this;
        std::vector<char> source;
        std::shared_ptr<peg::Ast> ast;
        ignore_case_ = context.ignore_case;
        clear_ = context.clear;
        allow_kw_ids_ = context.allow_keyword_identifiers;
        preprocessor(context.preprocessor_exec);
        include_paths_ = context.include_paths;
        if (context.preprocess)
        {
            std::string file_content = preprocess_file(idl_file);
            return parse(file_content, context);
        }
        else
        {
            if (!(read_file(idl_file.c_str(), source)
                  && parser_.parse_n(source.data(), source.size(), ast, idl_file.c_str())))
            {
                context.success = false;
                return false;
            }

            ast = peg::AstOptimizer(true).optimize(ast);
            build_on_ast(ast);
            root_scope_->fill_all_types(context.structs);
            //TODO: root_scope->fill_context(context);
            context.success = true;
            return true;
        }
    }

    void get_all_types(
            std::map<std::string, DynamicType::Ptr>& types_map)
    {
        if (root_scope_)
        {
            root_scope_->fill_all_types(types_map);
        }
    }

    class exception
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

        const std::string& message() const
        {
            return message_;
        }

        const peg::Ast& ast() const
        {
            return *ast_;
        }

    };

    void preprocessor(
            const std::string& path)
    {
        if (!path.empty())
        {
            preprocessor_path_ = path;
        }
    }

    void include_paths(
            const std::vector<std::string>& include_paths)
    {
        include_paths_ = include_paths;
    }

private:
    friend struct Context;

    peg::parser parser_;
    bool ignore_case_ = false;
    bool preprocess_ = true;
    bool clear_ = true;
    bool allow_kw_ids_ = false;
    std::string preprocessor_path_ = "cpp";
    std::vector<std::string> include_paths_;

    static Parser* get_instance()
    {
        static Parser* instance_ = nullptr;
        return instance_;
    }

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

    std::string exec(
            const std::string& cmd) const
    {
        std::array<char, 256> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        return result;
    }

    void replace_all_string(
            std::string& str,
            const std::string& from,
            const std::string& to) const
    {
        size_t froms = from.size();
        size_t tos = to.size();
        size_t pos = str.find(from);
        while (pos != std::string::npos)
        {
            str.replace(pos, froms, to);
            pos = str.find(from, pos + tos);
        }
    }

    std::string preprocess_string(
            const std::string& idl_string) const
    {
        std::string args = "-H ";
        for (const std::string inc_path : include_paths_)
        {
            args += "-I " + inc_path + " ";
        }
        // Escape double quotes inside the idl_string
        std::string escaped_idl_string = idl_string;
        replace_all_string(escaped_idl_string, "\"", "\\\"");
        std::string cmd = "echo \"" + escaped_idl_string + "\" | " + preprocessor_path_ + " " + args;
        return exec(cmd);
    }

    std::string preprocess_file(
            const std::string& idl_file)
    {
        std::vector<std::string> includes;
        std::string args = "-H ";
        for (const std::string inc_path : include_paths_)
        {
            args += "-I " + inc_path + " ";
        }
        std::string cmd = preprocessor_path_ + " " + args + idl_file;
        std::string output = exec(cmd);
        Context context;
        context.clear = false;
        context.preprocess = false;
        context.ignore_case = ignore_case_;
        return output;
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

        DynamicData get_constant(
                const std::string& name) const
        {
            auto it = constants.find(name);
            if (it != constants.end())
            {
                return it->second;
            }

            if (outer != nullptr)
            {
                return outer->get_constant(name);
            }

            return DynamicData(primitive_type<bool>());
        }

        bool has_constant(
                const std::string& name) const
        {
            auto it = constants.find(name);
            if (it != constants.end())
            {
                return true;
            }

            if (outer != nullptr)
            {
                return outer->has_constant(name);
            }

            return false;
        }

        bool set_constant(
                const std::string& name,
                const DynamicData& value)
        {
            auto inserted = constants_types.emplace(name, value.type());
            if (inserted.second)
            {
                DynamicData temp(*(inserted.first->second));
                temp = value;
                auto result = constants.emplace(name, temp);
                return result.second;
            }
            return false;
        }

        std::map<std::string, DynamicType::Ptr> constants_types;
        std::map<std::string, DynamicData> constants;
        //std::map<std::string, std::shared_ptr<Module>> modules;
        //std::map<std::string, DynamicType::Ptr> types;
        //std::map<std::string, std::shared_ptr<StructType>> structs;
        std::map<std::string, DynamicType::Ptr> structs;
        //std::map<std::string, std::shared_ptr<AnnotationType>> annotations;
        std::shared_ptr<SymbolScope> outer;
        std::map<std::string, std::shared_ptr<SymbolScope>> inner;
        std::string name;
    };

    std::shared_ptr<SymbolScope> root_scope_;

    std::shared_ptr<SymbolScope> build_on_ast(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> scope = nullptr)
    {
        using namespace peg::udl;
        if (scope == nullptr)
        {
            if (clear_ || root_scope_ == nullptr)
            {
                root_scope_ = std::make_shared<SymbolScope>(nullptr);
            }
            scope = root_scope_;
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
                annotation_dcl(ast, scope);
                break;
            case "BITSET_DCL"_:
                bitset_dcl(ast, scope);
                break;
            case "BITMASK_DCL"_:
                bitmask_dcl(ast, scope);
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

        if (!allow_kw_ids_ && is_token(identifier))
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
        }
        else if (ast->nodes[1]->tag == "ARRAY_DECLARATOR"_)
        {
            auto& node = ast->nodes[1];
            name = resolve_identifier(node, node->nodes[0]->token, outer);
            for (size_t idx = 1; idx < node->nodes.size(); ++idx)
            {
                dimensions.push_back(std::atoi(node->nodes[idx]->token.c_str()));
            }
            type = get_array_type(dimensions, type);
        }

        std::cout << "Found \"typedef " << name << " for type " << type->name()
                  << "\" but typedefs aren't supported. Ignoring." << std::endl;
    }

    void const_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        std::string identifier = ast->nodes[1]->token;
        DynamicData expr(*type);
        expr = solve_expr(*type, ast->nodes[2], outer);

        std::cout << "Found const " << type->name() << " " << identifier << " = " << expr.to_string();

        outer->set_constant(identifier, expr);
    }

    bool get_literal_value(
            DynamicData& data,
            const std::shared_ptr<peg::Ast> ast) const
    {
        using namespace peg::udl;
        const unsigned int tag = ast->tag;
        const std::string& literal = ast->token;
        int base = 10;
        if (literal.find("0x") == 0 || literal.find("0X") == 0)
        {
            base = 16;
        }
        else if (literal.find("0") == 0)
        {
            base = 8;
        }

        switch (data.type().kind())
        {
            case TypeKind::INT_8_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                int8_t value = static_cast<int8_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_8_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                uint8_t value = static_cast<uint8_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                int16_t value = static_cast<int16_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                uint16_t value = static_cast<uint16_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                int32_t value = static_cast<int32_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                uint32_t value = static_cast<uint32_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                int64_t value = static_cast<int64_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    std::cout << "WARNING: Expected an INTEGER_LITERAL, found " << literal << std::endl;
                }
                uint64_t value = static_cast<uint64_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::CHAR_8_TYPE:
            {
                if (tag != "CHAR_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a CHAR_LITERAL, found " << literal << std::endl;
                }
                char value = literal.c_str()[0];
                data = value;
                break;
            }
            case TypeKind::CHAR_16_TYPE:
            {
                if (tag != "WIDE_CHAR_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a WIDE_CHAR_LITERAL, found " << literal << std::endl;
                }
                using convert_type = std::codecvt_utf8<wchar_t>;
                std::wstring_convert<convert_type, wchar_t> converter;
                std::wstring temp = converter.from_bytes(literal);
                wchar_t value = temp[0];
                data = value;
                break;
            }
            case TypeKind::STRING_TYPE:
            {
                std::string aux = literal.substr(literal.find("\"") + 1, literal.rfind("\"") - 1);
                for (auto& node : ast->nodes) // SUBSTRING_LITERAL
                {
                    aux += node->token.substr(node->token.find("\"") + 1, node->token.rfind("\"") - 1);
                }

                if (tag != "STRING_LITERAL"_ && tag != "SUBSTRING_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a STRING_LITERAL, found " << aux << std::endl;
                }

                data = aux;
                break;
            }
            case TypeKind::WSTRING_TYPE:
            {
                std::string aux = literal.substr(literal.find("\"") + 1, literal.rfind("\"") - 1);
                for (auto& node : ast->nodes) // SUBSTRING_LITERAL
                {
                    aux += node->token.substr(node->token.find("\"") + 1, node->token.rfind("\"") - 1);
                }

                if (tag != "WIDE_STRING_LITERAL"_ && tag != "WIDE_SUBSTRING_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a WIDE_STRING_LITERAL, found " << aux << std::endl;
                }
                using convert_type = std::codecvt_utf8<wchar_t>;
                std::wstring_convert<convert_type, wchar_t> converter;
                std::wstring value = converter.from_bytes(aux);
                data = value;
                break;
            }
            case TypeKind::BOOLEAN_TYPE:
            {
                if (tag != "BOOLEAN_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a BOOLEAN_LITERAL, found " << literal << std::endl;
                }
                if (literal == "TRUE")
                {
                    data = true;
                }
                else if (literal == "FALSE")
                {
                    data = false;
                }
                else
                {
                    std::cout << "Expected bool value (TRUE or FALSE) but found '" << literal
                              << "'. It will be take the value 'FALSE'." << std::endl;
                    data = false;
                }
                break;
            }
            case TypeKind::FLOAT_32_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a FLOAT_LITERAL, found " << literal << std::endl;
                }
                float value = std::stof(literal, nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_64_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a FLOAT_LITERAL, found " << literal << std::endl;
                }
                double value = std::stod(literal, nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_128_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    std::cout << "WARNING: Expected a FLOAT_LITERAL, found " << literal << std::endl;
                }
                long double value = std::stold(literal, nullptr);
                data = value;
                break;
            }
            /*
            case TypeKind::ALIAS_TYPE:
            {
                break;
            }
            */
            default:
                return false;
        }
        return true;
    }

    DynamicData solve_expr(
            const DynamicType& type,
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer) const
    {
        using namespace peg::udl;
        DynamicData result(type);
        switch (ast->tag)
        {
            case "INTEGER_LITERAL"_:
            case "FLOAT_LITERAL"_:
            case "FIXED_PT_LITERAL"_:
            case "CHAR_LITERAL"_:
            case "WIDE_CHAR_LITERAL"_:
            case "BOOLEAN_LITERAL"_:
            case "STRING_LITERAL"_:
            case "SUBSTRING_LITERAL"_:
            case "WIDE_STRING_LITERAL"_:
            case "WIDE_SUBSTRING_LITERAL"_:
            {
                get_literal_value(result, ast);
                break;
            }
            case "SCOPED_NAME"_:
                result = outer->get_constant(ast->token);
                break;
            case "UNARY_EXPR"_:
                {
                    result = solve_expr(type, ast->nodes[1], outer);
                    if (ast->nodes[0]->tag == "SUB_OP"_)
                    {
                        DynamicData temp(-result);
                        result = temp;
                    }
                }
                break;
            case "MULT_EXPR"_:
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    if (ast->nodes[1]->tag == "MULT_OP"_)
                    {
                        DynamicData temp(lho * rho);
                        result = temp;
                    }
                    else if (ast->nodes[1]->tag == "DIV_OP"_)
                    {
                        DynamicData temp(lho / rho);
                        result = temp;
                        //return lho / rho;
                    }
                    else if (ast->nodes[1]->tag == "MOD_OP"_)
                    {
                        DynamicData temp(lho % rho);
                        result = temp;
                    }
                }
                break;
            case "ADD_EXPR"_:
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    DynamicData temp(type);
                    if (ast->nodes[1]->tag == "ADD_OP"_)
                    {
                        temp = DynamicData(lho + rho);
                    }
                    else if (ast->nodes[1]->tag == "SUB_OP"_)
                    {
                        temp = DynamicData(lho - rho);
                    }
                    result = temp;
                }
                break;
            case "SHIFT_EXPR"_:
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    DynamicData temp(type);
                    if (ast->nodes[1]->tag == "LSHIFT_OP"_)
                    {
                        temp = DynamicData(lho << rho);
                    }
                    else if (ast->nodes[1]->tag == "RSHIFT_OP"_)
                    {
                        temp = DynamicData(lho >> rho);
                    }
                    result = temp;
                }
                break;
            case "AND_EXPR"_:
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    result = DynamicData(lho & rho);
                }
                break;
            case "XOR_EXPR"_:
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    result = DynamicData(lho ^ rho);
                }
                break;
            case "CONST_EXPR"_: // OR_EXPR
                {
                    DynamicData lho = solve_expr(type, ast->nodes[0], outer);
                    DynamicData rho = solve_expr(type, ast->nodes[2], outer);

                    result = DynamicData(lho | rho);
                }
                break;
        }
        return result;
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
        outer->unions[name] = DynamicType::Ptr(*union_type);
        */
    }

    void annotation_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::map<std::string, Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    break;
                }
                case "ANNOTATION_BODY"_:
                    //annotation_body(node, outer, member_list, type);
                    // TODO:
                    //     + ANNOTATION_BODY
                    //         + ANNOTATION_MEMBER
                    //             + ANNOTATION_MEMBER_TYPE/0[SIGNED_TINY_INT]
                    //             - SIMPLE_DECLARATOR/0[IDENTIFIER] (my_int8)
                    //         + ANNOTATION_MEMBER
                    //             + ANNOTATION_MEMBER_TYPE/0[STRING_TYPE]
                    //             - SIMPLE_DECLARATOR/0[IDENTIFIER] (my_string)
                    break;
            }
        }

        std::cout << "Found \"@annotation " << name << "\" but unions aren't supported. Ignoring." << std::endl;
        /* TODO
        AnnotationType annotation_type(name);
        for (auto& member : member_list)
        {
            annotation_type.add_member(std::move(member.second));
        }
        // Replace
        outer->annotations.emplace(name, annotation_type));
        */
    }

    void bitset_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::map<std::string, Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    break;
                }
                case "BITFIELD"_:
                    //bitfield(node, outer, member_list, type);
                    // TODO:
                    // + BITFIELD
                    //     - BITFIELD_SPEC/0[POSITIVE_INT_CONST] (3)
                    //     - IDENTIFIER (a)
                    // + BITFIELD
                    //     - BITFIELD_SPEC/0[POSITIVE_INT_CONST] (1)
                    //     - IDENTIFIER (b)
                    // - BITFIELD/0[POSITIVE_INT_CONST] (4)
                    // + BITFIELD
                    //     + BITFIELD_SPEC
                    //         - POSITIVE_INT_CONST (10)
                    //         + DESTINATION_TYPE/2[SIGNED_LONG_INT]
                    //     - IDENTIFIER (c)
                    break;
            }
        }

        std::cout << "Found \"bitset " << name << "\" but bitsets aren't supported. Ignoring." << std::endl;
        /* TODO
        BitsetType bitset_type(name);
        for (auto& member : member_list)
        {
            bitset_type.add_member(std::move(member.second));
        }
        // Replace
        outer->bitsets[name] = DynamicType::Ptr(bitset_type);
        */
    }

    void bitmask_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<SymbolScope> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::map<std::string, Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    break;
                }
                case "BIT_VALUE"_:
                    //switch_body(node, outer, member_list, type);
                    // TODO:
                    //  - BIT_VALUE/0[IDENTIFIER] (flag0)
                    //  - BIT_VALUE/0[IDENTIFIER] (flag1)
                    //  + BIT_VALUE
                    //      + ANNOTATION_APPL
                    //          - SCOPED_NAME (position)
                    //          - ANNOTATION_APPL_PARAMS/1[LITERAL] (5)
                    //      - IDENTIFIER (flag5)
                    //  - BIT_VALUE/0[IDENTIFIER] (flag6)
                    break;
            }
        }

        std::cout << "Found \"bitmask " << name << "\" but bitmasks aren't supported. Ignoring." << std::endl;
        /* TODO
        BitmaskType bitmask_type(name);
        for (auto& member : member_list)
        {
            bitmask_type.add_member(std::move(member.second));
        }
        // Replace
        outer->bitmasks[name] = DynamicType::Ptr(bitmask_type);
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
                return primitive_type<int8_t>();
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
                if (outer->has_constant(node->token))
                {
                    return StringType(get_dimension(node->token, outer, node));
                }
                return StringType(std::atoi(node->token.c_str()));
            case "WIDE_STRING_TYPE"_:
                if (outer->has_constant(node->token))
                {
                    return WStringType(get_dimension(node->token, outer, node));
                }
                return WStringType();
            case "WSTRING_SIZE"_:
                return WStringType(std::atoi(node->token.c_str()));
            case "SEQUENCE_TYPE"_:
            {
                DynamicType::Ptr inner_type = type_spec(node->nodes[0], outer);
                size_t size = 0;
                if (node->nodes.size() > 1)
                {
                    //size = std::atoi(node->nodes[1]->token.c_str());
                    size = get_dimension(outer, node->nodes[1]);
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
                    //size = std::atoi(node->nodes[2]->token.c_str());
                    size = get_dimension(outer, node->nodes[1]);
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

        return DynamicType::Ptr();
    }

    size_t get_dimension(
            const std::string& value,
            std::shared_ptr<SymbolScope> outer,
            const std::shared_ptr<peg::Ast> node)
    {
        DynamicData c_data = outer->get_constant(value);
        size_t dim = 0;
        switch (c_data.type().kind())
        {
            case TypeKind::INT_8_TYPE:
                dim = c_data.value<int8_t>();
                break;
            case TypeKind::UINT_8_TYPE:
                dim = c_data.value<uint8_t>();
                break;
            case TypeKind::INT_16_TYPE:
                dim = c_data.value<int16_t>();
                break;
            case TypeKind::UINT_16_TYPE:
                dim = c_data.value<uint16_t>();
                break;
            case TypeKind::INT_32_TYPE:
                dim = c_data.value<int32_t>();
                break;
            case TypeKind::UINT_32_TYPE:
                dim = c_data.value<uint32_t>();
                break;
            case TypeKind::INT_64_TYPE:
                dim = c_data.value<int64_t>();
                break;
            case TypeKind::UINT_64_TYPE:
                dim = c_data.value<uint64_t>();
                break;
            default:
                throw exception("Only a positive intenger number can be used as dimension.", node);
        }
        return dim;
    }

    size_t get_dimension(
            std::shared_ptr<SymbolScope> outer,
            const std::shared_ptr<peg::Ast> node)
    {
        using namespace peg::udl;

        if (node->tag == "SCOPED_NAME"_)
        {
            return get_dimension(node->token, outer, node);
        }
        else if (!node->token.empty())
        {
            return std::stoul(node->token);
        }
        return 0;
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
                        case "INTEGER_LITERAL"_:
                        {
                            dimensions.push_back(std::stoul(subnode->token.c_str()));
                            break;
                        }
                        case "SCOPED_NAME"_:
                        {
                            DynamicData c_data = outer->get_constant(subnode->token);
                            size_t dim = 0;
                            switch (c_data.type().kind())
                            {
                                case TypeKind::INT_8_TYPE:
                                    dim = c_data.value<int8_t>();
                                    break;
                                case TypeKind::UINT_8_TYPE:
                                    dim = c_data.value<uint8_t>();
                                    break;
                                case TypeKind::INT_16_TYPE:
                                    dim = c_data.value<int16_t>();
                                    break;
                                case TypeKind::UINT_16_TYPE:
                                    dim = c_data.value<uint16_t>();
                                    break;
                                case TypeKind::INT_32_TYPE:
                                    dim = c_data.value<int32_t>();
                                    break;
                                case TypeKind::UINT_32_TYPE:
                                    dim = c_data.value<uint32_t>();
                                    break;
                                case TypeKind::INT_64_TYPE:
                                    dim = c_data.value<int64_t>();
                                    break;
                                case TypeKind::UINT_64_TYPE:
                                    dim = c_data.value<uint64_t>();
                                    break;
                                default:
                                    throw exception("Only a positive intenger number can be used as dimension.", node);
                            }
                            dimensions.push_back(dim);
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

void Context::clear_context()
{
    if (clear)
    {
        if (Parser::get_instance() == instance_)
        {
            Parser::destroy();
        }
    }
}

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDL_PARSER_HPP_
