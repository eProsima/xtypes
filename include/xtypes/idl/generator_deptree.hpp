/*
 * Copyright 2020, Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef EPROSIMA_XTYPES_IDL_GENERATOR_DEPENDENCYTREE_HPP_
#define EPROSIMA_XTYPES_IDL_GENERATOR_DEPENDENCYTREE_HPP_

#include <xtypes/idl/Module.hpp>
#include <xtypes/idl/generator.hpp>

namespace eprosima {

namespace xtypes {

namespace idl {

namespace generator {

namespace dependencytree {
// Forward declarations
class DependencyNode;
class DependencyModule;
};

// Forward declarations from "generator.hpp". Allows user to just include
// "idl/generator.hpp" and forget about this file.
inline std::string type_name(
        dependencytree::DependencyNode* node,
        const DynamicType& type,
        bool scoped = false);

inline std::string aliase(
        const std::string& name,
        const DynamicType& type,
        dependencytree::DependencyNode* alias_node);

inline std::string get_const_value(
        ReadableDynamicDataRef data);

inline std::string enumeration32(
        const std::string& name,
        const EnumerationType<uint32_t>& enumeration,
        size_t tabs = 0);

inline std::string structure(
        const std::string& name,
        const StructType& type,
        dependencytree::DependencyNode* struct_node = nullptr,
        size_t tabs = 0,
        std::map<std::string, std::string>* struct_idl = nullptr);

inline std::string generate_union(
        const std::string& name,
        const UnionType& type,
        dependencytree::DependencyNode* union_node,
        size_t tabs = 0);

namespace dependencytree {

/// \brief Alias given to some Module's content element (alias, struct, constant...).
/// std::pair is used, as Module's contents are stored using std::map containers.
using ModuleElement = std::pair<const std::string, Type>;
using ModuleRefSet = std::vector<std::weak_ptr<DependencyModule>>;

/// \brief Enumeration that describes the kind of a DependencyNode.
/// This enumeration reflects all possible kinds for a Module's content element.
enum class ModuleElementKind
{
    xALIAS,
    xCONST,
    xENUM,
    xSTRUCT,
    xUNION/*,
    xSTRUCT_FW,
    xUNION_FW*/
};

/// \brief A class to set a hierarchy between Module's contents.
/// It stores a list of references to it's hierarchically greater contents
/// (e.g. those who need to be declared before this one), as well as a pointer
/// to the DependencyModule needing this DependencyNode's existence for it
/// to be written into an IDL file (if any).
class DependencyNode
{
public:

    using NodeRefSet = std::vector<std::reference_wrapper<DependencyNode>>;

    /// \brief Construct a DependencyNode
    /// \param[in] from The DependencyModule this object belongs to.
    /// \param[in] node A key, value pair representing some Module's content element
    /// \param[in] kind Module's content element kind (rvalue)
    DependencyNode(
            DependencyModule* from,
            const ModuleElement& node,
            ModuleElementKind&& kind)
        : from_(from)
        , node_(node)
        , kind_(std::move(kind))
        , iterated_(false)
    {}

    /// \brief Check if this DependencyNode has been iterated.
    /// "Iterated" means that its IDL representation has already been generated.
    /// \returns A boolean value indicating if object has been iterated or not.
    bool iterated() const
    {
        return iterated_;
    }

    /// \brief Gets the DependencyModule which contains this DependencyNode.
    /// \returns A const pointer to the DependencyModule this object comes from.
    const DependencyModule* from() const
    {
        return from_;
    }

    /// \brief Gets the DependencyModule which contains this DependencyNode.
    /// \returns A non-const pointer to the DependencyModule this object comes from.
    DependencyModule* from()
    {
        return from_;
    }

    /// \brief Name of the DependencyNode.
    /// \returns Key of the ModuleElement pair.
    const std::string& name() const
    {
        return node_.first;
    }

    /// \brief Type of the DependencyNode.
    /// \returns Pointed type by the ModuleElement pair value.
    const DynamicType& type() const
    {
        return *node_.second;
    }

    /// \brief Kind of the DependencyNode.
    /// \returns The assigned ModuleElementKind enumeration value.
    const ModuleElementKind& kind() const
    {
        return kind_;
    }

    /// \brief Module containing this DependencyNode.
    /// \returns A const reference to the Module.
    const Module& module() const
    {
        return node_.second.parent();
    }

    /// \brief Ask the DependencyNode if it does have any ancestor.
    /// An ancestor would be another DependencyNode to be declared prior
    /// to this one during the IDL generation.
    /// \returns A logic value indicating if this DependencyNode has ancestors.
    bool has_ancestors() const
    {
        return !ancestors_.empty();
    }

    /// \brief Get some DependencyNode's ancestors.
    /// \returns A reference this node's ancestors container.
    const NodeRefSet& ancestors() const
    {
        return ancestors_;
    }

