// Copyright 2017 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cpu_features_macros.h"

#ifdef CPU_FEATURES_ARCH_AARCH64
#include "cpuinfo_aarch64.h"
#include "list_cpu_features__base_implementation.inl"

DEFINE_ADD_FLAGS(GetAarch64FeaturesEnumValue, GetAarch64FeaturesEnumName,
                 Aarch64Features, AARCH64_LAST_)

Node* CreateTree() {
  Node* root = CreateMap();
  const Aarch64Info info = GetAarch64Info();
  AddMapEntry(root, "arch", CreateString("aarch64"));
  AddMapEntry(root, "implementer", CreateInt(info.implementer));
  AddMapEntry(root, "variant", CreateInt(info.variant));
  AddMapEntry(root, "part", CreateInt(info.part));
  AddMapEntry(root, "revision", CreateInt(info.revision));
  AddFlags(root, &info.features);
  return root;
}
#endif  // CPU_FEATURES_ARCH_AARCH64
