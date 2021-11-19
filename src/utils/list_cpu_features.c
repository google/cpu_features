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

// This program dumps current host data to the standard output.
// Output can be text or json if the `--json` flag is passed.

#include "bump_allocator.h"
#include "node.h"

static void showUsage(const char* name) {
  printf(
      "\n"
      "Usage: %s [options]\n"
      "      Options:\n"
      "      -h | --help     Show help message.\n"
      "      -j | --json     Format output as json instead of plain text.\n"
      "\n",
      name);
}

int main(int argc, char** argv) {
  BA_Align();
  const Node* const root = CreateTree();
  bool outputJson = false;
  int i = 1;
  for (; i < argc; ++i) {
    const char* arg = argv[i];
    if (strcmp(arg, "-j") == 0 || strcmp(arg, "--json") == 0) {
      outputJson = true;
    } else {
      showUsage(argv[0]);
      if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        return EXIT_SUCCESS;
      return EXIT_FAILURE;
    }
  }
  if (outputJson)
    printJson(root);
  else
    printTextRoot(root);
  putchar('\n');
  return EXIT_SUCCESS;
}