    /// \brief Set a new ancestor for this DependencyNode.
    /// \pre New ancestor is not added if it already exists.
    /// \param[in] ancestor New ancestor to be set.
    void set_ancestor(DependencyNode& ancestor)
    {
        for (const auto& ancestor_ : ancestors_)
        {
            if (ancestor_.get() == ancestor)
            {
                return;
            }
        }
        ancestors_.push_back(ancestor);
    }

    /// \brief Ask this DependencyNode if any module depends on its existence.
    /// \returns A logic value indicating if a child DependencyModule has been set.
    bool has_child_modules() const
    {
        return !child_modules_.empty();
    }

    /// \brief Ask this DependencyNode if its child module dependency is an specific one.
    /// \param[in] child A pointer to the DependencyModule to be compared with the inner ones.
    /// \returns The boolean result of the comparison.
    bool has_child_module(
            const DependencyModule* child) const
    {
        for (const auto& child_ : child_modules_)
        {
            if (child_.lock().get() == child)
            {
                return true;
            }
        }

        return false;
    }

    /// \brief Get this node's child DependencyModule.
    /// \returns A const reference to the child module set.
    const ModuleRefSet& child_modules() const
    {
        return child_modules_;
    }

    /// \brief Set the child module pointer to an specific value.
    /// \param[in] child_module DependencyModule to be set.
    void set_child_module(
            const std::weak_ptr<DependencyModule>&& child_module)
    {
        for (const auto& child_ : child_modules_)
        {
            if (child_.lock().get() == child_module.lock().get())
            {
                return;
            }
        }
        child_modules_.emplace_back(std::move(child_module));
    }

    /// \brief Ask this DependencyNode if any module is required for it to exist.
    /// \returns A logic value indicating if a parent DependencyModule has been set.
    bool has_parent_modules() const
    {
        return !parent_modules_.empty();
    }

    /// \brief Get this node's parents DependencyModule.
    /// \returns A const reference to the parent module set.
    const ModuleRefSet& parent_modules() const
    {
        return parent_modules_;
    }

    /// \brief Get this node's parents DependencyModule.
    /// \returns A non-const reference to the parent module set.
    ModuleRefSet& parent_modules()
    {
        return parent_modules_;
    }

    /// \brief Set a new parent module, if it did not exist.
    /// \param[in] parent_module DependencyModule to be set.
    void set_parent_module(
            const std::weak_ptr<DependencyModule>&& parent_module)
    {
        for (const auto& parent_ : parent_modules_)
        {
            if (parent_.lock().get() == parent_module.lock().get())
            {
                return;
            }
        }

        parent_modules_.emplace_back(std::move(parent_module));
    }

    /// \brief Generates the corresponding IDL sentence for this node
    /// \param[in] tabs Padding relative to module's scope.
    /// \return the generated sentence for this DependencyNode object.
    std::string generate_idl_sentence(
            unsigned int tabs,
            std::map<std::string, std::string>* type_idl = nullptr)
    {
        using namespace generator;

        if (iterated_)
        {
            return std::string();
        }

        std::stringstream ss;
        Module& module_ = node_.second.parent();

        switch(kind_)
        {
            case ModuleElementKind::xALIAS:
            {
                ss << std::string(4 * tabs, ' ') << aliase(name(), static_cast<const AliasType&>(type()).get(), this);
                break;
            }
            case ModuleElementKind::xCONST:
            {
                for (const auto& pair : module_.constants_)
                {
                    if (pair.first == name() && !module_.is_const_from_enum(name()))
                        // Don't add as const the "fake" enumeration consts.
                    {
                        ss << std::string(4 * tabs, ' ') << "const " << type_name(this, pair.second.type())
                           << " " << pair.first  << " = " << get_const_value(pair.second) << ";" << std::endl;
                        break;
                    }
                }
                break;
            }
            case ModuleElementKind::xENUM:
            {
                ss << enumeration32(name(), static_cast<const EnumerationType<uint32_t>&>(type()), tabs);
                break;
            }
            case ModuleElementKind::xSTRUCT:
            {
                ss << structure(name(), static_cast<const StructType&>(type()), this, tabs, type_idl);
                break;
            }
            case ModuleElementKind::xUNION:
            {
                ss << generate_union(name(), static_cast<const UnionType&>(type()), this, tabs);
                break;
            }
            /*
            case ModuleElementKind::STRUCT_FW:
            case ModuleElementKind::UNION_FW:
            */
        }

        iterated_ = true;

        return ss.str();
    }

    /// \brief Equal comparison operator overload between two DependencyNode objects.
    /// \param[in] other The DependencyNode object to be compared with.
    /// \returns A logic value indicating whether the two DependencyNode objects are equal or not.
    inline bool operator == (
            const DependencyNode& other) const
    {
        return (kind_ == other.kind_ &&
                node_.first == node_.first &&
                node_.second.get() == other.node_.second.get() &&
                node_.second.parent().scope() == node_.second.parent().scope());
    }

