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

#ifdef CPU_FEATURES_ARCH_PPC
#include "cpuinfo_ppc.h"
#include "list_cpu_features__base_implementation.inl"

DEFINE_ADD_FLAGS(GetPPCFeaturesEnumValue, GetPPCFeaturesEnumName, PPCFeatures,
                 PPC_LAST_)

Node* CreateTree() {
  Node* root = CreateMap();
  const PPCInfo info = GetPPCInfo();
  const PPCPlatformStrings strings = GetPPCPlatformStrings();
  AddMapEntry(root, "arch", CreateString("ppc"));
  AddMapEntry(root, "platform", CreateString(strings.platform));
  AddMapEntry(root, "model", CreateString(strings.model));
  AddMapEntry(root, "machine", CreateString(strings.machine));
  AddMapEntry(root, "cpu", CreateString(strings.cpu));
  AddMapEntry(root, "instruction", CreateString(strings.type.platform));
  AddMapEntry(root, "microarchitecture",
              CreateString(strings.type.base_platform));
  AddFlags(root, &info.features);
  return root;
}
#endif  // CPU_FEATURES_ARCH_PPC
