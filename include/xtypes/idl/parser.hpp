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
#include <xtypes/Module.hpp>

#include <xtypes/idl/grammar.hpp>

#include <map>
#include <vector>
#include <fstream>
#include <memory>
#include <exception>
#include <locale>
#include <regex>
#include <functional>

namespace eprosima {
namespace xtypes {
namespace idl {

namespace log {

enum LogLevel
{
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

struct LogEntry
{
    std::string path;
    uint32_t line;
    uint32_t column;
    LogLevel level;
    std::string category;
    std::string message;

    LogEntry(
            const std::string& file,
            uint32_t line_number,
            uint32_t column_number,
            LogLevel log_level,
            const std::string& cat,
            const std::string& msg)
        : path(file)
        , line(line_number)
        , column(column_number)
        , level(log_level)
        , category(cat)
        , message(msg)
    {
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[";
        switch (level)
        {
        case ERROR:
            ss << "ERROR";
            break;
        case WARNING:
            ss << "WARNING";
            break;
        case INFO:
            ss << "INFO";
            break;
        case DEBUG:
            ss << "DEBUG";
            break;
        }
        ss << "] ";
        ss << category << ": ";
        ss << message;
        // TODO - path, line and column may be confusing to the user, because if preprocessed
        // they may change.
        // << "(";
        //if (!path.empty())
        //{
        //    ss << path << ":";
        //}
        //ss << line << ":" << column << ")";
        return ss.str();
    }
};

}

class Parser;

struct Context
{
    enum CharType
    {
        CHAR,
        UINT8,
        INT8
    };

    // Config
    bool ignore_case = false;
    bool clear = true;
    bool preprocess = true;
    bool allow_keyword_identifiers = false;
    bool ignore_redefinition = false;
    CharType char_translation = CHAR;
    std::string preprocessor_exec = "cpp";
    std::vector<std::string> include_paths;

    // Results
    bool success = false;

    std::map<std::string, DynamicType::Ptr> get_all_types(
            bool scope = false)
    {
        std::map<std::string, DynamicType::Ptr> result;
        if (module_ != nullptr)
        {
            module_->fill_all_types(result, scope);
        }
        return result;
    }

    std::map<std::string, DynamicType::Ptr> get_all_scoped_types()
    {
        return get_all_types(true);
    }

    Module& module()
    {
        return *module_;
    }

    ~Context()
    {
        clear_context();
    }

    // Logging
    const std::vector<log::LogEntry>& log() const
    {
        return log_;
    }

    std::vector<log::LogEntry> log(log::LogLevel level, bool strict = false) const
    {
        std::vector<log::LogEntry> result;
        for (const log::LogEntry& entry : log_)
        {
            if (entry.level == level || (!strict && entry.level < level))
            {
                result.push_back(entry);
            }
        }
        return result;
    }

    void log_level(log::LogLevel level)
    {
        log_level_ = level;
    }

    log::LogLevel log_level() const
    {
        return log_level_;
    }

    void print_log(bool enable)
    {
        print_log_ = enable;
    }

private:
    friend class Parser;
    Parser* instance_;
    std::shared_ptr<Module> module_ = nullptr;
    std::vector<log::LogEntry> log_;
    log::LogLevel log_level_ = log::LogLevel::WARNING;
    bool print_log_ = false;

    inline void clear_context();

    // Logging
    void log(
            log::LogLevel level,
            const std::string& category,
            const std::string& message,
            std::shared_ptr<peg::Ast> ast = nullptr)
    {
        if (log_level_ >= level)
        {
            if (ast != nullptr)
            {
                log_.emplace_back(ast->path, ast->line, ast->column, level, category, message);
            }
            else
            {
                log_.emplace_back("", 0, 0, level, category, message);
            }
            if (print_log_)
            {
                const log::LogEntry& entry = log_.back();
                std::cout << entry.to_string() << std::endl;
            }
        }
    }

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
        , context_(nullptr)
    {
        parser_.enable_ast();
        parser_.log = std::bind(&Parser::parser_log_cb_, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3);
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
        context_ = &context;
        std::shared_ptr<peg::Ast> ast;
        std::string idl_to_parse = idl_string;
        if (context.preprocess)
        {
            idl_to_parse = preprocess_string(idl_to_parse);
        }
        if (!parser_.parse(idl_to_parse.c_str(), ast))
        {
            context.success = false;
            context_->log(log::LogLevel::DEBUG, "RESULT",
                "The parser found errors while parsing.");
            return false;
        }
        ast = peg::AstOptimizer(true).optimize(ast);
        build_on_ast(ast);
        context.module_ = root_scope_;
        context.success = true;
        context_->log(log::LogLevel::DEBUG, "RESULT",
            "The parser finished.");
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
        context_ = &context;
        std::vector<char> source;
        std::shared_ptr<peg::Ast> ast;
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
                context_->log(log::LogLevel::DEBUG, "RESULT",
                    "The parser found errors while parsing.");
                return false;
            }