    /// \brief Not equal comparison operator overload between two DependencyNode objects.
    /// \param[in] other The DependencyNode object to be compared with.
    /// \returns A logic value indication whether the two DependencyNode objects are different or not.
    inline bool operator != (
            const DependencyNode& other) const
    {
        return !operator==(other);
    }

private:

    DependencyModule* from_;
    const ModuleElement& node_;
    ModuleElementKind kind_;
    bool iterated_;
    NodeRefSet ancestors_;
    ModuleRefSet parent_modules_;
    ModuleRefSet child_modules_;
};

/// \brief Class to set and store hierarchical relationships between Module objects.
class DependencyModule
{
public:

    using ModuleSet = std::vector<std::shared_ptr<DependencyModule>>;
    using NodeSet = std::vector<DependencyNode>;

    /// \brief Construct a new DependencyModule object.
    /// \param[in] d_root Reference to root DependencyModule in this tree.
    /// \param[in] module The Module object from which this objects keeps track of.
    /// \param[in] outer A pointer to its outer DependencyModule. Here, "outer" does not
    /// mean a hierarchically precedent DependencyModule to be printed before this in the
    /// generated IDL file, but is a pointer to this Module's outer scoped Module object;
    /// e.g. "this" is a submodule of "outer" (same as with Module class outer_ pointer)
    DependencyModule(
            const std::shared_ptr<DependencyModule>& d_root,
            const Module& module,
            DependencyModule* outer)
        : iterated_(false)
        , module_(module)
        , d_root_(d_root)
        , d_outer_(outer)
    {}

    /// \brief Check out if this DependencyModule has already been iterated.
    /// "Iterated" means that its IDL has been already generated (avoid duplicity).
    /// \returns A boolean value indicating if its IDL has been generated or not.
    bool iterated() const
    {
        return iterated_;
    }

    /// \brief Change a DependencyModule's iterated property.
    /// \param[in] Boolean value to set this DependencyModule as iterated (or not).
    void set_iterated(bool iterated)
    {
        iterated_ = iterated;
    }

    /// \brief Retrieve Module object referred by this DependencyModule.
    /// \returns A const reference to the Module.
    const Module& module() const
    {
        return module_;
    }

    /// \brief Get this DependencyModule's DependencyNode set.
    /// \returns A const reference to the NodeSet.
    const NodeSet& node_set() const
    {
        return node_set_;
    }

    /// \brief Get this DependencyModule's DependencyNode set.
    /// \returns A non-const reference to the NodeSet.
    NodeSet& node_set()
    {
        return node_set_;
    }

    /// \brief Check if all DependencyNode objects from NodeSet have been already iterated.
    /// \returns Boolean value with the result of the request.
    bool all_nodes_iterated() const
    {
        for (const auto& node : node_set_)
        {
            if (!node.iterated())
            {
                return false;
            }
        }

        return true;
    }

    /// \brief Check if all DependencyModule inner modules have been already iterated.
    /// \returns Boolean value with the result of the request.
    bool all_inner_iterated() const
    {
        for (const auto& inner : d_inner_)
        {
            if (!inner->iterated())
            {
                return false;
            }
        }

        return true;
    }

    /// \brief Check if a DependencyModule has an outer DependencyModule set.
    /// \returns A boolean value with the requested information.
    inline bool has_outer() const
    {
        return d_outer_ != nullptr;
    }

    /// \brief Get this DependencyModule's outer DependencyModule pointer.
    /// \returns A const pointer to the outer DependencyModule.
    const DependencyModule* outer() const
    {
        return d_outer_;
    }

    /// \brief Get this DependencyModule's outer DependencyModule pointer.
    /// \returns A non-const pointer to the outer DependencyModule.
    DependencyModule* outer()
    {
        return d_outer_;
    }

    /// \brief Check if this DependencyModule, or any one above its module's branch in the module tree,
    /// matches with the one provided.
    /// \param[in] outer The DependencyModule whose existance in outer scopes wants to be checked.
    /// \returns Boolean value indicating if any outer DependencyModule in this branch matches the provided one.
    bool has_outer(
            const DependencyModule* outer) const
    {
        if (has_outer())
        {
            if (d_outer_ == outer)
            {
                return true;
            }
            return d_outer_->has_outer(outer);
        }

        return false;
    }

    /// \brief Get a pointer to the outermost scoped DependencyModule in the module tree.
    /// returns The outermost DependencyModule in the tree.
    inline DependencyModule* outer_root()
    {
        return d_root_.get();
    }

    using DepModulesPair = std::pair<DependencyModule*, DependencyModule*>;

