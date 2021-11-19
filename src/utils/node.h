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

#ifndef CPU_FEATURES_NODE_H_
#define CPU_FEATURES_NODE_H_

#include <assert.h>
#include <cpu_features_cache_info.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The type of the nodes in the tree.
typedef enum {
  NT_INVALID,
  NT_INT,
  NT_MAP,
  NT_MAP_ENTRY,
  NT_ARRAY,
  NT_ARRAY_ELEMENT,
  NT_STRING,
} NodeType;

// The node in the tree.
typedef struct Node {
  NodeType type;
  unsigned integer;
  const char* string;
  struct Node* value;
  struct Node* next;
} Node;

Node* CreateTree();
Node* CreateMap();
Node* BA_CreateNode(NodeType type);
Node* CreateArray();
Node* CreateInt(int value);
Node* CreatePrintfString(const char* format, ...);
Node* CreateString(const char* value);
Node* CreateConstantString(const char* value);
Node* GetCacheTypeString(CacheType cache_type);
void AddMapEntry(Node* map, const char* key, Node* value);
void AddCacheInfo(Node* root, const CacheInfo* cache_info);
void AddArrayElement(Node* array, Node* value);

// Prints a json string with characters escaping.
void printJsonString(const char* str);

// Walks a Node and print it as json.
void printJson(const Node* current);

// Walks a Node and print it as text.
void printTextField(const Node* current);

void printTextRoot(const Node* current);

#endif  // CPU_FEATURES_NODE_H_