            ast = peg::AstOptimizer(true).optimize(ast);
            build_on_ast(ast);
            context.module_ = root_scope_;
            context.success = true;
            context_->log(log::LogLevel::DEBUG, "RESULT",
                "The parser finished.");
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

    class exception : public std::runtime_error
    {
    private:
        std::string message_;
        std::shared_ptr<peg::Ast> ast_;
    public:
        exception(
                const std::string& message,
                const std::shared_ptr<peg::Ast> ast)
            : std::runtime_error(
                  std::string("Parser exception (" + (ast->path.empty() ? "<no file>" : ast->path)
                  + ":" + std::to_string(ast->line)
                  + ":" + std::to_string(ast->column) + "): " + message))
            , message_(message)
            , ast_(ast)
        {}

        const std::string& message() const
        {
            return message_;
        }

        const peg::Ast& ast() const
        {
            return *ast_;
        }

    };

    static std::string preprocess(
            const std::string& preprocessor_path,
            const std::string& idl_file,
            const std::vector<std::string>& includes)
    {
        std::string args = "-H ";
        for (const std::string inc_path : includes)
        {
            args += "-I " + inc_path + " ";
        }
        std::string cmd = preprocessor_path + " " + args + idl_file;
        std::string output = exec(cmd);
        return output;
    }

private:
    friend struct Context;
    using LabelsCaseMemberPair = std::pair<std::vector<std::string>, Member>;

    peg::parser parser_;
    Context* context_;
    std::shared_ptr<Module> root_scope_;

    static Parser* get_instance()
    {
        static Parser* instance_ = nullptr;
        return instance_;
    }

    void parser_log_cb_(size_t l, size_t c, const std::string& msg) const
    {
        context_->log(log::DEBUG, "PEGLIB_PARSER", msg + " (" + std::to_string(l) + ":" + std::to_string(c) + ")");
    }

    bool read_file(
            const char* path,
            std::vector<char>& buff) const
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);

        if (ifs.fail())
        {
            context_->log(log::LogLevel::DEBUG, "FILE",
                "Cannot open file: " + std::string(path));
            return false;
        }

        buff.resize(static_cast<unsigned int>(ifs.seekg(0, std::ios::end).tellg()));

        if (!buff.empty())
        {
            ifs.seekg(0, std::ios::beg).read(&buff[0], static_cast<std::streamsize>(buff.size()));
        }
        context_->log(log::LogLevel::DEBUG, "FILE",
            "Loaded file: " + std::string(path));
        return true;
    }

    static std::string exec(
            const std::string& cmd,
            bool filter_stderr = true)
    {
        std::array<char, 256> buffer;
        std::string command = cmd;
        if (filter_stderr)
        {
            command.append(" 2> /dev/null");
        }
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
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
        for (const std::string inc_path : context_->include_paths)
        {
            args += "-I " + inc_path + " ";
        }
        // Escape double quotes inside the idl_string
        std::string escaped_idl_string = idl_string;
        replace_all_string(escaped_idl_string, "\"", "\\\"");
        std::string cmd = "echo \"" + escaped_idl_string + "\" | " + context_->preprocessor_exec + " " + args;
        context_->log(log::LogLevel::DEBUG, "PREPROCESS",
            "Calling preprocessor '" + context_->preprocessor_exec + "' for an IDL string.");
        return exec(cmd);
    }

    std::string preprocess_file(
            const std::string& idl_file)
    {
        std::vector<std::string> includes;
        std::string args = "-H ";
        for (const std::string inc_path : context_->include_paths)
        {
            args += "-I " + inc_path + " ";
        }
        std::string cmd = context_->preprocessor_exec + " " + args + idl_file;
        context_->log(log::LogLevel::DEBUG, "PREPROCESS",
            "Calling preprocessor with command: " + cmd);
        std::string output = exec(cmd);
        return output;
    }