    /// \brief Find common outer DependencyModule objects siblings between this one and another.
    /// "Siblings" means that they have a common outer module. For example:
    /// "root" module containing "a" and "b" as submodules; "c" is also a submodule of "a". Common outer
    /// siblings of "c" and "b" are <a, b>.
    /// \pre This DependencyModule object must not be root.
    /// \param[in] dep DependencyModule to find a common outer with.
    /// \returns A pair of sibling DependencyModules.
    DepModulesPair find_outer_siblings(
            const DependencyModule* dep)
    {
        xtypes_assert(has_outer(), "Cannot use 'find_outer_siblings()' in root node.");

        DependencyModule* inner_dep = d_outer_->has_inner(dep);

        if (inner_dep != nullptr)
        {
            return DepModulesPair(this, inner_dep);
        }
        else
        {
            return d_outer_->find_outer_siblings(dep);
        }
    }

    /// \brief Return immediate inner DependencyModules in the tree.
    /// \returns A reference to this DependencyModule's inner vector.
    const ModuleSet& inner() const
    {
        return d_inner_;
    }

    /// \brief Check if this DependencyModule has a certain module as inner.
    /// If recursive flag is set, it will look through its whole subtree.
    /// \param[in] module DependencyModule to search for.
    /// \param[in] recurse Go deep through the whole level (default), or just one level
    /// \returns Const pointer to inner module which matches (or contains) the one provided as parameter.
    const DependencyModule* has_inner(
            const DependencyModule* module,
            bool recurse=true) const
    {
        for (const auto& inner : d_inner_)
        {
            if (inner.get() == module || (recurse && inner->has_inner(module)))
            {
                return inner.get();
            }
        }

        return nullptr;
    }

    /// \brief Check if this DependencyModule has a certain module as inner.
    /// If recursive flag is set, it will look through its whole subtree.
    /// \param[in] module DependencyModule to search for.
    /// \param[in] recurse Go deep through the whole level (default), or just one level
    /// \returns Non-const pointer to inner module which matches (or contains) the one provided as parameter.
    DependencyModule* has_inner(
            const DependencyModule* module,
            bool recurse=true)
    {
        for (auto& inner : d_inner_)
        {
            if (inner.get() == module || (recurse && inner->has_inner(module)))
            {
                return inner.get();
            }
        }

        return nullptr;
    }

    /// \brief Add an inner DependencyModule to the inner list.
    /// \param[in] inner Reference to the new inner DependencyModule.
    void set_inner(
            const std::shared_ptr<DependencyModule>&& inner)
    {
        d_inner_.emplace_back(std::move(inner));
    }

    /// \brief Check if this DependencyModule has any ancestors.
    /// returns Boolean result of the request.
    bool has_ancestors() const
    {
        return !ancestors_.empty();
    }

    /// \brief Get a list of ancestors for this DependencyModule.
    /// \returns A const reference to the ancestors container.
    const ModuleRefSet& ancestors() const
    {
        return ancestors_;
    }

    /// \brief Add a new ancestor for this DependencyModule. The ancestor
    /// will be added only if it didn't exist before.
    /// param[in] ancestor New DependencyModule to be set as ancestor.
    void set_ancestor(
            DependencyModule* ancestor)
    {
        for (const auto& ancestor_ : ancestors_)
        {
            if (ancestor_.lock().get() == ancestor)
            {
                return;
            }
        }

        std::weak_ptr<DependencyModule> new_ancestor = search_module_in_tree(ancestor);
        ancestors_.emplace_back(std::move(new_ancestor));
    }

    /// \brief Set a hierarchical ancestor. This implies:
    /// - Not adding as ancestor a DependencyModule which is an outer of this one.
    /// - Searching for common outer siblings (read 'find_outer_siblings' doc).
    /// \param[in] ancestor The DependencyModule to be set as ancestor.
    void set_hierarchical_ancestor(
            const DependencyModule* ancestor)
    {
        if (!has_outer(ancestor))
        {
            DepModulesPair mpair = find_outer_siblings(ancestor);
            mpair.first->set_ancestor(mpair.second);
        }
    }

    #define ADD_INTO_DEPENDENCY_SET(SET, KIND) \
    {\
        for (const auto& node : SET)\
        {\
            node_set_.emplace_back(DependencyNode(this, node, ModuleElementKind::KIND));\
        }\
    }

    /// \brief Create a DependencyNode set for this DependencyModule.
    inline void create_dependency_set()
    {
        ADD_INTO_DEPENDENCY_SET(module_.aliases_, xALIAS);
        ADD_INTO_DEPENDENCY_SET(module_.constants_types_, xCONST);
        ADD_INTO_DEPENDENCY_SET(module_.enumerations_32_, xENUM);
        ADD_INTO_DEPENDENCY_SET(module_.structs_, xSTRUCT);
        ADD_INTO_DEPENDENCY_SET(module_.unions_, xUNION);
    }

