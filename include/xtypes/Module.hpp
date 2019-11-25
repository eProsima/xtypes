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

#ifndef EPROSIMA_XTYPES_MODULE_HPP_
#define EPROSIMA_XTYPES_MODULE_HPP_

#include <xtypes/xtypes.hpp>

namespace eprosima {
namespace xtypes {

class Module
{
protected:
    using PairModuleSymbol = std::pair<const Module*, std::string>;

public:
    Module()
        : outer(nullptr)
        , name_("")
    {}

    Module& create_submodule(
            const std::string& submodule)
    {
        Module* new_module = new Module(this, submodule);
        auto result = inner.emplace(submodule, new_module);
        //auto result = inner.emplace(submodule, std::make_shared<Module>(this, submodule));
        return *result.first->second.get();
    }

    Module& operator [] (
            const std::string& submodule)
    {
        return *inner[submodule];
    }

    bool emplace(
            std::shared_ptr<Module>&& module)
    {
        if (module->name_.find("::") != std::string::npos)
        {
            return false; // Cannot add a module with scoped name.
        }
        auto result = inner.emplace(module->name_, std::move(module));
        return result.second;
    }

    const std::string& name() const
    {
        return name_;
    }

    std::string scope()
    {
        if (outer != nullptr && !outer->scope().empty())
        {
            return outer->scope() + "::" + name_;
        }
        return name_;
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

    bool has_struct(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->structs.count(module.second) > 0;
    }

    const StructType& get_struct(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            // This will fail
            return static_cast<const StructType&>(*structs.end()->second);
        }

        auto it = module.first->structs.find(module.second);
        if (it != module.first->structs.end())
        {
            return static_cast<const StructType&>(*it->second);
        }
        // This will fail
        return static_cast<const StructType&>(*structs.end()->second);
    }

    bool set_struct(
            const StructType& struct_type)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        auto result = structs.emplace(struct_type.name(), struct_type);
        return result.second;
    }

    bool set_struct(
            StructType&& struct_type)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        auto result = structs.emplace(struct_type.name(), std::move(struct_type));
        return result.second;
    }

    // TODO has, get and set of:
    // enums, bitmasks and unions

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
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return DynamicData(primitive_type<bool>());
        }

        auto it = module.first->constants.find(module.second);
        if (it != module.first->constants.end())
        {
            return it->second;
        }

        return DynamicData(primitive_type<bool>());
    }

    bool has_constant(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }

        auto it = module.first->constants.find(module.second);
        if (it != module.first->constants.end())
        {
            return true;
        }

        return false;
    }

    bool set_constant(
            const std::string& name,
            const DynamicData& value)
    {
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

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

protected:
    std::map<std::string, DynamicType::Ptr> constants_types;
    std::map<std::string, DynamicData> constants;
    std::map<std::string, DynamicType::Ptr> structs;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations;
    Module* outer;
    std::map<std::string, std::shared_ptr<Module>> inner;
    std::string name_;

    Module(
            Module* outer,
            const std::string& name)
        : outer(outer)
        , name_(name)
    {
    }

    // Auxiliar method to resolve scoped. It will return the Module up to the last "::" by calling
    // recursively resolving each scoped name, looking for the scope path, and the symbol name without the scope.
    // If the path cannot be resolved, it will return nullptr as path, and the full given symbol name.
    PairModuleSymbol resolve_scope(
            const std::string& symbol_name) const
    {
        return resolve_scope(symbol_name, symbol_name, true);
    }

    PairModuleSymbol resolve_scope(
            const std::string& symbol_name,
            const std::string& original_name,
            bool first = false) const
    {
        if (!first && symbol_name == original_name)
        {
            // Loop trying to resolve the name. Abort failing.
            PairModuleSymbol pair;
            pair.first = nullptr;
            pair.second = original_name;
            return pair;
        }

        std::string name = symbol_name;
        // Solve scope
        if (symbol_name.find("::") != std::string::npos) // It is an scoped name
        {
            if (symbol_name.find("::") == 0) // Looking for root
            {
                if (outer == nullptr) // We are the root, now go down.
                {
                    return resolve_scope(symbol_name.substr(2), original_name);
                }
                else // We are not the root, go up, with the original name.
                {
                    return outer->resolve_scope(original_name, original_name, true);
                }
            }
            else // not looking for root
            {
                std::string inner_scope = symbol_name.substr(0, symbol_name.find("::"));
                // Maybe the current scope its me?
                if (inner_scope == name_)
                {
                    std::string innest_scope = inner_scope.substr(0, inner_scope.find("::"));
                    if (inner.count(innest_scope) > 0)
                    {
                        std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                        const auto& it = inner.find(innest_scope);
                        PairModuleSymbol result = it->second->resolve_scope(inner_name, original_name);
                        if (result.first != nullptr)
                        {
                            return result;
                        }
                    }
                }
                // Do we have a inner scope that matches?
                if (inner.count(inner_scope) > 0)
                {
                    std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                    const auto& it = inner.find(inner_scope);
                    return it->second->resolve_scope(inner_name, original_name);
                }
                // Try going back
                if (outer != nullptr && first)
                {
                    return outer->resolve_scope(original_name, original_name, true);
                }
                // Unknown scope
                PairModuleSymbol pair;
                pair.first = nullptr;
                pair.second = original_name;
                return pair;
            }
        }

        return std::make_pair<const Module*, std::string>(this, std::move(name));
    }

};

} // xtypes
} // eprosima

#endif // EPROSIMA_XTYPES_MODULE_HPP_
