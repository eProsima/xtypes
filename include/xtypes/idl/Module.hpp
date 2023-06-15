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

#ifndef EPROSIMA_XTYPES_IDL_MODULE_HPP_
#define EPROSIMA_XTYPES_IDL_MODULE_HPP_

#include <xtypes/xtypes.hpp>

#include <xtypes/idl/Type.hpp>

#include <string>
#include <memory>

namespace eprosima {
namespace xtypes {
namespace idl {

class Module;

namespace generator {
namespace dependencytree {
// Forward declarations
class DependencyNode;
class DependencyModule;
}
}

class Module
    : public std::enable_shared_from_this<Module>
{
protected:

    using PairModuleSymbol = std::pair<const Module*, std::string>;

public:

    Module()
        : outer_(nullptr)
        , name_("")
    {
    }

    Module(
            const Module& other) = delete;

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

    size_t submodule_size()
    {
        return inner_.size();
    }

    using ModuleVisitor = std::function<void (const Module& mod)>;

    void for_each_submodule(
            ModuleVisitor visitor,
            const Module* module,
            bool recursive = true) const
    {
        for (const auto& inner : module->inner_)
        {
            visitor(*inner.second.get());
            if (recursive)
            {
                for_each_submodule(visitor, inner.second.get());
            }
        }
    }

    void for_each_submodule(
            ModuleVisitor visitor,
            bool recursive = true) const
    {
        for_each_submodule(visitor, this, recursive);
    }

    void for_each(
            ModuleVisitor visitor) const
    {
        visitor(*this);
        for_each_submodule(visitor, this);
    }

    bool has_submodule(
            const std::string& submodule) const
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

    const std::string& name() const
    {
        return name_;
    }

    std::string scope() const
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
        bool has_it = structs_.count(ident) > 0
                || unions_.count(ident) > 0
                || aliases_.count(ident) > 0
                || constants_.count(ident) > 0
                || enumerations_32_.count(ident) > 0
                || inner_.count(ident) > 0;

        if (has_it)
        {
            return true;
        }
        if (extend && outer_ != nullptr)
        {
            return outer_->has_symbol(ident, extend);
        }
        return false;
    }

    unsigned int symbol_count(
            const std::string& ident) const
    {
        unsigned int count = 0;
        if (has_symbol(ident, false))
        {
            count++;
        }

        if (outer_ != nullptr)
        {
            count += outer_->symbol_count(ident);
        }
        return count;
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

        xtypes_assert(module.first != nullptr, "Cannot solve scope for structure '" + name + "'.");

        auto it = module.first->structs_.find(module.second);

        xtypes_assert(it != module.first->structs_.end(), "Cannot find structure '" + name + "'.");
        return static_cast<const StructType&>(*it->second.get());
    }

    StructType& structure(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for structure '" + name + "'.");

        auto it = module.first->structs_.find(module.second);
        xtypes_assert(it != module.first->structs_.end(), "Cannot find structure '" + name + "'.");
        return static_cast<StructType&>(const_cast<DynamicType&>(*it->second.get()));
    }

    bool structure(
            StructType& struct_type)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        std::string name = struct_type.name();
        std::string name_space = scope();
        struct_type.name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = structs_.emplace(
            name,
            Type(*this, struct_type));
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

        std::string name = struct_type.name();
        std::string name_space = scope();
        struct_type.name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = structs_.emplace(
            name,
            Type(*this, std::move(struct_type)));
        return result.second;
    }

    bool has_union(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->unions_.count(module.second) > 0;
    }

    const UnionType& union_switch(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for union '" + name + "'.");

        auto it = module.first->unions_.find(module.second);

        xtypes_assert(it != module.first->unions_.end(), "Cannot find union '" + name + "'.");
        return static_cast<const UnionType&>(*it->second.get());
    }

    UnionType& union_switch(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for union '" + name + "'.");

        auto it = module.first->unions_.find(module.second);

        xtypes_assert(it != module.first->unions_.end(), "Cannot find union '" + name + "'.");
        return static_cast<UnionType&>(const_cast<DynamicType&>(*it->second.get()));
    }