    std::shared_ptr<Module> build_on_ast(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> scope = nullptr)
    {
        using namespace peg::udl;
        if (scope == nullptr)
        {
            if (context_->clear || root_scope_ == nullptr)
            {
                root_scope_ = std::make_shared<Module>();
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
            std::shared_ptr<Module> scope,
            bool ignore_already_used = false)
    {
        if (identifier.find("_") == 0)
        {
            context_->log(log::LogLevel::INFO, "ESCAPED_IDENTIFIER",
                "The identifier \"" + identifier + "\" is escaped. It will be replaced by \""
                + identifier.substr(1) + "\"",
                ast);
            return identifier.substr(1); // If the identifier starts with "_", remove the underscode and return.
        }

        if (is_token(identifier))
        {
            if (!context_->allow_keyword_identifiers)
            {
                context_->log(log::LogLevel::ERROR, "EXCEPTION",
                    "The identifier \"" + identifier + "\" is a reserved word.",
                    ast);
                throw exception("The identifier \"" + identifier + "\" is a reserved word.", ast);
            }
            context_->log(log::LogLevel::INFO, "RESERVED_WORD",
                "The identifier \"" + identifier + "\" is a reserved word.",
                ast);
        }

        if (scope->has_symbol(identifier))
        {
            if (!ignore_already_used)
            {
                context_->log(log::LogLevel::ERROR, "EXCEPTION",
                    "The identifier \"" + identifier + "\" is already used.",
                    ast);
                throw exception("The identifier \"" + identifier + "\" is already used.", ast);
            }
            context_->log(log::LogLevel::INFO, "ALREADY_USED",
                "The identifier \"" + identifier + "\" is already used.",
                ast);
        }

        context_->log(log::LogLevel::DEBUG, "RESOLVE_IDENTIFIER",
            identifier, ast);
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

        if (!context_->ignore_case)
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
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::shared_ptr<Module> scope;
        for (auto& node : ast->nodes)
        {
            switch (node->tag){
                case "IDENTIFIER"_:
                {
                    std::string name = resolve_identifier(node, node->token, outer, true);
                    if (!outer->has_submodule(name))
                    {
                        // New scope
                        outer->create_submodule(name);
                        scope = outer->submodule(name);
                        context_->log(log::LogLevel::DEBUG, "MODULE_DCL",
                            "New submodule: " + scope->scope(),
                            ast);
                    }
                    else
                    {
                        // Adding to an already defined scope
                        scope = outer->submodule(name);
                        context_->log(log::LogLevel::DEBUG, "MODULE_DCL",
                            "Existing submodule: " + scope->scope(),
                            ast);
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
            std::shared_ptr<Module> outer)
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

        outer->create_alias(std::move(type), name);
    }

    void const_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        std::string identifier = ast->nodes[1]->token;
        DynamicData expr(*type);
        expr = solve_expr(*type, ast->nodes[2], outer);

        context_->log(log::LogLevel::DEBUG, "DECLARATION",
            "Found const " + type->name() + " " + identifier + " = " + expr.to_string(),
            ast);

        outer->create_constant(identifier, expr);
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
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                int8_t value = static_cast<int8_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_8_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                uint8_t value = static_cast<uint8_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                int16_t value = static_cast<int16_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                uint16_t value = static_cast<uint16_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                int32_t value = static_cast<int32_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                uint32_t value = static_cast<uint32_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                int64_t value = static_cast<int64_t>(std::strtoll(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an INTEGER_LITERAL, found " + literal,
                        ast);
                }
                uint64_t value = static_cast<uint64_t>(std::strtoull(literal.c_str(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::CHAR_8_TYPE:
            {
                if (tag != "CHAR_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an CHAR_LITERAL, found " + literal,
                        ast);
                }
                char value = literal.c_str()[0];
                data = value;
                break;
            }
            case TypeKind::CHAR_16_TYPE:
            {
                if (tag != "WIDE_CHAR_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an WIDE_CHAR_LITERAL, found " + literal,
                        ast);
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

                if (tag != "STRING_LITERAL"_ && tag != "UNEXPECTED_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "STRING",
                        "Expected an STRING_LITERAL, found " + literal,
                        ast);
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
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an WIDE_STRING_LITERAL, found " + literal,
                        ast);
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
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected an BOOLEAN_LITERAL, found " + literal,
                        ast);
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
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected bool value (TRUE or FALSE) but found '" + literal
                        + "'. It will be take the value 'FALSE'.",
                        ast);
                    data = false;
                }
                break;
            }
            case TypeKind::FLOAT_32_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected a FLOAT_LITERAL, found " + literal,
                        ast);
                }
                float value = std::stof(literal, nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_64_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected a FLOAT_LITERAL, found " + literal,
                        ast);
                }
                double value = std::stod(literal, nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_128_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::WARNING, "UNEXPECTED_LITERAL",
                        "Expected a FLOAT_LITERAL, found " + literal,
                        ast);
                }
                long double value = std::stold(literal, nullptr);
                data = value;
                break;
            }
            default:
                context_->log(log::LogLevel::ERROR, "UNEXPECTED_LITERAL_TYPE",
                    "Unknown literal type: " + data.type().name() + " (" + literal + ")",
                    ast);
                return false;
        }
        return true;
    }

    DynamicData solve_expr(
            const DynamicType& type,
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer) const
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
                result = outer->constant(ast->token);
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
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;

        std::string name = ast->nodes[0]->token;
        EnumerationType<uint32_t> result(name); // TODO: Support other Enum types?, the grammar should be upgraded.
        context_->log(log::LogLevel::DEBUG, "ENUM_DCL",
            "Found enum \"" + name + "\"",
            ast);
        for (size_t idx = 1; idx < ast->nodes.size(); ++idx)
        {
            const std::string& token = ast->nodes[idx]->token;
            context_->log(log::LogLevel::DEBUG, "ENUM_DCL_VALUE",
                "Adding \"" + token + "\" to enum \"" + name + "\"",
                ast);
            result.add_enumerator(token);
            // Little hack. Don't judge me.
            DynamicData hack(primitive_type<uint32_t>());
            hack = result.value(token);
            outer->create_constant(name + "::" + token, hack, false, true); // Mark it as "from_enum"
            outer->create_constant(token, hack, false, true); // Typically both are accessible
            // End of hack
        }
        outer->enum_32(std::move(result));
    }

    void struct_fw_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name = resolve_identifier(ast, ast->token, outer);
        if (outer->has_symbol(name, false))
        {
            context_->log(log::LogLevel::ERROR, "EXCEPTION",
                "Struct " + ast->token + " was already declared.",
                ast);
            throw exception("Struct " + ast->token + " was already declared.", ast);
        }

        StructType result(name);
        context_->log(log::LogLevel::DEBUG, "STRUCT_FW_DCL",
            "Found forward struct declaration: \"" + name + "\"",
            ast);
        outer->structure(std::move(result));
    }

    void union_fw_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name = resolve_identifier(ast, ast->token, outer);
        if (outer->has_symbol(name, false))
        {
            context_->log(log::LogLevel::ERROR, "EXCEPTION",
                "Union " + ast->token + " was already declared.",
                ast);
            throw exception("Union " + ast->token + " was already declared.", ast);
        }

        // TODO Replace by Unions. Kept as Struct to allow name solving.
        StructType result(name);
        context_->log(log::LogLevel::DEBUG, "UNION_FW_DCL",
            "Found forward union declaration: \"" + name + "\"",
            ast);
        outer->structure(std::move(result));
    }

    void struct_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name;
        std::vector<Member> member_list;
        for (const auto& node : ast->nodes)
        {
            switch (node->tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    StructType result(name);
                    outer->structure(std::move(result));
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

        StructType& struct_type = outer->structure(name);
        if (!struct_type.members().empty())
        {
            const std::string msg = "Struct \"" + name + "\" redefinition.";
            if (context_->ignore_redefinition)
            {
                context_->log(log::LogLevel::INFO, "REDEFINITION",
                    msg,
                    ast);
                return;
            }
            context_->log(log::LogLevel::ERROR, "EXCEPTION",
                msg,
                ast);
            throw exception(msg, ast);
        }
        context_->log(log::LogLevel::DEBUG, "STRUCT_DEF",
            "Struct \"" + name + "\" definition.",
            ast);
        for (auto& member : member_list)
        {
            context_->log(log::LogLevel::DEBUG, "STRUCT_DEF_MEMBER",
                "Struct \"" + name + "\" member: " + member.name(),
                ast);
            struct_type.add_member(std::move(member));
        }
    }

    void union_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name;
        std::vector<LabelsCaseMemberPair> member_list;
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
                    UnionType result(name, *type);
                    outer->union_switch(std::move(result));
                    break;
                }
                case "SWITCH_BODY"_:
                    switch_body(node, outer, member_list, type);
                    break;
            }
        }

        UnionType& union_type = outer->union_switch(name);
        if (!union_type.members().empty())
        {
            const std::string msg = "Union \"" + name + "\" redefinition.";
            if (context_->ignore_redefinition)
            {
                context_->log(log::LogLevel::INFO, "REDEFINITION",
                    msg,
                    ast);
                return;
            }
            context_->log(log::LogLevel::ERROR, "EXCEPTION",
                msg,
                ast);
            throw exception(msg, ast);
        }
        context_->log(log::LogLevel::DEBUG, "UNION_DEF",
            "Union \"" + name + "\" definition.",
            ast);
        for (auto& pair : member_list)
        {
            context_->log(log::LogLevel::DEBUG, "UNION_DEF_MEMBER",
                "Union \"" + name + "\" member: " + pair.second.name(),
                ast);
            union_type.add_case_member(pair.first, std::move(pair.second));
        }
    }

    void switch_body(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer,
            std::vector<LabelsCaseMemberPair>& member_list,
            const DynamicType::Ptr type)
    {
        using namespace peg::udl;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "CASE"_:
                {
                    std::cout << "CASE FOUND: " << std::endl;
                    switch_case(node, outer, member_list, type);
                    break;
                }
                default:
                {
                    context_->log(log::LogLevel::ERROR, "UNSUPPORTED",
                       "Found unexepcted node \"" + node->name + "\" while parsing an Union. Ignoring.",
                        node);
                }
            }
        }
    }

    // TODO:
    //     + SWITCH_BODY
    //          + CASE
    //              - CASE_LABEL/0 (0)
    //              - CASE_LABEL/0 (2)
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

    void switch_case(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer,
            std::vector<LabelsCaseMemberPair>& member_list,
            const DynamicType::Ptr type)
    {
        using namespace peg::udl;
        for (const auto& node : ast->nodes)
        {
            std::vector<std::string> labels;
            std::vector<Member> member;

            switch (node->original_tag)
            {
                case "CASE_LABEL"_:
                {
                    std::cout << "LABEL: " << node->token << std::endl;
                    labels.push_back(node->token); // Raw token, must be processed later.
                    break;
                }
                case "ELEMENT_SPEC"_:
                {
                    case_member_def(node, outer, member);
                    std::cout << "Member: " << member.at(0).name() << std::endl;
                    // Label cases only have one member per case.
                    LabelsCaseMemberPair pair = std::make_pair(labels, std::move(member.at(0)));
                    member_list.emplace_back(std::move(pair));
                    break;
                }
            }
        }
    }

    void case_member_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer,
            std::vector<Member>& result)
    {
        using namespace peg::udl;
        DynamicType::Ptr type;

        using id_pair = std::pair<std::string, std::vector<size_t>>;
        using id_pair_vector = std::vector<id_pair>;

        id_pair_vector pairs;

        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "TYPE_SPEC"_:
                    type = type_spec(node, outer);
                    break;
                case "DECLARATOR"_:
                    identifier(node, outer, pairs);
                    break;
            }
        }

        for (id_pair& pair : pairs)
        {
            std::string name = pair.first;
            std::vector<size_t> dimensions = pair.second;

            bool already_defined = false;
            for (const Member& m : result)
            {
                if (m.name() == name)
                {
                    already_defined = true;
                    break;
                }
            }

            if (already_defined)
            {
                context_->log(log::LogLevel::ERROR, "EXCEPTION",
                    "Member identifier " + ast->token + " already defined",
                    ast);
                throw exception("Member identifier " + ast->token + " already defined", ast);
            }
            if (dimensions.empty())
            {
                result.emplace_back(Member(name, *type));
            }
            else
            {
                member_array(name, dimensions, type, result);
            }
        }
    }

    void annotation_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::vector<Member> member_list;
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

        context_->log(log::LogLevel::WARNING, "UNSUPPORTED",
            "Found \"@annotation " + name + "\" but annotations aren't supported. Ignoring.",
            ast);
        /* TODO
        AnnotationType annotation_type(name);
        for (auto& member : member_list)
        {
            annotation_type.add_member(std::move(member.second));
        }
        // Replace
        outer->set_annotation(annotation_type));
        */
    }

    void bitset_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::vector<Member> member_list;
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

        context_->log(log::LogLevel::WARNING, "UNSUPPORTED",
            "Found \"bitset " + name + "\" but bitsets aren't supported. Ignoring.",
            ast);
        /* TODO
        BitsetType bitset_type(name);
        for (auto& member : member_list)
        {
            bitset_type.add_member(std::move(member.second));
        }
        // Replace ?
        outer->set_bitset(bitset_type);
        */
    }

    void bitmask_dcl(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        std::string name;
        //std::vector<Member> member_list;
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

        context_->log(log::LogLevel::WARNING, "UNSUPPORTED",
            "Found \"bitmask " + name + "\" but bitmasks aren't supported. Ignoring.",
            ast);
        /* TODO
        BitmaskType bitmask_type(name);
        for (auto& member : member_list)
        {
            bitmask_type.add_member(std::move(member.second));
        }
        // Replace
        outer->set_bitmask(bitmask_type);
        */
    }

    void member_def(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer,
            std::vector<Member>& result)
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
            std::shared_ptr<Module> outer)
    {
        using namespace peg::udl;
        switch (node->tag)
        {
            case "SCOPED_NAME"_: // Scoped name
            case "IDENTIFIER"_:
            {
                DynamicType::Ptr type = outer->type(node->token);
                if (type.get() == nullptr)
                {
                    if (outer->has_alias(node->token))
                    {
                        return AliasType(outer->alias(node->token));
                    }
                    context_->log(log::LogLevel::ERROR, "EXCEPTION",
                        "Member type " + node->token + " is unknown",
                        node);
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
            {
                switch (context_->char_translation)
                {
                    case Context::CHAR:
                        return primitive_type<char>();
                    case Context::UINT8:
                        return primitive_type<uint8_t>();
                    case Context::INT8:
                        return primitive_type<int8_t>();
                }
            }
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
                    size = get_dimension(outer, node->nodes[2]);
                }
                context_->log(log::LogLevel::WARNING, "UNSUPPORTED",
                    "Found \"map<" + key_type->name() + ", " + inner_type->name()
                    + ((size > 0) ? (", " + std::to_string(size)) : "") + ">\" "
                    + "but maps aren't supported. Ignoring.",
                    node);
                break;
                // return MapType(*key_type, *inner_type, size); // TODO, uncomment when maps are implemented.
            }
            default:
                return type_spec(node, outer);
        }

        context_->log(log::LogLevel::ERROR, "UNKNOWN_TYPE",
            node->token,
            node);

        return DynamicType::Ptr();
    }

    size_t get_dimension(
            const std::string& value,
            std::shared_ptr<Module> outer,
            const std::shared_ptr<peg::Ast> node)
    {
        DynamicData c_data = outer->constant(value);
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
                context_->log(log::LogLevel::ERROR, "EXCEPTION",
                    "Only a positive intenger number can be used as dimension.",
                    node);
                throw exception("Only a positive intenger number can be used as dimension.", node);
        }
        return dim;
    }

    size_t get_dimension(
            std::shared_ptr<Module> outer,
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
            std::vector<Member>& result)
    {
        result.emplace_back(Member(name, *get_array_type(dimensions, type)));
    }

    void members(
            const std::shared_ptr<peg::Ast> ast,
            std::shared_ptr<Module> outer,
            const DynamicType::Ptr type,
            std::vector<Member>& result)
    {
        using namespace peg::udl;
        using id_pair = std::pair<std::string, std::vector<size_t>>;
        using id_pair_vector = std::vector<id_pair>;

        id_pair_vector pairs = identifier_list(ast, outer);
        for (id_pair& pair : pairs)
        {
            std::string name = pair.first;
            std::vector<size_t> dimensions = pair.second;

            bool already_defined = false;
            for (const Member& m : result)
            {
                if (m.name() == name)
                {
                    already_defined = true;
                    break;
                }
            }

            if (already_defined)
            {
                context_->log(log::LogLevel::ERROR, "EXCEPTION",
                    "Member identifier " + ast->token + " already defined",
                    ast);
                throw exception("Member identifier " + ast->token + " already defined", ast);
            }
            if (dimensions.empty())
            {
                result.emplace_back(Member(name, *type));
            }
            else
            {
                member_array(name, dimensions, type, result);
            }
        }
    }

    std::vector<std::pair<std::string, std::vector<size_t>>> identifier_list(
            const std::shared_ptr<peg::Ast> node,
            std::shared_ptr<Module> outer)
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
            std::shared_ptr<Module> outer,
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
                            DynamicData c_data = outer->constant(subnode->token);
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
                                    context_->log(log::LogLevel::ERROR, "EXCEPTION",
                                        "Only a positive intenger number can be used as dimension.",
                                        node);
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
        else
        {
            delete instance_;
            instance_ = nullptr;
        }
    }
}

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDL_PARSER_HPP_
