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

#define CPP_PEGLIB_LINE_COUNT_ERROR 7

#include <peglib.h>

#include <xtypes/ArrayType.hpp>
#include <xtypes/DynamicData.hpp>
#include <xtypes/SequenceType.hpp>
#include <xtypes/StringConversion.hpp>
#include <xtypes/StringType.hpp>
#include <xtypes/StructType.hpp>

#include <xtypes/idl/Module.hpp>
#include <xtypes/idl/grammar.hpp>


#ifdef _MSC_VER
#   include <cstdio>
#else
#   include <stdlib.h>
#   include <unistd.h>
#endif //_MSC_VER

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <string_view>
#include <thread>
#include <type_traits>
#include <vector>

// mimic posix pipe APIs
#ifdef _MSC_VER
#   pragma push_macro("popen")
#   define popen _popen
#   pragma push_macro("pipe")
#   define pipe _pipe
#   pragma push_macro("pclose")
#   define pclose _pclose
#endif

// define preprocessor strategy
#ifdef _MSC_VER
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cl /EP /I."
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::temporary_file
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "/I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>nul"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "rt"
#else
#   define EPROSIMA_PLATFORM_PREPROCESSOR "cpp -H"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY preprocess_strategy::pipe_stdin
#   define EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES "-I"
#   define EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR " 2>/dev/null"
#   define EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS "r"
#endif

namespace peg {

using Ast = AstBase<EmptyType>;

} // namespace peg

namespace eprosima {
namespace xtypes {
namespace idl {

namespace log {

enum LogLevel
{
    xERROR,
    xWARNING,
    xINFO,
    xDEBUG
};

struct LogEntry
{
    std::string path;
    size_t line;
    size_t column;
    LogLevel level;
    std::string category;
    std::string message;

    LogEntry(
            const std::string& file,
            size_t line_number,
            size_t column_number,
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
            case xERROR:
                ss << "ERROR";
                break;
            case xWARNING:
                ss << "WARNING";
                break;
            case xINFO:
                ss << "INFO";
                break;
            case xDEBUG:
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

} // namespace log

class LogContext
{
    mutable std::vector<log::LogEntry> log_;
    log::LogLevel log_level_ = log::LogLevel::xWARNING;
    bool print_log_ = false;

public:

    const std::vector<log::LogEntry>& log() const
    {
        return log_;
    }

    std::vector<log::LogEntry> log(
            log::LogLevel level,
            bool strict = false) const
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

    void log_level(
            log::LogLevel level)
    {
        log_level_ = level;
    }

    log::LogLevel log_level() const
    {
        return log_level_;
    }

    void print_log(
            bool enable)
    {
        print_log_ = enable;
    }