    /// \brief Looks for an specific module in the shared_ptr tree, given its pointer.
    /// \param[in] module DependencyModule to look for.
    /// \param[in] from_root Trigger first call from root node.
    /// \returns A weak_ptr to the DependencyModule.
    std::weak_ptr<DependencyModule> search_module_in_tree(
            DependencyModule* module,
            bool from_root=true)
    {
        if (from_root)
        {
            if (d_root_.get() == module)
            {
                return std::weak_ptr<DependencyModule>(d_root_);
            }
            else
            {
                return d_root_->search_module_in_tree(module, false);
            }
        }
        else
        {
            for (const auto& inner : d_inner_)
            {
                if (inner.get() == module)
                {
                    return std::weak_ptr<DependencyModule>(inner);
                }

                std::weak_ptr<DependencyModule> inner_inner =
                    inner->search_module_in_tree(module, false);

                if (inner_inner.lock().get() != nullptr)
                {
                    return inner_inner;
                }
            }

            return std::weak_ptr<DependencyModule>(std::shared_ptr<DependencyModule>(nullptr));
        }
    }

    /// \brief Given a DependencyNode's name, search for it through the DependencyModule tree.
    /// \param[in] scoped_name Name of the DependencyNode to be found.
    /// \param[in] from_search Reference to DependencyModule that triggered the search. It will
    /// be set as a child DependencyModule of the found DependencyNode, if it is not nullptr.
    /// \param[in] from_root Start search from DependencyModule's tree root.
    /// \returns A pointer to the found DependencyModule.
    DependencyModule* search_module_with_node(
            const std::string& scoped_name,
            DependencyModule* from_search=nullptr,
            bool from_root=true)
    {
        if (from_root)
        {
            DependencyModule* res =
                outer_root()->search_module_with_node(scoped_name, from_search, false);
            xtypes_assert(res != nullptr,
                "Could not find module containing dependency named '" << scoped_name << "'.");
            return res;
        }
        else
        {
            bool scope_match = true;
            std::string unscoped_name;
            std::string scope;
            // Separe unscoped name and scope
            size_t pos = scoped_name.rfind("::");
            if (pos != std::string::npos)
            {
                unscoped_name = scoped_name.substr(pos + 2);
                scope = scoped_name.substr(0, pos);
                if (scope != ((scope.find("::") == 0) ? std::string("::") + module_.scope()
                                                      : module_.scope()))
                {
                    scope_match = false;
                }
            }
            else
            {
                unscoped_name = scoped_name;
            }

            for (auto& node : node_set_)
            {
                if (node.name() == unscoped_name && scope_match)
                {
                    if (from_search != nullptr)
                    {
                        DependencyModule* new_child = from_search;
                        if (!node.from()->has_inner(from_search, false))
                        {
                            for (const auto& inner : node.from()->inner())
                            {
                                if (inner->has_inner(from_search))
                                {
                                    new_child = inner.get();
                                    break;
                                }
                            }
                        }
                        node.set_child_module(search_module_in_tree(new_child));
                    }

                    return this;
                }
            }

            for (const auto& inner : d_inner_)
            {
                DependencyModule* found;
                found = inner->search_module_with_node(scoped_name, from_search, false);

                if (found != nullptr)
                {
                    return found;
                }
            }
        }

        return nullptr;
    }

    /// \brief Set a dependency between two DependencyNodes.
    /// \param[in] dependent The DependencyNode objects which depends on master.
    /// \param[in] master Name of the DependencyNode to be found in the tree.
    /// \param[in] dep_list A ModuleElement map to search within this DependencyModule.
    /// before triggering search in the whole tree.
    void set_dependency(
            DependencyNode& dependent,
            const std::string& master,
            const ModuleElementKind& kind)
    {
        if (dependent.name() == master) // Cannot add dependency to itself
        {
            return;
        }

        bool scoped_this = true;
        std::string unscoped_master, master_scope;
        if (master.find("::") != std::string::npos)
        {
            size_t scope_end = master.rfind("::");
            unscoped_master = master.substr(scope_end + 2);
            master_scope = master.substr(0, scope_end);

            if (master_scope != ((master_scope.find("::") == 0) ? std::string("::") + module_.scope()
                                                                : module_.scope()))
            {
                scoped_this = false;
            }
        }
        else
        {
            unscoped_master = master;
        }

        if (module_.has_symbol(unscoped_master, false) && scoped_this)
        {
            for (auto& node : node_set_)
            {
                if (node.kind() != kind)
                {
                    continue;
                }
                else if (node.name() == unscoped_master)
                {
                    dependent.set_ancestor(node);
                    break;
                }
            }
        }
        else
        {
            DependencyModule* dep_mod = search_module_with_node(master, this);

            // Check if found module is a submodule of this one
            DependencyModule* is_submod = has_inner(dep_mod);

            if (is_submod != nullptr)
            {
                dependent.set_parent_module(search_module_in_tree(is_submod));
            }
            else
            {
                set_hierarchical_ancestor(dep_mod);
            }
        }
    }

