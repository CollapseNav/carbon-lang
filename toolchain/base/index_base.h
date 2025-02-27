// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef CARBON_TOOLCHAIN_BASE_INDEX_BASE_H_
#define CARBON_TOOLCHAIN_BASE_INDEX_BASE_H_

#include <type_traits>

#include "common/ostream.h"
#include "llvm/ADT/DenseMapInfo.h"

namespace Carbon {

template <typename DataType>
class DataIterator;

// A lightweight handle to an item identified by an opaque ID.
//
// This class is intended to be derived from by classes representing a specific
// kind of ID, whose meaning as an integer is an implementation detail of the
// type that vends the IDs. Typically this will be a vector index.
//
// Classes derived from IdBase are designed to be passed by value, not
// reference or pointer. They are also designed to be small and efficient to
// store in data structures.
struct IdBase : public Printable<IdBase> {
  static constexpr int32_t InvalidIndex = -1;

  IdBase() = delete;
  constexpr explicit IdBase(int index) : index(index) {}

  auto Print(llvm::raw_ostream& output) const -> void {
    if (is_valid()) {
      output << index;
    } else {
      output << "<invalid>";
    }
  }

  auto is_valid() const -> bool { return index != InvalidIndex; }

  int32_t index;
};

// A lightweight handle to an item that behaves like an index.
//
// Unlike IdBase, classes derived from IndexBase are not completely opaque, and
// provide at least an ordering between indexes that has meaning to an API
// user. Additional semantics may be specified by the derived class.
struct IndexBase : public IdBase {
  using IdBase::IdBase;
};

// Equality comparison for both IdBase and IndexBase.
template <
    typename IndexType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr>
auto operator==(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index == rhs.index;
}
template <
    typename IndexType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr>
auto operator!=(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index != rhs.index;
}

template <
    typename IndexType, typename RHSType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr,
    typename std::enable_if_t<std::is_convertible_v<RHSType, IndexType>>* =
        nullptr>
auto operator==(IndexType lhs, RHSType rhs) -> bool {
  return lhs.index == IndexType(rhs).index;
}
template <
    typename IndexType, typename RHSType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr,
    typename std::enable_if_t<std::is_convertible_v<RHSType, IndexType>>* =
        nullptr>
auto operator!=(IndexType lhs, RHSType rhs) -> bool {
  return lhs.index != IndexType(rhs).index;
}

template <
    typename LHSType, typename IndexType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr,
    typename std::enable_if_t<std::is_convertible_v<LHSType, IndexType>>* =
        nullptr>
auto operator==(LHSType lhs, IndexType rhs) -> bool {
  return IndexType(lhs).index == rhs.index;
}
template <
    typename LHSType, typename IndexType,
    typename std::enable_if_t<std::is_base_of_v<IdBase, IndexType>>* = nullptr,
    typename std::enable_if_t<std::is_convertible_v<LHSType, IndexType>>* =
        nullptr>
auto operator!=(LHSType lhs, IndexType rhs) -> bool {
  return IndexType(lhs).index != rhs.index;
}

// The < and > comparisons for only IndexBase.
template <typename IndexType,
          typename std::enable_if_t<std::is_base_of_v<IndexBase, IndexType>>* =
              nullptr>
auto operator<(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index < rhs.index;
}
template <typename IndexType,
          typename std::enable_if_t<std::is_base_of_v<IndexBase, IndexType>>* =
              nullptr>
auto operator<=(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index <= rhs.index;
}
template <typename IndexType,
          typename std::enable_if_t<std::is_base_of_v<IndexBase, IndexType>>* =
              nullptr>
auto operator>(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index > rhs.index;
}
template <typename IndexType,
          typename std::enable_if_t<std::is_base_of_v<IndexBase, IndexType>>* =
              nullptr>
auto operator>=(IndexType lhs, IndexType rhs) -> bool {
  return lhs.index >= rhs.index;
}

// Provides base support for use of IdBase types as DenseMap/DenseSet keys.
//
// Usage (in global namespace):
//   template <>
//   struct llvm::DenseMapInfo<Carbon::MyType>
//       : public Carbon::IndexMapInfo<Carbon::MyType> {};
template <typename Index>
struct IndexMapInfo {
  static inline auto getEmptyKey() -> Index {
    return Index(llvm::DenseMapInfo<int32_t>::getEmptyKey());
  }

  static inline auto getTombstoneKey() -> Index {
    return Index(llvm::DenseMapInfo<int32_t>::getTombstoneKey());
  }

  static auto getHashValue(const Index& val) -> unsigned {
    return llvm::DenseMapInfo<int32_t>::getHashValue(val.index);
  }

  static auto isEqual(const Index& lhs, const Index& rhs) -> bool {
    return lhs == rhs;
  }
};

}  // namespace Carbon

#endif  // CARBON_TOOLCHAIN_BASE_INDEX_BASE_H_
