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
        : outer_(nullptr)
        , name_("")
    {}

    Module& create_submodule(
            const std::string& submodule)
    {
        std::shared_ptr<Module> new_submodule(new Module(this, submodule));
        auto result = inner_.emplace(submodule, new_submodule);
        return *result.first->second.get();
    }

    std::shared_ptr<Module> submodule(
            const std::string& submodule)
    {
        return inner_[submodule];
    }


    bool has_submodule(
            const std::string& submodule)
    {
        return inner_.count(submodule) > 0;
    }

    Module& operator [] (
            const std::string& submodule)
    {
        return *inner_[submodule];
    }

    const Module& operator [] (
            const std::string& submodule) const
    {
        return *inner_.at(submodule);
    }

    /* TODO - Probably should be removed.
    bool emplace(
            std::shared_ptr<Module>&& module)
    {
        if (module->name_.find("::") != std::string::npos)
        {
            return false; // Cannot add a module with scoped name.
        }
        module->outer_ = this;
        auto result = inner_.emplace(module->name_, std::move(module));
        return result.second;
    }
    */

    const std::string& name() const
    {
        return name_;
    }

    std::string scope()
    {
        if (outer_ != nullptr && !outer_->scope().empty())
        {
            return outer_->scope() + "::" + name_;
        }
        return name_;
    }

    bool has_symbol(
            const std::string& ident,
            bool extend = true) const
    {
        size_t n_elems = structs_.count(ident); // + constants_.count(ident) + members.count(ident) + types.count(ident);
        if (n_elems > 0)
        {
            return true;
        }
        if (extend && outer_ != nullptr)
        {
            return outer_->has_symbol(ident, extend);
        }
        return false;
    }

    bool has_structure(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->structs_.count(module.second) > 0;
    }

    const StructType& structure(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            // This will fail
            return static_cast<const StructType&>(*structs_.end()->second);
        }

        auto it = module.first->structs_.find(module.second);
        if (it != module.first->structs_.end())
        {
            return static_cast<const StructType&>(*it->second);
        }
        // This will fail
        return static_cast<const StructType&>(*structs_.end()->second);
    }

    StructType& structure(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            // This will fail
            return static_cast<StructType&>(const_cast<DynamicType&>(*structs_.end()->second));
        }

        auto it = module.first->structs_.find(module.second);
        if (it != module.first->structs_.end())
        {
            return static_cast<StructType&>(const_cast<DynamicType&>(*it->second));
        }
        // This will fail
        return static_cast<StructType&>(const_cast<DynamicType&>(*structs_.end()->second));
    }

    bool structure(
            const StructType& struct_type)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        auto result = structs_.emplace(struct_type.name(), struct_type);
        return result.second;
    }

    bool structure(
            StructType&& struct_type,
            bool replace = false)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = structs_.find(struct_type.name());
            if (it != structs_.end())
            {
                structs_.erase(it);
            }
        }

        auto result = structs_.emplace(struct_type.name(), std::move(struct_type));
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
        map.insert(structs_.begin(), structs_.end());
        for (const auto& pair : inner_)
        {
            pair.second->fill_all_types(map);
        }
    }

    DynamicData constant(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return DynamicData(primitive_type<bool>());
        }

        auto it = module.first->constants_.find(module.second);
        if (it != module.first->constants_.end())
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

        auto it = module.first->constants_.find(module.second);
        if (it != module.first->constants_.end())
        {
            return true;
        }

        return false;
    }

    bool create_constant(
            const std::string& name,
            const DynamicData& value,
            bool replace = false)
    {
        if (name.find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = constants_.find(name);
            if (it != constants_.end())
            {
                constants_.erase(it);
                constants_types_.erase(constants_types_.find(name));
            }
        }

        auto inserted = constants_types_.emplace(name, value.type());
        if (inserted.second)
        {
            DynamicData temp(*(inserted.first->second));
            temp = value;
            auto result = constants_.emplace(name, temp);
            return result.second;
        }
        return false;
    }

    // Generic type retrieval.
    DynamicType::Ptr type(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return DynamicType::Ptr();
        }

        // Check enums
        // TODO

        // Check structs
        if (module.first->has_structure(module.second))
        {
            return module.first->structs_.at(module.second);
        }

        // Check unions
        // TODO

        // Check bitsets
        // TODO

        // Check bitmasks
        // TODO

        return DynamicType::Ptr();
    }

protected:
    std::map<std::string, DynamicType::Ptr> constants_types_;
    std::map<std::string, DynamicData> constants_;
    std::map<std::string, DynamicType::Ptr> structs_;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations_;
    Module* outer_;
    std::map<std::string, std::shared_ptr<Module>> inner_;
    std::string name_;

    Module(
            Module* outer,
            const std::string& name)
        : outer_(outer)
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
                if (outer_ == nullptr) // We are the root, now go down.
                {
                    return resolve_scope(symbol_name.substr(2), original_name);
                }
                else // We are not the root, go up, with the original name.
                {
                    return outer_->resolve_scope(original_name, original_name, true);
                }
            }
            else // not looking for root
            {
                std::string inner_scope = symbol_name.substr(0, symbol_name.find("::"));
                // Maybe the current scope its me?
                if (inner_scope == name_)
                {
                    std::string innest_scope = inner_scope.substr(0, inner_scope.find("::"));
                    if (inner_.count(innest_scope) > 0)
                    {
                        std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                        const auto& it = inner_.find(innest_scope);
                        PairModuleSymbol result = it->second->resolve_scope(inner_name, original_name);
                        if (result.first != nullptr)
                        {
                            return result;
                        }
                    }
                }
                // Do we have a inner scope that matches?
                if (inner_.count(inner_scope) > 0)
                {
                    std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                    const auto& it = inner_.find(inner_scope);
                    return it->second->resolve_scope(inner_name, original_name);
                }
                // Try going back
                if (outer_ != nullptr && first)
                {
                    return outer_->resolve_scope(original_name, original_name, true);
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