    /// \brief Checks if a DynamicType is adequate for dependency setting.
    /// \param[in] type The DynamicType to be checked.
    /// \returns Boolean with the requested information.
    inline bool opts_for_dependency_setting(
            const DynamicType& type) const
    {
        if (!type.is_constructed_type())
        {
            return false;
        }

        const TypeKind& kind = type.kind();

        return (kind == TypeKind::ALIAS_TYPE
            ||  kind == TypeKind::ENUMERATION_TYPE
            ||  kind == TypeKind::STRUCTURE_TYPE
            ||  kind == TypeKind::UNION_TYPE);
    }

    /// \brief Wrapper method to set dependency based on DynamicType object.
    /// If the DynamicType does not opt for dependency setting (i.e. is constructed),
    /// no dependency is set.
    /// \param[in] node The dependent node to be set a dependency ancestor.
    /// \param[in] type The DynamicType of the master DependencyNode.
    /// \param[in] master Name of the master DependencyNode to be set as ancestor.
    inline void set_dynamic_type_dependency(
            DependencyNode& node,
            const DynamicType& type,
            const std::string& master)
    {
        if (opts_for_dependency_setting(type))
        {
            set_dependency(node, master, node.kind());
        }
    }

    /// \brief Inspects a DependencyNode object and sets dependencies according to its kind.
    /// \param[in] node The DependencyNode whose dependencies wants to be solved.
    void set_node_dependencies(DependencyNode& node)
    {
        switch (node.kind())
        {
            case ModuleElementKind::xALIAS:
            {
                const AliasType& alias = static_cast<const AliasType&>(node.type());
                set_dynamic_type_dependency(node, *alias, alias->name());
                break;
            }

            case ModuleElementKind::xCONST:
            {
                const DynamicType& const_type = node.type();
                set_dynamic_type_dependency(node, const_type, const_type.name());
                break;
            }

            case ModuleElementKind::xENUM:
            {
                const EnumerationType<uint32_t>& enum_type = static_cast<const EnumerationType<uint32_t>&>(node.type());
                set_dynamic_type_dependency(node, enum_type, enum_type.name());
                break;

            }
            case ModuleElementKind::xSTRUCT:
            {
                // TODO: detect if we are setting cyclical struct dependencies, and generate a forward declaration.
                const StructType& structure = static_cast<const StructType&>(node.type());

                if (structure.has_parent())
                {
                    set_dynamic_type_dependency(node, structure.parent(), structure.parent().name());
                }

                structure.for_each([&](const DynamicType::TypeNode& tnode)
                {
                    const DynamicType& type = tnode.type();
                    if (type.name() != structure.name())
                    {
                        set_dynamic_type_dependency(node, type, type.name());
                    }
                });
                break;
            }
            case ModuleElementKind::xUNION:
            {
                // TODO: detect if we are setting cyclical union dependencies, and generate a forward declaration.
                const UnionType& union_type = static_cast<const UnionType&>(node.type());
                union_type.for_each([&](const DynamicType::TypeNode& tnode)
                {
                    const DynamicType& type = tnode.type();
                    if (type.name() != union_type.name())
                    {
                        set_dynamic_type_dependency(node, type, type.name());
                    }
                });
                break;
            }
            /*
            case ModuleElementKind::xSTRUCT_FW:
            case ModuleElementKind::xUNION_FW:
            */
            default:
            {
                xtypes_assert(false, "Type '" << node.name() << "' does not opt for dependency setting.");
            }
        }
    }

    /// \brief Helper method to solve dependencies for all DependencyNodes in a DependencyModule.
    void solve_dependency_tree()
    {
        for (auto& node : node_set_)
        {
            set_node_dependencies(node);
        }
    }

    /// \brief Resolve relative scope between this module and another one, provided as parameter.
    /// \param[in] other The DependencyModule whose scope wants to be resolved against this one.
    /// \returns Relative scope between the two modules.
    std::string relative_scope(
            const DependencyModule* other)
    {
        if (other == this)
        {
            return std::string();
        }

        std::stringstream ss;

        if (has_outer(other))
        {
            ss << other->module().name() << "::";
        }
        else if (has_inner(other) != nullptr)
        {
            DependencyModule* inner = has_inner(other);

            while (inner != other)
            {
                ss << inner->module().name() << "::";
                inner = inner->has_inner(other);
            }

            ss << other->module().name() << "::";
        }
        else // no relationship
        {
            ss << "::" << other->module().scope() << "::";
        }

        return ss.str();
    }