    // Logging
    void log(
            log::LogLevel level,
            const std::string& category,
            const std::string& message,
            std::shared_ptr<peg::Ast> ast = nullptr) const
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

class PreprocessorContext
    : public LogContext
{
public:

    // Preprocessors capability to use shared memory (pipes) or stick to file input
    enum class preprocess_strategy
    {
        pipe_stdin,
        temporary_file
    };

    bool preprocess = true;
    std::string preprocessor_exec = EPROSIMA_PLATFORM_PREPROCESSOR;
    std::string error_redir = EPROSIMA_PLATFORM_PREPROCESSOR_ERRORREDIR;
    preprocess_strategy strategy = EPROSIMA_PLATFORM_PREPROCESSOR_STRATEGY;
    std::string include_flag = EPROSIMA_PLATFORM_PREPROCESSOR_INCLUDES;
    std::vector<std::string> include_paths;

    std::string preprocess_file(
            const std::string& idl_file) const
    {
        std::vector<std::string> includes;
        std::string args;
        for (const std::string& inc_path : include_paths)
        {
            args += include_flag + inc_path + " ";
        }

        std::string cmd = preprocessor_exec + " " + args + idl_file + error_redir;

        log(log::LogLevel::xDEBUG, "PREPROCESS",
                "Calling preprocessor with command: " + cmd);
        std::string output = exec(cmd);
        return output;
    }

    std::string preprocess_string(const std::string& idl_string) const;

private:

    template<preprocess_strategy e>
    std::string preprocess_string(const std::string& idl_string) const;

#ifdef _MSC_VER
    std::pair<std::ofstream, std::filesystem::path> get_temporary_file() const
    {
        // Create temporary filename
        char filename_buffer[L_tmpnam];
        auto res = std::tmpnam(filename_buffer);
        xtypes_assert(res, "Unable to create a temporary file", true);

        std::filesystem::path tmp(filename_buffer);
        std::ofstream tmp_file(tmp);
        xtypes_assert(tmp_file, "Unable to create a temporary file", true);

        return std::make_pair(std::move(tmp_file), std::move(tmp));
    }
#else
    std::pair<std::ofstream, std::filesystem::path> get_temporary_file() const
    {
        // Create temporary filename
        static const std::filesystem::path tmpdir = std::filesystem::temp_directory_path();
        std::string tmp = (tmpdir / "xtypes_XXXXXX").string();
        int fd = mkstemp(tmp.data());
        xtypes_assert(fd != -1, "Unable to create a temporary file", true);

        std::ofstream tmp_file(tmp, std::ios_base::trunc | std::ios_base::out);
        close(fd);
        xtypes_assert(tmp_file, "Unable to create a temporary file", true);

        return std::make_pair(std::move(tmp_file), std::move(tmp));
    }
#endif // _MSC_VER

    void replace_all_string(
            std::string& str,
            const std::string& from,
            const std::string& to) const
    {
        size_t froms = from.size();
        size_t tos = to.size();
        size_t pos = str.find(from);
        const std::string escaped = "\\\\\"";
        size_t escaped_size = escaped.size();
        while (pos != std::string::npos)
        {
            str.replace(pos, froms, to);
            pos = str.find(from, pos + tos);
            while (str[pos - 1] == '\\')
            {
                str.replace(pos, froms, escaped);
                pos = str.find(from, pos + escaped_size);
            }

        }
    }

    std::string exec(
            const std::string& cmd) const
    {
        std::unique_ptr<FILE, decltype(& pclose)> pipe(
                popen(cmd.c_str(), EPROSIMA_PLATFORM_PIPE_OPEN_FLAGS), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

    #ifdef _MSC_VER
        std::filebuf buff(pipe.get());
        std::ostringstream os;
        os << &buff;
        return os.str();
    #else
        std::array<char, 256> buffer;
        std::string result;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        return result;
    #endif // _MSC_VER
    }
};

// preprocessing using pipes
template<>
inline std::string PreprocessorContext::preprocess_string<PreprocessorContext::preprocess_strategy::pipe_stdin>(
            const std::string& idl_string) const
{
    std::string args;
    for (const std::string& inc_path : include_paths)
    {
        args += include_flag + inc_path + " ";
    }
    // Escape double quotes inside the idl_string
    std::string escaped_idl_string = idl_string;
    replace_all_string(escaped_idl_string, "\"", "\\\"");
    std::string cmd = "echo \"" + escaped_idl_string + "\" | "
            + preprocessor_exec + " " + args + error_redir;

    log(log::LogLevel::xDEBUG, "PREPROCESS",
            "Calling preprocessor '" + preprocessor_exec + "' for an IDL string.");

    return exec(cmd);
}

// preprocessing using files
template<>
inline std::string PreprocessorContext::preprocess_string<PreprocessorContext::preprocess_strategy::temporary_file>(
            const std::string& idl_string) const
{
    auto [os, tmp] = get_temporary_file();

    // Populate
    os << idl_string;
    os.close();

    auto processed = preprocess_file(tmp.string());

    // dispose
    std::filesystem::remove(tmp);

    return processed;
}

inline std::string PreprocessorContext::preprocess_string(
        const std::string& idl_string) const
{
    switch(strategy)
    {
        case preprocess_strategy::pipe_stdin:
            return PreprocessorContext::preprocess_string<preprocess_strategy::pipe_stdin>(idl_string);
        case preprocess_strategy::temporary_file:
            return PreprocessorContext::preprocess_string<preprocess_strategy::temporary_file>(idl_string);
    }

    xtypes_assert(true, "unknown preprocessor strategy selected.", true)

    unreachable();
}

class Parser;

class Context
    : public PreprocessorContext
{
public:
    enum CharType
    {
        CHAR,
        UINT8,
        INT8
    };

    enum WideCharType
    {
        WCHAR_T,
        CHAR16_T
    };

    // Config
    bool ignore_case = false;
    bool clear = true;
    bool allow_keyword_identifiers = false;
    bool ignore_redefinition = false;
    CharType char_translation = CHAR;
    WideCharType wchar_type = WCHAR_T;

    static const Context DEFAULT_CONTEXT;

    // Results
    bool success = false;

    std::map<std::string, DynamicType::Ptr> get_all_types(
            bool scope = false)
    {
        std::map<std::string, DynamicType::Ptr> result;

        if(module_)
        {
            module_->fill_all_types(result, scope);
        }

        return result;
    }

    std::map<std::string, DynamicType::Ptr> get_all_scoped_types()
    {
        return get_all_types(true);
    }

    idl::Module& module()
    {
        if(!module_)
        {
            module_ = std::make_shared<idl::Module>();
        }
        return *module_;
    }

    inline void clear_context();

    ~Context()
    {
        clear_context();
    }

private:

    friend class Parser;
    std::shared_ptr<Parser> instance_;
    std::shared_ptr<idl::Module> module_;
};

inline const Context Context::DEFAULT_CONTEXT;

class Parser
    : public std::enable_shared_from_this<Parser>
{
public:

    static std::shared_ptr<Parser> instance(bool create = true)
    {
        if (!instance_ && create)
        {
            std::lock_guard<std::mutex> _(mtx_);
            instance_ = std::make_shared<Parser>();
        }

        return instance_;
    }

    static void destroy()
    {
        std::lock_guard<std::mutex> _(mtx_);
        instance_.reset();
    }

    Parser()
        : parser_(idl_grammar())
        , context_(nullptr)
    {
        parser_.enable_ast();
        parser_.set_logger(
                std::function<void(size_t line, size_t col, const std::string &msg)>(
                std::bind(
                    &Parser::parser_log_cb_,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3)));
    }

    Context parse(
            const std::string& idl_string)
    {
        Context context = Context::DEFAULT_CONTEXT;
        parse(idl_string, context);
        return context;
    }

    bool parse(
            const std::string& idl_string,
            Context& context)
    {
        context.instance_ = shared_from_this();
        context_ = &context;
        std::shared_ptr<peg::Ast> ast;

        if (!parser_.parse(idl_string.c_str(), ast))
        {
            context_->success = false;
            context_->log(log::LogLevel::xDEBUG, "RESULT",
                    "The parser found errors while parsing.");
            return false;
        }

        ast = parser_.optimize_ast(ast);
        build_on_ast(ast);
        context_->success = true;
        context_->log(log::LogLevel::xDEBUG, "RESULT",
                "The parser finished.");
        return true;
    }

    Context parse_file(
            const std::string& idl_file)
    {
        Context context = Context::DEFAULT_CONTEXT;
        parse_file(idl_file, context);
        return context;
    }

    Context parse_string(
            const std::string& idl_string)
    {
        Context context = Context::DEFAULT_CONTEXT;
        parse_string(idl_string, context);
        return context;
    }

    bool parse_file(
            const std::string& idl_file,
            Context& context)
    {
        context.instance_ = shared_from_this();
        context_ = &context;
        std::shared_ptr<peg::Ast> ast;
        if (context_->preprocess)
        {
            std::string file_content = context_->preprocess_file(idl_file);
            return parse(file_content, context);
        }

        std::ostringstream os;
        os << std::ifstream(idl_file).rdbuf();
        return parse(os.str(), context);
    }

    bool parse_string(
            const std::string& idl_string,
            Context& context)
    {
        context.instance_ = shared_from_this();

        if (context.preprocess)
        {
            return parse(context.preprocess_string(idl_string), context);
        }
        else
        {
            return parse(idl_string, context);
        }
    }

    void get_all_types(
            std::map<std::string, DynamicType::Ptr>& types_map)
    {
        if(context_)
        {
            context_->module().fill_all_types(types_map);
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
        {
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

    static std::string preprocess(
            const std::string& idl_file,
            const std::vector<std::string>& includes)
    {
        Context ctx = Context::DEFAULT_CONTEXT;
        ctx.include_paths = includes;
        return ctx.preprocess_file(idl_file);
    }

private:

    friend struct Context;
    using LabelsCaseMemberPair = std::pair<std::vector<std::string>, Member>;

    peg::parser parser_;
    Context* context_;
    static std::shared_ptr<Parser> instance_;
    static std::mutex mtx_;

    void parser_log_cb_(
            size_t l,
            size_t c,
            const std::string& msg
            ) const
    {
        context_->log(log::xDEBUG, "PEGLIB_PARSER", msg + " (" + std::to_string(
                    l - CPP_PEGLIB_LINE_COUNT_ERROR) + ":" + std::to_string(c) + ")");
    }

    bool read_file(
            const char* path,
            std::vector<char>& buff) const
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);

        if (ifs.fail())
        {
            context_->log(log::xDEBUG, "FILE",
                    "Cannot open file: " + std::string(path));
            return false;
        }

        buff.resize(static_cast<unsigned int>(ifs.seekg(0, std::ios::end).tellg()));

        if (!buff.empty())
        {
            ifs.seekg(0, std::ios::beg).read(&buff[0], static_cast<std::streamsize>(buff.size()));
        }
        context_->log(log::xDEBUG, "FILE",
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
        std::unique_ptr<FILE, decltype(& pclose)> pipe(popen(command.c_str(), "r"), pclose);
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
        const std::string escaped = "\\\\\"";
        size_t escaped_size = escaped.size();
        while (pos != std::string::npos)
        {
            str.replace(pos, froms, to);
            pos = str.find(from, pos + tos);
            while (str[pos - 1] == '\\')
            {
                str.replace(pos, froms, escaped);
                pos = str.find(from, pos + escaped_size);
            }

        }
    }

    std::string preprocess_string(
            const std::string& idl_string) const
    {
        std::string args = "-H ";
        for (const std::string& inc_path : context_->include_paths)
        {
            args += "-I " + inc_path + " ";
        }
        // Escape double quotes inside the idl_string
        std::string escaped_idl_string = idl_string;
        replace_all_string(escaped_idl_string, "\"", "\\\"");
        replace_all_string(escaped_idl_string, "#include", "\n#include");
        std::string cmd = "echo \"" + escaped_idl_string + "\" | " + context_->preprocessor_exec + " " + args;
        context_->log(log::xDEBUG, "PREPROCESS",
                "Calling preprocessor '" + context_->preprocessor_exec + "' for an IDL string.");
        return exec(cmd);
    }

    std::string preprocess_file(
            const std::string& idl_file)
    {
        std::vector<std::string> includes;
        std::string args = "-H ";
        for (const std::string& inc_path : context_->include_paths)
        {
            args += "-I " + inc_path + " ";
        }
        std::string cmd = context_->preprocessor_exec + " " + args + idl_file;
        context_->log(log::xDEBUG, "PREPROCESS",
                "Calling preprocessor with command: " + cmd);
        std::string output = exec(cmd);
        return output;
    }

    std::shared_ptr<Module> build_on_ast(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module> scope = nullptr)
    {
        using namespace peg::udl;

        if (scope == nullptr)
        {
            scope = context_->module().shared_from_this();
        }

        switch (ast->tag){
            case "ANNOTATION_APPL"_:
                // Not supported yet
                break;
            case "MODULE_DCL"_:
                module_dcl(ast, scope);
                break;
            case "CONST_DCL"_:
                const_dcl(ast, scope);
                break;
            case "STRUCT_DEF"_:
            case "IDENTIFIER"_: // An empty struct is reduced to an IDENTIFIER. No other type is allowed to be empty.
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
            const std::shared_ptr<peg::Ast>& ast,
            const std::string_view& identifier,
            std::shared_ptr<Module>& scope,
            bool ignore_already_used = false)
    {
        if (identifier.find("_") == 0)
        {
            std::stringstream message;
            message << "The identifier \"" << identifier << "\" is escaped. It will be replaced by \""
                    << identifier.substr(1) << "\"";
            context_->log(log::LogLevel::xINFO, "ESCAPED_IDENTIFIER", message.str(), ast);
            return std::string(identifier.substr(1).data(), identifier.substr(1).size()); // If the identifier starts with "_", remove the underscode and return.
        }

        if (is_token(identifier))
        {
            std::stringstream message;
            message << "The identifier \"" << identifier << "\" is a reserved word.";
            if (!context_->allow_keyword_identifiers)
            {
                context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
                throw exception(message.str(), ast);
            }
            context_->log(log::LogLevel::xINFO, "RESERVED_WORD", message.str(), ast);
        }

        if (scope->has_symbol(std::string(identifier.data(), identifier.size())))
        {
            std::stringstream message;
            message << "The identifier \"" << identifier << "\" is already used.";
            if (!ignore_already_used)
            {
                context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
                throw exception(message.str(), ast);
            }
            context_->log(log::LogLevel::xINFO, "ALREADY_USED", message.str(), ast);
        }

        context_->log(log::LogLevel::xDEBUG, "RESOLVE_IDENTIFIER",
                identifier.data(), ast);
        return std::string(identifier.data(), identifier.size());
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
            const std::string_view& identifier)
    {
        std::string aux_id(identifier.data(), identifier.size());

        if (context_->ignore_case)
        {
            to_lower(aux_id);
        }

        for (const auto& rule : parser_.get_grammar())
        {
            if (rule.first.find("KW_") == 0) // If it starts with "KW_", is a reserved word. You are welcome.
            {
                if (rule.second.parse(aux_id.c_str()).ret)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void module_dcl(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;
        std::shared_ptr<Module> scope;
        for (auto& node : ast->nodes)
        {
            switch (node->tag){
                case "IDENTIFIER"_:
                {
                    if (node->original_name == "IDENTIFIER") // MODULE
                    {
                        std::string name = resolve_identifier(node, node->token, outer, true);
                        if (!outer->has_submodule(name))
                        {
                            // New scope
                            outer->create_submodule(name);
                            scope = outer->submodule(name);
                            context_->log(log::LogLevel::xDEBUG, "MODULE_DCL",
                                    "New submodule: " + scope->scope(),
                                    ast);
                        }
                        else
                        {
                            // Adding to an already defined scope
                            scope = outer->submodule(name);
                            context_->log(log::LogLevel::xDEBUG, "MODULE_DCL",
                                    "Existing submodule: " + scope->scope(),
                                    ast);
                        }
                    }
                    else
                    {
                        // Empty struct
                        struct_def(node, scope);
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        std::string name;
        std::vector<size_t> dimensions;

        if (ast->nodes[1]->tag == "IDENTIFIER"_)
        {
            name = resolve_identifier(ast, ast->nodes[1]->token, outer, context_->ignore_redefinition);
        }
        else if (ast->nodes[1]->tag == "ARRAY_DECLARATOR"_)
        {
            auto& node = ast->nodes[1];
            name = resolve_identifier(node, node->nodes[0]->token, outer, context_->ignore_redefinition);
            for (size_t idx = 1; idx < node->nodes.size(); ++idx)
            {
                dimensions.push_back(std::atoi(node->nodes[idx]->token.data()));
            }
            type = get_array_type(dimensions, type);
        }

        outer->create_alias(std::move(type), name);
    }

    void const_dcl(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;

        DynamicType::Ptr type = type_spec(ast->nodes[0], outer);
        const auto& identifier = ast->nodes[1]->token;
        DynamicData expr(*type);
        expr = solve_expr(*type, ast->nodes[2], outer);

        std::stringstream message;
        message << "Found const " << type->name() << " " << identifier << " = " << expr.to_string();
        context_->log(log::LogLevel::xDEBUG, "DECLARATION", message.str(), ast);

        outer->create_constant(std::string(identifier.data(), identifier.size()), expr);
    }

    bool get_literal_value(
            DynamicData& data,
            const std::shared_ptr<peg::Ast>& ast) const
    {
        using namespace peg::udl;
        const unsigned int tag = ast->tag;
        const auto& literal = ast->token;
        int base = 10;
        if (literal.find("0x") == 0 || literal.find("0X") == 0)
        {
            base = 16;
        }
        else if (literal.find("0") == 0)
        {
            base = 8;
        }

        auto message =
                [&](const char* type) -> const std::string
                {
                    std::stringstream ss;
                    ss << "Expected an " << type << "_LITERAL, found " << literal;
                    return ss.str();
                };

        switch (data.type().kind())
        {
            case TypeKind::INT_8_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                int8_t value = static_cast<int8_t>(std::strtoll(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_8_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                uint8_t value = static_cast<uint8_t>(std::strtoull(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                int16_t value = static_cast<int16_t>(std::strtoll(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_16_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                uint16_t value = static_cast<uint16_t>(std::strtoull(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                int32_t value = static_cast<int32_t>(std::strtoll(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_32_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                uint32_t value = static_cast<uint32_t>(std::strtoull(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::INT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                int64_t value = static_cast<int64_t>(std::strtoll(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::UINT_64_TYPE:
            {
                if (tag != "INTEGER_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("INTEGER"), ast);
                }
                uint64_t value = static_cast<uint64_t>(std::strtoull(literal.data(), nullptr, base));
                data = value;
                break;
            }
            case TypeKind::CHAR_8_TYPE:
            {
                if (tag != "CHAR_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("CHAR"), ast);
                }
                char value = literal.at(0);
                data = value;
                break;
            }
            case TypeKind::CHAR_16_TYPE:
            {
                if (tag != "WIDE_CHAR_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("WIDE_CHAR"), ast);
                }

                std::basic_string<XTYPES_CHAR> aux(literal.begin(), literal.end());
                auto temp = code_conversion_tool<char16_t>(aux);
                data = temp[0];
                break;
            }
            case TypeKind::WIDE_CHAR_TYPE:
            {
                if (tag != "WIDE_CHAR_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("WIDE_CHAR"), ast);
                }

                std::basic_string<XTYPES_CHAR> aux(literal.begin(), literal.end());
                auto aux2 = code_conversion_tool<char16_t>(aux);
                std::wstring temp(aux2.begin(), aux2.end());
                data = temp[0];
                break;
            }
            case TypeKind::STRING_TYPE:
            {
                const auto& aux_view = literal.substr(literal.find("\"") + 1, literal.rfind("\"") - 1);
                std::string aux(aux_view.data(), aux_view.size());
                for (auto& node : ast->nodes) // SUBSTRING_LITERAL
                {
                    aux.append(node->token.substr(node->token.find("\"") + 1, node->token.rfind("\"") - 1));
                }

                if (tag != "STRING_LITERAL"_ && tag != "UNEXPECTED_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "STRING", message("STRING"), ast);
                }

                data = aux;
                break;
            }
            case TypeKind::WSTRING_TYPE:
            {
                const auto& aux_view = literal.substr(literal.find("\"") + 1, literal.rfind("\"") - 1);
                std::string aux(aux_view.data(), aux_view.size());
                for (auto& node : ast->nodes) // SUBSTRING_LITERAL
                {
                    aux.append(node->token.substr(node->token.find("\"") + 1, node->token.rfind("\"") - 1));
                }

                if (tag != "WIDE_STRING_LITERAL"_ && tag != "WIDE_SUBSTRING_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("WIDE_STRING"), ast);
                }

                std::basic_string<XTYPES_CHAR> aux2(literal.begin(), literal.end());
                auto temp = code_conversion_tool<char16_t>(aux2);
                data = std::wstring(temp.begin(), temp.end());
                break;
            }
            case TypeKind::STRING16_TYPE:
            {
                std::basic_string<XTYPES_CHAR> aux(literal.begin(), literal.end());
                data = code_conversion_tool<char16_t>(aux);
                break;
            }
            case TypeKind::BOOLEAN_TYPE:
            {
                if (tag != "BOOLEAN_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("BOOLEAN"), ast);
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
                    std::stringstream message;
                    message << "Expected bool value (TRUE or FALSE) but found '" << literal
                            << "'. It will be take the value 'FALSE'.";
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message.str(), ast);
                    data = false;
                }
                break;
            }
            case TypeKind::FLOAT_32_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("FLOAT"), ast);
                }
                float value = std::stof(literal.data(), nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_64_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("FLOAT"), ast);
                }
                double value = std::stod(literal.data(), nullptr);
                data = value;
                break;
            }
            case TypeKind::FLOAT_128_TYPE:
            {
                if (tag != "FLOAT_LITERAL"_)
                {
                    context_->log(log::LogLevel::xWARNING, "UNEXPECTED_LITERAL", message("FLOAT"), ast);
                }
                long double value = std::stold(literal.data(), nullptr);
                data = value;
                break;
            }
            default:
                context_->log(log::LogLevel::xERROR, "UNEXPECTED_LITERAL_TYPE",
                        message(data.type().name().c_str()), ast);
                return false;
        }
        return true;
    }

    DynamicData solve_expr(
            const DynamicType& type,
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer) const
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
                result = outer->constant(std::string(ast->token.data(), ast->token.size()));
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
                    DynamicData temp(lho* rho);
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;

        const auto& name_view = ast->nodes[0]->token;
        const std::string name(name_view.data(), name_view.size());
        EnumerationType<uint32_t> result(name); // TODO: Support other Enum types?, the grammar should be upgraded.
        context_->log(log::LogLevel::xDEBUG, "ENUM_DCL",
                "Found enum \"" + name + "\"",
                ast);
        for (size_t idx = 1; idx < ast->nodes.size(); ++idx)
        {
            const auto& token_view = ast->nodes[idx]->token;
            const std::string token(token_view.data(), token_view.size());
            context_->log(log::LogLevel::xDEBUG, "ENUM_DCL_VALUE",
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;
        std::string name = resolve_identifier(ast, ast->token, outer);
        if (outer->has_symbol(name, false))
        {
            std::stringstream message;
            message << "Struct " << ast->token << " was already declared.";
            context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
            throw exception(message.str(), ast);
        }

        StructType result(name);
        context_->log(log::LogLevel::xDEBUG, "STRUCT_FW_DCL",
                "Found forward struct declaration: \"" + name + "\"",
                ast);
        outer->structure(std::move(result));
    }

    void union_fw_dcl(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;
        std::string name = resolve_identifier(ast, ast->token, outer);
        if (outer->has_symbol(name, false))
        {
            std::stringstream message;
            message << "Union " << ast->token << " was already declared.";
            context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
            throw exception(message.str(), ast);
        }

        // TODO Replace by Unions. Kept as Struct to allow name solving.
        // Union forward declaration may be an special case of Union,
        // because they doesn't defines the switch type
        StructType result(name);
        context_->log(log::LogLevel::xDEBUG, "UNION_FW_DCL",
                "Found forward union declaration: \"" + name + "\"",
                ast);
        outer->structure(std::move(result));
    }

    void struct_def(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;
        std::string name;
        std::vector<Member> member_list;
        std::string_view parent_name;
        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "IDENTIFIER"_:
                {
                    name = resolve_identifier(ast, node->token, outer, true);
                    StructType result(name);
                    outer->structure(std::move(result));
                    break;
                }
                case "INHERITANCE"_:
                    if (outer->has_structure(std::string(node->token.data(), node->token.size())))
                    {
                        parent_name = node->token;
                    }
                    else
                    {
                        std::stringstream message;
                        message << "Struct \"" << name << "\" cannot inherit from \""
                                << node->token.data() << "\": Struct was not found.";
                        context_->log(log::LogLevel::xERROR, "STRUCT_DEF", message.str(), ast);
                        throw exception(message.str(), ast);
                    }
                    break;
                case "MEMBER"_:
                    member_def(node, outer, member_list);
                    break;
            }
        }

        if (name.empty() && ast->tag == "IDENTIFIER"_)
        {
            name = resolve_identifier(ast, ast->token, outer, true);
            StructType result(name);
            outer->structure(std::move(result));
        }

        StructType* struct_type = &outer->structure(name);
        if (!struct_type->members().empty())
        {
            const std::string msg = "Struct \"" + name + "\" redefinition.";
            if (context_->ignore_redefinition)
            {
                context_->log(log::LogLevel::xINFO, "REDEFINITION",
                        msg,
                        ast);
                return;
            }
            context_->log(log::LogLevel::xERROR, "EXCEPTION",
                    msg,
                    ast);
            throw exception(msg, ast);
        }
        context_->log(log::LogLevel::xDEBUG, "STRUCT_DEF",
                "Struct \"" + name + "\" definition.",
                ast);

        if (!parent_name.empty())
        {
            // Replace already existing struct with a new one which inherits
            StructType replace_struct(name, &outer->structure(std::string(parent_name.data(), parent_name.size())));
            outer->structure(std::move(replace_struct), true);
            struct_type = &outer->structure(name);
        }

        for (auto& member : member_list)
        {
            context_->log(log::LogLevel::xDEBUG, "STRUCT_DEF_MEMBER",
                    "Struct \"" + name + "\" member: {" + member.name() +
                    ": " + member.type().name() + "}", ast);
            struct_type->add_member(std::move(member));
        }
    }

    void union_def(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
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
        if (union_type.members().size() > 1)
        {
            const std::string msg = "Union \"" + name + "\" redefinition.";
            if (context_->ignore_redefinition)
            {
                context_->log(log::LogLevel::xINFO, "REDEFINITION",
                        msg,
                        ast);
                return;
            }
            context_->log(log::LogLevel::xERROR, "EXCEPTION",
                    msg,
                    ast);
            throw exception(msg, ast);
        }
        context_->log(log::LogLevel::xDEBUG, "UNION_DEF",
                "Union \"" + name + "\" definition.",
                ast);
        for (auto& pair : member_list)
        {
            Member& member = pair.second;
            context_->log(log::LogLevel::xDEBUG, "UNION_DEF_MEMBER",
                    "Union \"" + name + "\" member: " + member.name(),
                    ast);
            union_type.add_case_member(pair.first, std::move(member));
        }
    }

    void switch_body(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer,
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
                    switch_case(node, outer, member_list, type);
                    break;
                }
                default:
                {
                    context_->log(log::LogLevel::xERROR, "UNSUPPORTED",
                            "Found unexepcted node \"" + node->name + "\" while parsing an Union. Ignoring.",
                            node);
                }
            }
        }
    }

    void switch_case(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer,
            std::vector<LabelsCaseMemberPair>& member_list,
            const DynamicType::Ptr)
    {
        using namespace peg::udl;
        std::vector<std::string> labels;
        std::vector<Member> member;

        for (const auto& node : ast->nodes)
        {
            switch (node->original_tag)
            {
                case "CASE_LABEL"_:
                {
                    const auto& value_view = node->token;
                    std::string value(value_view.data(), value_view.size());
                    if (outer->has_constant(value))
                    {
                        DynamicData constant = outer->constant(value);
                        value = constant.cast<std::string>();
                    }
                    labels.emplace_back(std::move(value));
                    break;
                }
                case "ELEMENT_SPEC"_:
                {
                    case_member_def(node, outer, member);
                    // Label cases only have one member per case.
                    member_list.emplace_back(std::make_pair(std::move(labels), std::move(member.at(0))));
                    break;
                }
            }
        }
    }

    void case_member_def(
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer,
            std::vector<Member>& result)
    {
        using namespace peg::udl;
        DynamicType::Ptr type;

        using id_pair = std::pair<std::string, std::vector<size_t> >;
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
                std::stringstream message;
                message << "Member identifier " << ast->token << " already defined";
                context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
                throw exception(message.str(), ast);
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
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

        context_->log(log::LogLevel::xWARNING, "UNSUPPORTED",
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
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

        context_->log(log::LogLevel::xWARNING, "UNSUPPORTED",
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer)
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

        context_->log(log::LogLevel::xWARNING, "UNSUPPORTED",
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer,
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
            const std::shared_ptr<peg::Ast>& node, //ast,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;

        const auto& token_view = node->token;
        const std::string token(token_view.data(), token_view.size());

        switch (node->tag)
        {
            case "SCOPED_NAME"_: // Scoped name
            case "IDENTIFIER"_:
            {
                DynamicType::Ptr type = outer->type(token);
                if (type.get() == nullptr)
                {
                    context_->log(log::LogLevel::xERROR, "EXCEPTION",
                            "Member type " + token + " is unknown",
                            node);
                    throw exception("Member type " + token + " is unknown", node);
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
                    default:
                        xtypes_assert(false, "invalid char type")
                        return primitive_type<char>();
                }
            }
            case "WIDE_CHAR_TYPE"_:
                if (context_->wchar_type == Context::WCHAR_T)
                {
                    return primitive_type<wchar_t>();
                }
                else if (context_->wchar_type == Context::CHAR16_T)
                {
                    return primitive_type<char16_t>();
                }
                else
                {
                    return primitive_type<wchar_t>();
                }
            case "STRING_TYPE"_:
                return StringType();
            case "STRING_SIZE"_:
                if (outer->has_constant(token))
                {
                    return StringType(get_dimension(token, outer, node));
                }
                return StringType(std::atoi(token.c_str()));
            case "WIDE_STRING_TYPE"_:
                if (outer->has_constant(token))
                {
                    if (context_->wchar_type == Context::WCHAR_T)
                    {
                        return WStringType(get_dimension(token, outer, node));
                    }
                    else // CHAR16_T
                    {
                        return String16Type(get_dimension(token, outer, node));
                    }
                }
                if (context_->wchar_type == Context::WCHAR_T)
                {
                    return WStringType();
                }
                else // CHAR16_T
                {
                    return String16Type();
                }
            case "WSTRING_SIZE"_:
                if (context_->wchar_type == Context::WCHAR_T)
                {
                    return WStringType(std::atoi(token.c_str()));
                }
                else // CHAR16_T
                {
                    return String16Type(std::atoi(token.c_str()));
                }
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
                return MapType(*key_type, *inner_type, size);
            }
            default:
                return type_spec(node, outer);
        }

        context_->log(log::LogLevel::xERROR, "UNKNOWN_TYPE",
                token,
                node);

        return DynamicType::Ptr();
    }

    size_t get_dimension(
            const std::string& value,
            std::shared_ptr<Module>& outer,
            const std::shared_ptr<peg::Ast>& node)
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
                context_->log(log::LogLevel::xERROR, "EXCEPTION",
                        "Only a positive integer number can be used as dimension.",
                        node);
                throw exception("Only a positive integer number can be used as dimension.", node);
        }
        return dim;
    }

    size_t get_dimension(
            std::shared_ptr<Module>& outer,
            const std::shared_ptr<peg::Ast>& node)
    {
        using namespace peg::udl;

        const auto& token_view = node->token;
        const std::string token(token_view.data(), token_view.size());

        if (node->tag == "SCOPED_NAME"_)
        {
            return get_dimension(token, outer, node);
        }
        else if (!token.empty())
        {
            return std::stoul(token);
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
            const std::shared_ptr<peg::Ast>& ast,
            std::shared_ptr<Module>& outer,
            const DynamicType::Ptr type,
            std::vector<Member>& result)
    {
        using namespace peg::udl;
        using id_pair = std::pair<std::string, std::vector<size_t> >;
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
                std::stringstream message;
                message << "Member identifier " << ast->token << " already defined";
                context_->log(log::LogLevel::xERROR, "EXCEPTION", message.str(), ast);
                throw exception(message.str(), ast);
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

    std::vector<std::pair<std::string, std::vector<size_t> > > identifier_list(
            const std::shared_ptr<peg::Ast>& node,
            std::shared_ptr<Module>& outer)
    {
        using namespace peg::udl;
        std::vector<std::pair<std::string, std::vector<size_t> > > result;

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
            const std::shared_ptr<peg::Ast>& node,
            std::shared_ptr<Module>& outer,
            std::vector<std::pair<std::string, std::vector<size_t> > >& list)
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
                            dimensions.push_back(std::stoul(subnode->token.data()));
                            break;
                        }
                        case "SCOPED_NAME"_:
                        {
                            DynamicData c_data =
                                    outer->constant(std::string(subnode->token.data(),
                                            subnode->token.size()));
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
                                    context_->log(log::LogLevel::xERROR, "EXCEPTION",
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

inline std::shared_ptr<Parser> Parser::instance_;
inline std::mutex Parser::mtx_;

void Context::clear_context()
{
    if (clear)
    {
        instance_.reset();
        module_.reset();
    }
}

} // namespace idl
} // namespace xtypes
} // namespace eprosima


#ifdef _MSVC_LANG
#   pragma pop_macro("popen")
#   pragma pop_macro("pipe")
#   pragma pop_macro("pclose")
#endif

#endif // EPROSIMA_XTYPES_IDL_PARSER_HPP_
