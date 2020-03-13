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

class Module;

namespace idl {
namespace generator {
std::string module_contents(
        Module module,
        size_t tabs);
}
}

namespace internal {
class ModuleImpl;
}

class Module : public std::shared_ptr<internal::ModuleImpl>
{
public:
    using PairModuleSymbol = std::pair<Module, std::string>;
    using Base = std::shared_ptr<internal::ModuleImpl>;

    using Base::operator->;
    using Base::operator*;
    using Base::operator bool;

    Module()
    {
    }

    explicit Module(
            internal::ModuleImpl* ptr)
        : Base(ptr/*,
               [](internal::ModuleImpl* ptr)
               {
                   delete ptr;
               }*/)
    {
    }

    Module(
            const Module&) = default;

    Module(
            Module&&) = default;

    Module& operator =(
            const Module&) = default;

    Module& operator =(
            Module&& other) = default;

    Module& operator =(
            internal::ModuleImpl* ptr)
    {
        return operator =(Module(ptr));
    }

    Module create_submodule(
            const std::string& submodule);

    Module& operator [] (
            const std::string& submodule);

    const Module& operator [] (
            const std::string& submodule) const;

    Module submodule(
            const std::string& submodule);

    bool has_submodule(
            const std::string& submodule) const;

    const std::string& name() const;

    std::string scope() const;

    bool has_symbol(
            const std::string& ident,
            bool extend = true) const;

    bool has_structure(
            const std::string& name) const;

    const StructType& structure(
            const std::string& name) const;

    StructType& structure(
            const std::string& name);

    bool structure(
            StructType& struct_type);

    bool structure(
            StructType&& struct_type,
            bool replace = false);

    bool has_union(
            const std::string& name) const;

    const UnionType& union_switch(
            const std::string& name) const;

    UnionType& union_switch(
            const std::string& name);

    bool union_switch(
            UnionType& union_type);

    bool union_switch(
            UnionType&& union_type,
            bool replace = false);

    // TODO has, get and set of:
    // enums, bitmasks and unions

    std::map<std::string, DynamicType::Ptr> get_all_types(
            bool add_scope = false) const;

    void fill_all_types(
            std::map<std::string, DynamicType::Ptr>& map,
            bool add_scope = false) const;

    DynamicData constant(
            const std::string& name) const;

    bool has_constant(
            const std::string& name) const;

    bool is_const_from_enum(
            const std::string& name) const;

    bool create_constant(
            const std::string& name,
            const DynamicData& value,
            bool replace = false,
            bool from_enumeration = false);

    EnumerationType<uint32_t>& enum_32(
            const std::string& name);

    bool has_enum_32(
            const std::string& name) const;

    const EnumerationType<uint32_t>& enum_32(
            const std::string& name) const;

    bool enum_32(
            EnumerationType<uint32_t>&& enumeration,
            bool replace = false);

    const AliasType& alias(
            const std::string& name) const;

    AliasType& alias(
            const std::string& name);

    bool has_alias(
            const std::string& name) const;

    bool create_alias(
            const DynamicType::Ptr&& type,
            const std::string& name);

    bool add_alias(
            AliasType& alias);

    bool add_alias(
            AliasType&& alias);

    // Generic type retrieval.
    DynamicType::Ptr type(
            const std::string& name);

protected:
    friend internal::ModuleImpl;

    PairModuleSymbol resolve_scope(
            const std::string& symbol_name) const;

    PairModuleSymbol resolve_scope(
            const std::string& symbol_name,
            const std::string& original_name,
            bool first = false) const;

};

namespace internal {
class ModuleImpl
{
protected:
    friend Module;
    using PairModuleSymbol = Module::PairModuleSymbol;

    ModuleImpl()
        : outer_(nullptr)
        , name_("")
    {
    }

    Module submodule(
            const std::string& submodule)
    {
        return inner_[submodule];
    }

    bool has_submodule(
            const std::string& submodule) const
    {
        return inner_.count(submodule) > 0;
    }

    const std::string& name() const
    {
        return name_;
    }

    std::string scope() const
    {
        if (outer_ != nullptr && !outer_.scope().empty())
        {
            return outer_.scope() + "::" + name_;
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
            return outer_.has_symbol(ident, extend);
        }
        return false;
    }

    bool has_structure(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->structs_.count(module.second) > 0;
    }

    const StructType& structure(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for structure '" + name + "'.");

        auto it = module.first->structs_.find(module.second);

        xtypes_assert(it != module.first->structs_.end(), "Cannot find structure '" + name + "'.");
        return static_cast<const StructType&>(*it->second);
    }