    /// \brief Helper method to generate an IDL sentence from a DependencyModule
    /// reference, given the corresponding DependencyNode whose IDL sentence wants to be
    /// generated. The method first checks if the node belongs to this DependencyModule.
    /// \param[in] node DependencyNode whose IDL sentence will be generated.
    /// \param[in] tabs Padding relative to module's scope.
    /// \returns An string with the generated IDL sentence, if node belongs to this module.
    std::string generate_idl_sentence(
            DependencyNode& node,
            unsigned int tabs,
            std::map<std::string, std::string>* module_idl = nullptr) const
    {
        if (std::find(node_set_.begin(), node_set_.end(), node) != node_set_.end())
        {
            return node.generate_idl_sentence(tabs, module_idl);
        }
        else
        {
            return std::string();
        }
    }

    /// \brief Generate IDL sentence for the DependencyNode contents of this module.
    /// \param[in] tabs Padding relative to module's scope.
    /// \param[in] is_child If set, generate IDL sentence only for DependencyNode that has as child
    /// the "is_child" DependencyModule reference.
    /// \returns An string with the generated IDL.
    std::string generate_idl_module(
            unsigned int tabs = 0,
            std::map<std::string, std::string>* struct_idl = nullptr)
    {
        std::stringstream ss;

        for (auto& node : node_set_)
        {
            if (node.has_ancestors())
            {
                for (const auto& ancestor : node.ancestors())
                {
                    std::stringstream ss_ancestor;
                    ss_ancestor << ancestor.get().generate_idl_sentence(tabs, struct_idl);

                    if (ss_ancestor.rdbuf()->in_avail())
                    {
                        ss << ss_ancestor.str() << std::endl;
                    }
                }
            }

            std::stringstream ss_node;
            ss_node << node.generate_idl_sentence(tabs, struct_idl);

            if (ss_node.rdbuf()->in_avail() &&
                (!all_nodes_iterated() || (!d_inner_.empty() && !all_inner_iterated())))
            {
                ss_node << std::endl;
            }

            ss << ss_node.str();

            if (struct_idl != nullptr)
            {
                (*struct_idl)[node.type().name()] = ss_node.str();
            }
        }

        return ss.str();
    }

private:

    bool iterated_;
    const Module& module_;
    const std::shared_ptr<DependencyModule>& d_root_;
    DependencyModule* d_outer_;
    ModuleSet d_inner_;
    ModuleRefSet ancestors_;
    NodeSet node_set_;
};

/// \brief A class for setting a Module object dependencies properly and generating its IDL.
class ModuleDependencyTree
{
public:

    /// \brief Construct a new ModuleDependencyTree object.
    /// \param[in] root Module whose dependencies wants to be solved.
    ModuleDependencyTree(
            const Module& root)
    {
        dep_root_ = std::make_shared<DependencyModule>(dep_root_, root, nullptr);
        create_module_dep_tree(dep_root_.get());
        solve_dependency_tree(dep_root_.get());
    }

    /// \brief Create DependencyModule tree from root node.
    /// \param[in] outer DependencyModule to be set as outer during construction of
    /// new DependencyModule objects.
    void create_module_dep_tree(
            DependencyModule* outer) const
    {
        outer->create_dependency_set();

        outer->module().for_each_submodule([&](const Module& submod)
        {
            std::shared_ptr<DependencyModule> child =
                std::make_shared<DependencyModule>(dep_root_, submod, outer);
            create_module_dep_tree(child.get());
            outer->set_inner(std::move(child));
        }, false);
    }

    /// \brief Solve dependency tree between DependencyModule objects.
    /// \param[in] dep_mod the DependencyModule whose dependencies wants to be solved.
    void solve_dependency_tree(
            DependencyModule* dep_mod) const
    {
        dep_mod->solve_dependency_tree();

        for (const auto& inner : dep_mod->inner())
        {
            this->solve_dependency_tree(inner.get());
        }
    }

