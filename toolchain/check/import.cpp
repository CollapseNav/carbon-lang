// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "toolchain/check/import.h"

#include "common/check.h"
#include "toolchain/check/context.h"
#include "toolchain/parse/node_ids.h"
#include "toolchain/sem_ir/file.h"
#include "toolchain/sem_ir/ids.h"
#include "toolchain/sem_ir/inst.h"
#include "toolchain/sem_ir/typed_insts.h"
#include "toolchain/sem_ir/value_stores.h"

namespace Carbon::Check {

// Returns name information for the entity, corresponding to IDs in the import
// IR rather than the current IR. May return Invalid for a TODO.
static auto GetImportName(Parse::NodeId parse_node, Context& context,
                          const SemIR::File& import_sem_ir,
                          SemIR::Inst import_inst)
    -> std::pair<SemIR::NameId, SemIR::NameScopeId> {
  switch (import_inst.kind()) {
    case SemIR::InstKind::BindName: {
      const auto& bind_name = import_sem_ir.bind_names().Get(
          import_inst.As<SemIR::BindName>().bind_name_id);
      return {bind_name.name_id, bind_name.enclosing_scope_id};
    }

    case SemIR::InstKind::FunctionDecl: {
      const auto& function = import_sem_ir.functions().Get(
          import_inst.As<SemIR::FunctionDecl>().function_id);
      return {function.name_id, function.enclosing_scope_id};
    }

    case SemIR::InstKind::Namespace: {
      auto namespace_inst = import_inst.As<SemIR::Namespace>();
      const auto& scope =
          import_sem_ir.name_scopes().Get(namespace_inst.name_scope_id);
      return {namespace_inst.name_id, scope.enclosing_scope_id};
    }

    default:
      context.TODO(parse_node, (llvm::Twine("Support GetImportName of ") +
                                import_inst.kind().name())
                                   .str());
      return {SemIR::NameId::Invalid, SemIR::NameScopeId::Invalid};
  }
}

// Translate the name to the current IR. It will usually be an identifier, but
// could also be a builtin name ID which is equivalent cross-IR.
static auto CopyNameFromImportIR(Context& context,
                                 const SemIR::File& import_sem_ir,
                                 SemIR::NameId import_name_id) {
  if (auto import_identifier_id = import_name_id.AsIdentifierId();
      import_identifier_id.is_valid()) {
    auto name = import_sem_ir.identifiers().Get(import_identifier_id);
    return SemIR::NameId::ForIdentifier(context.identifiers().Add(name));
  }
  return import_name_id;
}

// Creates a namespace. The type ID is builtin, and reused to avoid duplicative
// canonicalization.
static auto AddNamespace(Context& context,
                         SemIR::NameScopeId enclosing_scope_id,
                         SemIR::NameId name_id, SemIR::TypeId namespace_type_id)
    -> std::pair<SemIR::InstId, SemIR::NameScopeId> {
  // Use the invalid node because there's no node to associate with.
  auto inst =
      SemIR::Namespace{namespace_type_id, name_id, SemIR::NameScopeId::Invalid};
  auto id = context.AddPlaceholderInst({Parse::NodeId::Invalid, inst});
  inst.name_scope_id = context.name_scopes().Add(id, enclosing_scope_id);
  context.ReplaceInstBeforeConstantUse(id, {Parse::NodeId::Invalid, inst});
  return {id, inst.name_scope_id};
}

static auto CacheCopiedNamespace(
    llvm::DenseMap<SemIR::NameScopeId, SemIR::NameScopeId>& copied_namespaces,
    SemIR::NameScopeId import_scope_id, SemIR::NameScopeId to_scope_id)
    -> void {
  auto [it, success] = copied_namespaces.insert({import_scope_id, to_scope_id});
  CARBON_CHECK(success || it->second == to_scope_id)
      << "Copy result for namespace changed from " << import_scope_id << " to "
      << to_scope_id;
}

// Copies enclosing name scopes from the import IR. Handles the parent
// traversal. Returns the NameScope corresponding to the copied
// import_enclosing_scope_id.
static auto CopyEnclosingNameScopeFromImportIR(
    Context& context, SemIR::TypeId namespace_type_id,
    const SemIR::File& import_sem_ir,
    SemIR::NameScopeId import_enclosing_scope_id,
    llvm::DenseMap<SemIR::NameScopeId, SemIR::NameScopeId>& copied_namespaces)
    -> SemIR::NameScopeId {
  // Package-level names don't need work.
  if (import_enclosing_scope_id == SemIR::NameScopeId::Package) {
    return import_enclosing_scope_id;
  }

  // The scope to add namespaces to. Note this may change while looking at
  // enclosing scopes, if we encounter a namespace that's already added.
  auto scope_cursor = SemIR::NameScopeId::Package;

  // Build a stack of enclosing namespace names, with innermost first.
  llvm::SmallVector<std::pair<SemIR::NameScopeId, SemIR::NameId>>
      new_namespaces;
  while (import_enclosing_scope_id != SemIR::NameScopeId::Package) {
    // If the namespace was already copied, reuse the results.
    if (auto it = copied_namespaces.find(import_enclosing_scope_id);
        it != copied_namespaces.end()) {
      // We inject names at the provided scope, and don't need to keep
      // traversing parents.
      scope_cursor = it->second;
      break;
    }

    // The namespace hasn't been copied yet, so add it to our list.
    const auto& scope =
        import_sem_ir.name_scopes().Get(import_enclosing_scope_id);
    auto scope_inst =
        import_sem_ir.insts().GetAs<SemIR::Namespace>(scope.inst_id);
    new_namespaces.push_back({scope_inst.name_scope_id, scope_inst.name_id});
    import_enclosing_scope_id = scope.enclosing_scope_id;
  }

  // Add enclosing namespace names, starting with the outermost.
  for (auto import_namespace : llvm::reverse(new_namespaces)) {
    auto name_id =
        CopyNameFromImportIR(context, import_sem_ir, import_namespace.second);
    auto& scope = context.name_scopes().Get(scope_cursor);
    auto [it, success] = scope.names.insert({name_id, SemIR::InstId::Invalid});
    if (!success) {
      auto inst = context.insts().Get(it->second);
      if (auto namespace_inst = inst.TryAs<SemIR::Namespace>()) {
        // Namespaces are open, so we can append to the existing one even if it
        // comes from a different file.
        scope_cursor = namespace_inst->name_scope_id;
        CacheCopiedNamespace(copied_namespaces, import_namespace.first,
                             scope_cursor);
        continue;
      }
      // Produce a diagnostic, but still produce the namespace to supersede the
      // name conflict in order to avoid repeat diagnostics.
      // TODO: Produce a diagnostic.
    }

    // Produce the namespace for the entry.
    auto [namespace_id, name_scope_id] =
        AddNamespace(context, scope_cursor, name_id, namespace_type_id);

    it->second = namespace_id;
    scope_cursor = name_scope_id;
    CacheCopiedNamespace(copied_namespaces, import_namespace.first,
                         scope_cursor);
  }

  return scope_cursor;
}

auto Import(Context& context, SemIR::TypeId namespace_type_id,
            Parse::NodeId import_node, const SemIR::File& import_sem_ir)
    -> void {
  auto ir_id = context.sem_ir().cross_ref_irs().Add(&import_sem_ir);

  for (const auto import_inst_id :
       import_sem_ir.inst_blocks().Get(SemIR::InstBlockId::Exports)) {
    auto import_inst = import_sem_ir.insts().Get(import_inst_id);
    auto [import_name_id, import_enclosing_scope_id] =
        GetImportName(import_node, context, import_sem_ir, import_inst);
    // TODO: This should only be invalid when GetImportName for an inst
    // isn't yet implemented. Long-term this should be removed.
    if (!import_name_id.is_valid()) {
      continue;
    }

    llvm::DenseMap<SemIR::NameScopeId, SemIR::NameScopeId> copied_namespaces;

    auto name_id = CopyNameFromImportIR(context, import_sem_ir, import_name_id);
    SemIR::NameScopeId enclosing_scope_id = CopyEnclosingNameScopeFromImportIR(
        context, namespace_type_id, import_sem_ir, import_enclosing_scope_id,
        copied_namespaces);

    if (auto import_namespace_inst = import_inst.TryAs<SemIR::Namespace>()) {
      // Namespaces are always imported because they're essential for
      // qualifiers, and the type is simple.
      auto [namespace_id, name_scope_id] =
          AddNamespace(context, enclosing_scope_id, name_id, namespace_type_id);
      context.name_scopes().AddEntry(enclosing_scope_id, name_id, namespace_id);
      CacheCopiedNamespace(copied_namespaces,
                           import_namespace_inst->name_scope_id, name_scope_id);
    } else {
      // Leave a placeholder that the inst comes from the other IR.
      auto target_id = context.AddPlaceholderInst(
          {Parse::NodeId::Invalid,
           SemIR::LazyImportRef{.ir_id = ir_id, .inst_id = import_inst_id}});
      // TODO: When importing from other packages, the scope's names should
      // be changed to allow for ambiguous names. When importing from the
      // current package, as is currently being done, we should issue a
      // diagnostic on conflicts.
      context.name_scopes().AddEntry(enclosing_scope_id, name_id, target_id);
    }
  }
}

}  // namespace Carbon::Check