    bool union_switch(
            UnionType& union_type)
    {
        if (union_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        std::string name = union_type.name();
        std::string name_space = scope();
        union_type.name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = unions_.emplace(
            name,
            Type(*this, union_type));
        return result.second;
    }

    bool union_switch(
            UnionType&& union_type,
            bool replace = false)
    {
        if (union_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = unions_.find(union_type.name());
            if (it != unions_.end())
            {
                unions_.erase(it);
            }
        }

        std::string name = union_type.name();
        std::string name_space = scope();
        union_type.name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = unions_.emplace(
            name,
            Type(*this, std::move(union_type)));
        return result.second;
    }

    // TODO has, get and set of:
    // enums, bitmasks and unions

    std::map<std::string, DynamicType::Ptr> get_all_types(
            bool add_scope = false) const
    {
        std::map<std::string, DynamicType::Ptr> result;
        fill_all_types(result, add_scope);
        return result;
    }

    void fill_all_types(
            std::map<std::string, DynamicType::Ptr>& map,
            bool add_scope = false) const
    {
        std::string module_name = add_scope ? scope() : "";
        if (!module_name.empty())
        {
            module_name += "::";
        }

        for (const auto& pair : structs_)
        {
            map.emplace(module_name + pair.first, pair.second.get());
        }
        for (const auto& pair : unions_)
        {
            map.emplace(module_name + pair.first, pair.second.get());
        }
        for (const auto& pair : aliases_)
        {
            map.emplace(module_name + pair.first, pair.second.get());
        }
        for (const auto& pair : enumerations_32_)
        {
            map.emplace(module_name + pair.first, pair.second.get());
        }
        // TODO Add other types...

        for (const auto& pair : inner_)
        {
            pair.second->fill_all_types(map, add_scope);
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

    bool is_const_from_enum(
            const std::string& name) const
    {
        return std::find(from_enum_.begin(), from_enum_.end(), name) != from_enum_.end();
    }

    bool create_constant(
            const std::string& name,
            const DynamicData& value,
            bool replace = false,
            bool from_enumeration = false)
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

        auto inserted = constants_types_.emplace(name, Type(*this, value.type()));
        if (inserted.second)
        {
            DynamicData temp(*inserted.first->second.get());
            temp = value;
            auto result = constants_.emplace(name, temp);
            if (result.second && from_enumeration)
            {
                from_enum_.push_back(name);
            }
            return result.second;
        }
        return false;
    }

    EnumerationType<uint32_t>& enum_32(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for enumeration '" + name + "'.");

        auto it = module.first->enumerations_32_.find(module.second);
        xtypes_assert(it != module.first->enumerations_32_.end(), "Cannot find enumeration '" + name + "'.");
        return static_cast<EnumerationType<uint32_t>&>(const_cast<DynamicType&>(*it->second.get()));
    }

    bool has_enum_32(
            const std::string& name) const
    {
        return enumerations_32_.count(name) > 0;
    }

    const EnumerationType<uint32_t>& enum_32(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for enumeration '" + name + "'.");

        auto it = module.first->enumerations_32_.find(module.second);
        xtypes_assert(it != module.first->enumerations_32_.end(), "Cannot find enumeration '" + name + "'.");
        return static_cast<const EnumerationType<uint32_t>&>(*it->second.get());
    }

    bool enum_32(
            EnumerationType<uint32_t>&& enumeration,
            bool replace = false)
    {
        if (enumeration.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        if (replace)
        {
            auto it = enumerations_32_.find(enumeration.name());
            if (it != enumerations_32_.end())
            {
                enumerations_32_.erase(it);
            }
        }

        std::string name = enumeration.name();
        std::string name_space = scope();
        enumeration.name(name_space + (name_space.empty() ? "" : "::") + name);
        auto result = enumerations_32_.emplace(name, Type(*this, std::move(enumeration)));
        return result.second;
    }

    const AliasType& alias(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        xtypes_assert(module.first != nullptr, "Cannot solve scope for alias '" + name + "'.");

        return static_cast<const AliasType&>(*module.first->aliases_.at(module.second).get());
    }

    AliasType& alias(
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        xtypes_assert(module.first != nullptr, "Cannot solve scope for alias '" + name + "'.");

        return static_cast<AliasType&>(const_cast<DynamicType&>(*module.first->aliases_.at(module.second).get()));
    }

    bool has_alias(
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }

        auto it = module.first->aliases_.find(module.second);
        return it != module.first->aliases_.end();
    }

    bool create_alias(
            const DynamicType::Ptr&& type,
            const std::string& name)
    {
        if (name.find("::") != std::string::npos || has_alias(name))
        {
            return false; // Cannot define alias with scoped name (or already defined).
        }

        std::string name_space = scope();
        AliasType alias(type, name_space + (name_space.empty() ? "" : "::") + name);
        return aliases_.emplace(name, Type(*this, alias)).second;
    }

    bool add_alias(
            AliasType& alias)
    {
        std::string name = alias.name();
        std::string name_space = scope();
        alias.name(name_space + (name_space.empty() ? "" : "::") + name);
        return aliases_.emplace(name, Type(*this, AliasType(alias))).second;
    }

    bool add_alias(
            AliasType&& alias)
    {
        std::string name = alias.name();
        std::string name_space = scope();
        alias.name(name_space + (name_space.empty() ? "" : "::") + name);
        return aliases_.emplace(name, Type(*this, std::move(alias))).second;
    }

    // Generic type retrieval.
    DynamicType::Ptr type(
            const std::string& name,
            bool recursive = false)
    {
        DynamicType::Ptr ret_type;

        if (recursive)
        {
            for (auto [key, mod]: inner_)
            {
                ret_type = mod.get()->type(name, recursive);
            }
        }

        // Solve scope
        PairModuleSymbol module = resolve_scope(name);
        if (module.first == nullptr)
        {
            return ret_type;
        }

        // Check enums
        if (module.first->has_enum_32(module.second))
        {
            ret_type = module.first->enumerations_32_.at(module.second).get();
        }

        // Check structs
        if (module.first->has_structure(module.second))
        {
            ret_type = module.first->structs_.at(module.second).get();
        }

        // Check unions
        if (module.first->has_union(module.second))
        {
            ret_type = module.first->unions_.at(module.second).get();
        }

        // Check aliases
        if (module.first->has_alias(module.second))
        {
            ret_type = module.first->aliases_.at(module.second).get();
        }

        if (name.find("::") == 0)
        {
            // Scope ambiguity solver was originally used, add it to the retrieved DynamicType
            ret_type->name(name);
        }

        // Check bitsets
        // TODO

        // Check bitmasks
        // TODO

        return ret_type;
    }

protected:

    friend class generator::dependencytree::DependencyNode;
    friend class generator::dependencytree::DependencyModule;

    std::map<std::string, Type> aliases_;
    std::map<std::string, Type> constants_types_;
    std::map<std::string, DynamicData> constants_;
    std::vector<std::string> from_enum_;
    std::map<std::string, Type> enumerations_32_;
    std::map<std::string, Type> structs_;
    std::map<std::string, Type> unions_;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations_;
    Module* outer_;
    std::map<std::string, std::shared_ptr<Module> > inner_;
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

        if (has_symbol(name, false))
        {
            return std::make_pair<const Module*, std::string>(this, std::move(name));
        }

        if (outer_ != nullptr)
        {
            return outer_->resolve_scope(symbol_name, original_name, true);
        }

        // Failed, not found
        PairModuleSymbol pair;
        pair.first = nullptr;
        pair.second = original_name;
        return pair;
    }

};

} // namespace idl
} // namespace xtypes
} // namespace eprosima

#endif // EPROSIMA_XTYPES_IDL_MODULE_HPP_