    /// \brief Generate IDL sentence for a given DependencyModule.
    /// \param[in] dep_mod the DependencyModule whose IDL wants to be generated.
    /// \param[in] tabs Padding relative to module's scope.
    /// \returns A string representing IDL definition for the given module.
    std::string generate_idl(
            DependencyModule* dep_mod,
            unsigned int tabs=0,
            std::map<std::string, std::string>* module_idl = nullptr) const
    {
        if (dep_mod->iterated())
        {
            return std::string();
        }

        bool root = dep_mod->module().name().empty();
        std::stringstream ss;

        // Generate DependencyModule ancestors first
        if (dep_mod->has_ancestors())
        {
            for (const auto& ancestor : dep_mod->ancestors())
            {
                DependencyModule* p_ancestor = ancestor.lock().get();
                if (!p_ancestor->iterated())
                {
                    ss << generate_idl(p_ancestor, root ? 0 : tabs, module_idl) << std::endl;
                }
            }
        }

        if (dep_mod->has_outer())
        {
            DependencyModule* outer = dep_mod->outer();
            for (auto& onode : outer->node_set())
            {
                if (onode.has_child_module(dep_mod))
                {
                    ss << outer->generate_idl_sentence(onode, tabs, module_idl);

                    if (!outer->all_nodes_iterated())
                    {
                        ss << std::endl;
                    }
                }
            }
        }

        if (!root)
        {
            ss << std::string(4 * tabs, ' ') << "module " << dep_mod->module().name() << std::endl;
            ss << std::string(4 * tabs, ' ') << "{" << std::endl;

            if (module_idl != nullptr)
            {
                if (module_idl->find(dep_mod->module().name() + ":DependencyModule") == module_idl->end() && !root)
                {
                    if (dep_mod->has_outer() && module_idl->find(dep_mod->outer()->module().name() + ":DependencyModule")
                         != module_idl->end())
                    {
                        (*module_idl)[dep_mod->outer()->module().name() + ":DependencyModule"] += ss.str();
                    }
                    else
                    {
                        (*module_idl)[dep_mod->module().name() + ":DependencyModule"] = ss.str();
                    }
                }
            }
        }

        for (auto& node : dep_mod->node_set())
        {
            if (node.has_parent_modules())
            {
                for (const auto& parent : node.parent_modules())
                {
                    if (!parent.lock().get()->iterated())
                    {
                        ss << generate_idl(parent.lock().get(), root ? 0 : tabs + 1, module_idl) << std::endl;
                    }
                }
            }
        }

        ss << dep_mod->generate_idl_module(root ? 0 : tabs + 1, module_idl);

        // Iterate over inner DependencyModule list
        for (const auto& inner : dep_mod->inner())
        {
            ss << generate_idl(inner.get(), root ? 0 : tabs + 1, module_idl);

            if (!dep_mod->all_inner_iterated())
            {
                ss << std::endl;
            }
        }

        if (module_idl != nullptr)
        {
            for (const auto& pair : *module_idl)
            {
                if (pair.first.find(":DependencyModule") == std::string::npos
                        && pair.first.find(":dependencies") == std::string::npos
                        && pair.first.find(dep_mod->module().name()) != std::string::npos)
                {
                    std::string dep = "";

                    if (module_idl->find(dep_mod->module().name() + ":DependencyModule") != module_idl->end())
                    {
                        dep = (*module_idl)[dep_mod->module().name() + ":DependencyModule"];
                    }

                    (*module_idl)[pair.first] = dep + pair.second;
                }
            }
        }

        if (!root)
        {
            ss << std::string(4 * tabs, ' ') << "};" << std::endl;

            if (module_idl != nullptr)
            {
                for (const auto& pair : *module_idl)
                {
                    if (pair.first.find(":DependencyModule") == std::string::npos
                            && pair.first.find(":dependencies") == std::string::npos
                            && pair.first.find(dep_mod->module().name()) != std::string::npos)
                    {
                        if ((dep_mod->has_outer() && pair.first.find(dep_mod->outer()->module().name()) != std::string::npos)
                                || !dep_mod->has_outer())
                        {
                            (*module_idl)[pair.first] = pair.second + std::string(4 * tabs, ' ') + "};\n";
                        }

                    }
                }
            }
        }

        dep_mod->set_iterated(true);

        if (module_idl != nullptr)
        {
            auto it = module_idl->find(dep_mod->module().name() + ":DependencyModule");
            if(it != module_idl->end())
            {
                module_idl->erase(it);
            }
        }

        return ss.str();
    }

    /// \brief Helper method to generate IDL from root Module of this ModuleDependencyTree object.
    /// \returns A string representing IDL definition for root Module.
    inline std::string generate_idl(
        std::map<std::string, std::string>* module_idl = nullptr) const
    {
        return generate_idl(dep_root_.get(), 0, module_idl);
    }

private:

    std::shared_ptr<DependencyModule> dep_root_;
};

/// \brief Helper function to generate an IDL from a Module.
/// \param[in] module The Module whose IDL wants to be created.
/// \returns An string containing IDL definition for the given Module.
inline std::string idl_from_module(
    const Module& module,
    std::map<std::string, std::string>* module_idl = nullptr)
{
    return ModuleDependencyTree(module).generate_idl(module_idl);
}

} //namespace dependencytree
/// \brief Helper function to generate an IDL from a Module.
/// Keeps backwards compatibility with previous 'generator::module' method
/// and abstracts the user about accessing dependencytree namespace.
/// \param[in] module The module whose IDL wants to be created.
/// \returns An string containing IDL definition for the given Module.
inline std::string module(
    const Module& module,
    std::map<std::string, std::string>* module_idl = nullptr)
{
    return dependencytree::idl_from_module(module, module_idl);
}
} //namespace generator
} //namespace idl
} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_IDL_GENERATOR_DEPENDENCYTREE_HPP_