    StructType& structure(
            const Module& module,
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for structure '" + name + "'.");

        auto it = module.first->structs_.find(module.second);
        xtypes_assert(it != module.first->structs_.end(), "Cannot find structure '" + name + "'.");
        return static_cast<StructType&>(const_cast<DynamicType&>(*it->second));
    }

    bool structure(
            const Module& module,
            StructType& struct_type)
    {
        if (struct_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        struct_type.attach_to_module(this);
        auto result = structs_.emplace(struct_type.name(), struct_type);
        return result.second;
    }

    bool structure(
            const Module& module,
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

        struct_type.attach_to_module(this);
        auto result = structs_.emplace(struct_type.name(), std::move(struct_type));
        return result.second;
    }

    bool has_union(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }
        return module.first->unions_.count(module.second) > 0;
    }

    const UnionType& union_switch(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for union '" + name + "'.");

        auto it = module.first->unions_.find(module.second);

        xtypes_assert(it != module.first->unions_.end(), "Cannot find union '" + name + "'.");
        return static_cast<const UnionType&>(*it->second);
    }

    UnionType& union_switch(
            const Module& module,
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for union '" + name + "'.");

        auto it = module.first->unions_.find(module.second);

        xtypes_assert(it != module.first->unions_.end(), "Cannot find union '" + name + "'.");
        return static_cast<UnionType&>(const_cast<DynamicType&>(*it->second));
    }

    bool union_switch(
            const Module& module,
            UnionType& union_type)
    {
        if (union_type.name().find("::") != std::string::npos)
        {
            return false; // Cannot add a symbol with scoped name.
        }

        union_type.attach_to_module(this);
        auto result = unions_.emplace(union_type.name(), union_type);
        return result.second;
    }

    bool union_switch(
            const Module& module,
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

        union_type.attach_to_module(this);
        auto result = unions_.emplace(union_type.name(), std::move(union_type));
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
        std::string module_name = scope();
        if (add_scope && !module_name.empty())
        {
            for (const auto& pair : structs_)
            {
                map.emplace(module_name + "::" + pair.first, pair.second);
            }
            for (const auto& pair : unions_)
            {
                map.emplace(module_name + "::" + pair.first, pair.second);
            }
            // TODO Add other types...
        }
        else
        {
            map.insert(structs_.begin(), structs_.end());
            map.insert(unions_.begin(), unions_.end());
            // TODO Add other types...
        }

        for (const auto& pair : inner_)
        {
            pair.second->fill_all_types(map, add_scope);
        }
    }

    DynamicData constant(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
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
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
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
            const Module& module,
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

        auto inserted = constants_types_.emplace(name, value.type());
        if (inserted.second)
        {
            DynamicData temp(*(inserted.first->second));
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
            const Module& module,
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for enumeration '" + name + "'.");

        auto it = module.first->enumerations_32_.find(module.second);
        xtypes_assert(it != module.first->enumerations_32_.end(), "Cannot find enumeration '" + name + "'.");
        return static_cast<EnumerationType<uint32_t>&>(const_cast<DynamicType&>(*it->second));
    }

    bool has_enum_32(
            const std::string& name) const
    {
        return enumerations_32_.count(name) > 0;
    }

    const EnumerationType<uint32_t>& enum_32(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);

        xtypes_assert(module.first != nullptr, "Cannot solve scope for enumeration '" + name + "'.");

        auto it = module.first->enumerations_32_.find(module.second);
        xtypes_assert(it != module.first->enumerations_32_.end(), "Cannot find enumeration '" + name + "'.");
        return static_cast<const EnumerationType<uint32_t>&>(*it->second);
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

        auto result = enumerations_32_.emplace(enumeration.name(), std::move(enumeration));
        return result.second;
    }

    const AliasType& alias(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
        xtypes_assert(module.first != nullptr, "Cannot solve scope for alias '" + name + "'.");

        return static_cast<const AliasType&>(*module.first->aliases_.at(module.second));
    }

    AliasType& alias(
            const Module& module,
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
        xtypes_assert(module.first != nullptr, "Cannot solve scope for alias '" + name + "'.");

        return static_cast<AliasType&>(const_cast<DynamicType&>(*module.first->aliases_.at(module.second)));
    }

    bool has_alias(
            const Module& module,
            const std::string& name) const
    {
        // Solve scope
        PairModuleSymbol module = module.resolve_scope(name);
        if (module.first == nullptr)
        {
            return false;
        }

        auto it = module.first->aliases_.find(module.second);
        return it != module.first->aliases_.end();
    }

    bool create_alias(
            const Module& module,
            const DynamicType::Ptr&& type,
            const std::string& name)
    {
        if (name.find("::") != std::string::npos || has_alias(name))
        {
            return false; // Cannot define alias with scoped name (or already defined).
        }

        AliasType alias(type, name);
        alias.attach_to_module(this);
        return aliases_.emplace(name, alias).second;
    }

