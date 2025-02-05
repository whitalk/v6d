/** Copyright 2020-2023 Alibaba Group Holding Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef SRC_COMMON_UTIL_ARROW_H_
#define SRC_COMMON_UTIL_ARROW_H_

#include <memory>
#include <utility>
#include <vector>

#include "arrow/api.h"
#include "arrow/io/api.h"

#include "cityhash/cityhash.hpp"
#include "wyhash/wyhash.hpp"

#include "common/util/status.h"

namespace vineyard {

#if ARROW_VERSION_MAJOR >= 10
using arrow_string_view = std::string_view;
#else
using arrow_string_view = arrow::util::string_view;
#endif

using table_vec_t = std::vector<std::shared_ptr<arrow::Table>>;
using array_vec_t = std::vector<std::shared_ptr<arrow::Array>>;

}  // namespace vineyard

namespace wy {
#if !nssv_USES_STD_STRING_VIEW && \
    ((!__cpp_lib_string_view) || ARROW_VERSION_MAJOR < 10)
template <>
struct hash<vineyard::arrow_string_view>
    : public internal::hash_string_base<vineyard::arrow_string_view> {
  using hash_string_base::hash_string_base;  // Inherit constructors
};
#endif
}  // namespace wy

namespace city {
#if !nssv_USES_STD_STRING_VIEW && \
    ((!__cpp_lib_string_view) || ARROW_VERSION_MAJOR < 10)
template <>
class hash<vineyard::arrow_string_view> {
  inline uint64_t operator()(
      const vineyard::arrow_string_view& data) const noexcept {
    return detail::CityHash64(reinterpret_cast<const char*>(data.data()),
                              data.size());
  }
};
#endif
}  // namespace city

namespace vineyard {

#ifndef CHECK_ARROW_ERROR
#define CHECK_ARROW_ERROR(expr) \
  VINEYARD_CHECK_OK(::vineyard::Status::ArrowError(expr))
#endif  // CHECK_ARROW_ERROR

// discard and ignore the error status.
#ifndef DISCARD_ARROW_ERROR
#define DISCARD_ARROW_ERROR(expr)                               \
  do {                                                          \
    auto status = (expr);                                       \
    if (!status.ok()) {} /* NOLINT(whitespace/empty_if_body) */ \
  } while (0)
#endif  // DISCARD_ARROW_ERROR

#ifndef CHECK_ARROW_ERROR_AND_ASSIGN
#define CHECK_ARROW_ERROR_AND_ASSIGN(lhs, expr) \
  do {                                          \
    auto status = (expr);                       \
    CHECK_ARROW_ERROR(status.status());         \
    lhs = std::move(status).ValueOrDie();       \
  } while (0)
#endif  // CHECK_ARROW_ERROR_AND_ASSIGN

#ifndef RETURN_ON_ARROW_ERROR
#define RETURN_ON_ARROW_ERROR(expr)                  \
  do {                                               \
    auto status = (expr);                            \
    if (!status.ok()) {                              \
      return ::vineyard::Status::ArrowError(status); \
    }                                                \
  } while (0)
#endif  // RETURN_ON_ARROW_ERROR

#ifndef RETURN_ON_ARROW_ERROR_AND_ASSIGN
#define RETURN_ON_ARROW_ERROR_AND_ASSIGN(lhs, expr)           \
  do {                                                        \
    auto result = (expr);                                     \
    if (!result.status().ok()) {                              \
      return ::vineyard::Status::ArrowError(result.status()); \
    }                                                         \
    lhs = std::move(result).ValueOrDie();                     \
  } while (0)
#endif  // RETURN_ON_ARROW_ERROR_AND_ASSIGN

}  // namespace vineyard

#endif  // SRC_COMMON_UTIL_ARROW_H_
