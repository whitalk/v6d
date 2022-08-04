/** Copyright 2020-2021 Alibaba Group Holding Limited.

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

#ifndef SRC_SERVER_MEMORY_MIMALLOC_H_
#define SRC_SERVER_MEMORY_MIMALLOC_H_

#if defined(WITH_MIMALLOC)

#include "common/memory/mimalloc.h"

namespace vineyard {

namespace memory {

class MimallocAllocator : public Mimalloc {
 public:
  void* Init(const size_t size);
};

}  // namespace memory

}  // namespace vineyard

#endif  // WITH_MIMALLOC

#endif  // SRC_SERVER_MEMORY_MIMALLOC_H_