    bool add_alias(
            const Module& module,
            AliasType& alias)
    {
        alias.attach_to_module(this);
        return aliases_.emplace(alias.name(), AliasType(alias)).second;
    }

    bool add_alias(
            const Module& module,
            AliasType&& alias)
    {
        alias.attach_to_module(this);
        return aliases_.emplace(alias.name(), std::move(alias)).second;
    }

    // Generic type retrieval.
    DynamicType::Ptr type(
            const Module& module,
            const std::string& name)
    {
        // Solve scope
        PairModuleSymbol pair = module.resolve_scope(name);
        if (pair.first == nullptr)
        {
            return DynamicType::Ptr();
        }

        // Check enums
        if (pair.first->has_enum_32(pair.second))
        {
            return pair.first->enumerations_32_.at(pair.second);
        }

        // Check structs
        if (pair.first.has_structure(pair.second))
        {
            return pair.first->structs_.at(pair.second);
        }

        // Check unions
        if (pair.first.has_union(pair.second))
        {
            return pair.first->unions_.at(pair.second);
        }

        // Check aliases
        if (pair.first.has_alias(pair.second))
        {
            return pair.first->aliases_.at(pair.second);
        }

        // Check bitsets
        // TODO

        // Check bitmasks
        // TODO

        return DynamicType::Ptr();
    }

protected:

    friend std::string idl::generator::module_contents(
            Module module,
            size_t tabs);

    std::map<std::string, DynamicType::Ptr> aliases_;
    std::map<std::string, DynamicType::Ptr> constants_types_;
    std::map<std::string, DynamicData> constants_;
    std::vector<std::string> from_enum_;
    std::map<std::string, DynamicType::Ptr> enumerations_32_;
    std::map<std::string, DynamicType::Ptr> structs_;
    std::map<std::string, DynamicType::Ptr> unions_;
    //std::map<std::string, std::shared_ptr<AnnotationType>> annotations_;
    Module outer_;
    std::map<std::string, Module > inner_;
    std::string name_;

    ModuleImpl(
            Module outer,
            const std::string& name)
        : outer_(outer)
        , name_(name)
    {
    }

};
} // namespace internal

Module Module::create_submodule(
        const std::string& submodule)
{
    Module new_submodule(new internal::ModuleImpl(*this, submodule));
    (Base::get())->inner_.emplace(submodule, new_submodule);
    return new_submodule;
}

Module& Module::operator [] (
        const std::string& submodule)
{
    return (Base::get())->inner_[submodule];
}

const Module& Module::operator [] (
        const std::string& submodule) const
{
    return (Base::get())->inner_.at(submodule);
}

// Auxiliar method to resolve scoped. It will return the ModuleImpl up to the last "::" by calling
// recursively resolving each scoped name, looking for the scope path, and the symbol name without the scope.
// If the path cannot be resolved, it will return nullptr as path, and the full given symbol name.
Module::PairModuleSymbol Module::resolve_scope(
        const std::string& symbol_name) const
{
    return resolve_scope(symbol_name, symbol_name, true);
}

Module::PairModuleSymbol Module::resolve_scope(
        const std::string& symbol_name,
        const std::string& original_name,
        bool first) const
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
            if ((Base::get())->outer_ == nullptr) // We are the root, now go down.
            {
                return resolve_scope(symbol_name.substr(2), original_name);
            }
            else // We are not the root, go up, with the original name.
            {
                return (Base::get())->outer_.resolve_scope(original_name, original_name, true);
            }
        }
        else // not looking for root
        {
            std::string inner_scope = symbol_name.substr(0, symbol_name.find("::"));
            // Maybe the current scope its me?
            if (inner_scope == (Base::get())->name_)
            {
                std::string innest_scope = inner_scope.substr(0, inner_scope.find("::"));
                if ((Base::get())->inner_.count(innest_scope) > 0)
                {
                    std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                    const auto& it = (Base::get())->inner_.find(innest_scope);
                    PairModuleSymbol result = it->second.resolve_scope(inner_name, original_name);
                    if (result.first != nullptr)
                    {
                        return result;
                    }
                }
            }
            // Do we have a inner scope that matches?
            if ((Base::get())->inner_.count(inner_scope) > 0)
            {
                std::string inner_name = symbol_name.substr(symbol_name.find("::") + 2);
                const auto& it = (Base::get())->inner_.find(inner_scope);
                return it->second.resolve_scope(inner_name, original_name);
            }
            // Try going back
            if ((Base::get())->outer_ != nullptr && first)
            {
                return (Base::get())->outer_.resolve_scope(original_name, original_name, true);
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
        return std::make_pair<Module, std::string>(Module(*this), std::move(name));
    }

    if ((Base::get())->outer_ != nullptr)
    {
        return (Base::get())->outer_.resolve_scope(symbol_name, original_name, true);
    }

    // Failed, not found
    PairModuleSymbol pair;
    pair.first = nullptr;
    pair.second = original_name;
    return pair;
}

