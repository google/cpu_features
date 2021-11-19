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

#ifdef CPU_FEATURES_ARCH_X86
#include "cpuinfo_x86.h"
#include "list_cpu_features__base_implementation.inl"

DEFINE_ADD_FLAGS(GetX86FeaturesEnumValue, GetX86FeaturesEnumName, X86Features,
                 X86_LAST_)

Node* CreateTree() {
  Node* root = CreateMap();
  char brand_string[49];
  const X86Info info = GetX86Info();
  const CacheInfo cache_info = GetX86CacheInfo();
  FillX86BrandString(brand_string);
  AddMapEntry(root, "arch", CreateString("x86"));
  AddMapEntry(root, "brand", CreateString(brand_string));
  AddMapEntry(root, "family", CreateInt(info.family));
  AddMapEntry(root, "model", CreateInt(info.model));
  AddMapEntry(root, "stepping", CreateInt(info.stepping));
  AddMapEntry(root, "uarch",
              CreateString(
                  GetX86MicroarchitectureName(GetX86Microarchitecture(&info))));
  AddFlags(root, &info.features);
  AddCacheInfo(root, &cache_info);
  return root;
}
#endif  // CPU_FEATURES_ARCH_X86