Module Module::submodule(
        const std::string& submodule)
{
    return (Base::get())->submodule(submodule);
}

bool Module::has_submodule(
        const std::string& submodule) const
{
    return (Base::get())->has_submodule(submodule);
}

const std::string& Module::name() const
{
    return (Base::get())->name();
}

std::string Module::scope() const
{
    return (Base::get())->scope();
}

bool Module::has_symbol(
        const std::string& ident,
        bool extend) const
{
    return (Base::get())->has_symbol(ident, extend);
}

bool Module::has_structure(
        const std::string& name) const
{
    return (Base::get())->has_structure(*this, name);
}

const StructType& Module::structure(
        const std::string& name) const
{
    return (Base::get())->structure(*this, name);
}

StructType& Module::structure(
        const std::string& name)
{
    return (Base::get())->structure(*this, name);
}

bool Module::structure(
        StructType& struct_type)
{
    return (Base::get())->structure(*this, struct_type);
}

bool Module::structure(
        StructType&& struct_type,
        bool replace)
{
    return (Base::get())->structure(*this, std::move(struct_type), replace);
}

bool Module::has_union(
        const std::string& name) const
{
    return (Base::get())->has_union(*this, name);
}

const UnionType& Module::union_switch(
        const std::string& name) const
{
    return (Base::get())->union_switch(*this, name);
}

UnionType& Module::union_switch(
        const std::string& name)
{
    return (Base::get())->union_switch(*this, name);
}

bool Module::union_switch(
        UnionType& union_type)
{
    return (Base::get())->union_switch(*this, union_type);
}

bool Module::union_switch(
        UnionType&& union_type,
        bool replace)
{
    return (Base::get())->union_switch(*this, std::move(union_type), replace);
}

std::map<std::string, DynamicType::Ptr> Module::get_all_types(
        bool add_scope) const
{
    return (Base::get())->get_all_types(add_scope);
}

void Module::fill_all_types(
        std::map<std::string, DynamicType::Ptr>& map,
        bool add_scope) const
{
    return (Base::get())->fill_all_types(map, add_scope);
}

DynamicData Module::constant(
        const std::string& name) const
{
    return (Base::get())->constant(*this, name);
}

bool Module::has_constant(
        const std::string& name) const
{
    return (Base::get())->has_constant(*this, name);
}

bool Module::is_const_from_enum(
        const std::string& name) const
{
    return (Base::get())->is_const_from_enum(name);
}

bool Module::create_constant(
        const std::string& name,
        const DynamicData& value,
        bool replace,
        bool from_enumeration)
{
    return (Base::get())->create_constant(*this, name, value, replace, from_enumeration);
}

EnumerationType<uint32_t>& Module::enum_32(
        const std::string& name)
{
    return (Base::get())->enum_32(*this, name);
}

bool Module::has_enum_32(
        const std::string& name) const
{
    return (Base::get())->has_enum_32(name);
}

const EnumerationType<uint32_t>& Module::enum_32(
        const std::string& name) const
{
    return (Base::get())->enum_32(*this, name);
}

bool Module::enum_32(
        EnumerationType<uint32_t>&& enumeration,
        bool replace)
{
    return (Base::get())->enum_32(std::move(enumeration), replace);
}

const AliasType& Module::alias(
        const std::string& name) const
{
    return (Base::get())->alias(*this, name);
}

AliasType& Module::alias(
        const std::string& name)
{
    return (Base::get())->alias(*this, name);
}

bool Module::has_alias(
        const std::string& name) const
{
    return (Base::get())->has_alias(*this, name);
}

bool Module::create_alias(
        const DynamicType::Ptr&& type,
        const std::string& name)
{
    return (Base::get())->create_alias(*this, std::move(type), name);
}

bool Module::add_alias(
        AliasType& alias)
{
    return (Base::get())->add_alias(*this, alias);
}

bool Module::add_alias(
        AliasType&& alias)
{
    return (Base::get())->add_alias(*this, std::move(alias));
}

DynamicType::Ptr Module::type(
        const std::string& name)
{
    return (Base::get())->type(*this, name);
}

} // xtypes
} // eprosima

#endif // EPROSIMA_XTYPES_MODULE_HPP_
