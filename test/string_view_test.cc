// Copyright 2017 Google Inc.
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

#include "internal/string_view.h"

#include "gtest/gtest.h"

namespace cpu_features {

bool operator==(const StringView& a, const StringView& b) {
  return IsEquals(a, b);
}

namespace {

TEST(StringViewTest, Empty) {
  EXPECT_EQ(kEmptyStringView.ptr, nullptr);
  EXPECT_EQ(kEmptyStringView.size, 0);
}

TEST(StringViewTest, Build) {
  const auto view = str("test");
  EXPECT_EQ(view.ptr[0], 't');
  EXPECT_EQ(view.size, 4);
}

TEST(StringViewTest, IndexOfChar) {
  // Found.
  EXPECT_EQ(IndexOfChar(str("test"), 'e'), 1);
  // Not found.
  EXPECT_EQ(IndexOfChar(str("test"), 'z'), -1);
  // Empty.
  EXPECT_EQ(IndexOfChar(kEmptyStringView, 'z'), -1);
}

TEST(StringViewTest, IndexOf) {
  // Found.
  EXPECT_EQ(IndexOf(str("test"), str("es")), 1);
  // Not found.
  EXPECT_EQ(IndexOf(str("test"), str("aa")), -1);
  // Empty.
  EXPECT_EQ(IndexOf(kEmptyStringView, str("aa")), -1);
  EXPECT_EQ(IndexOf(str("aa"), kEmptyStringView), -1);
}

TEST(StringViewTest, StartsWith) {
  EXPECT_TRUE(StartsWith(str("test"), str("te")));
  EXPECT_FALSE(StartsWith(str("test"), str("")));
  EXPECT_FALSE(StartsWith(str("test"), kEmptyStringView));
  EXPECT_FALSE(StartsWith(kEmptyStringView, str("test")));
}

TEST(StringViewTest, IsEquals) {
  EXPECT_TRUE(IsEquals(kEmptyStringView, kEmptyStringView));
  EXPECT_TRUE(IsEquals(kEmptyStringView, str("")));
  EXPECT_TRUE(IsEquals(str(""), kEmptyStringView));
  EXPECT_TRUE(IsEquals(str("a"), str("a")));
  EXPECT_FALSE(IsEquals(str("a"), str("b")));
  EXPECT_FALSE(IsEquals(str("a"), kEmptyStringView));
  EXPECT_FALSE(IsEquals(kEmptyStringView, str("a")));
}

TEST(StringViewTest, PopFront) {
  EXPECT_EQ(PopFront(str("test"), 2), str("st"));
  EXPECT_EQ(PopFront(str("test"), 0), str("test"));
  EXPECT_EQ(PopFront(str("test"), 4), str(""));
  EXPECT_EQ(PopFront(str("test"), 100), str(""));
}

TEST(StringViewTest, ParsePositiveNumber) {
  EXPECT_EQ(ParsePositiveNumber(str("42")), 42);
  EXPECT_EQ(ParsePositiveNumber(str("0x2a")), 42);
  EXPECT_EQ(ParsePositiveNumber(str("0x2A")), 42);

  EXPECT_EQ(ParsePositiveNumber(str("-0x2A")), -1);
  EXPECT_EQ(ParsePositiveNumber(str("abc")), -1);
  EXPECT_EQ(ParsePositiveNumber(str("")), -1);
}

TEST(StringViewTest, CopyString) {
  char buf[4];
  buf[0] = 'X';

  // Empty
  CopyString(str(""), buf, sizeof(buf));
  EXPECT_STREQ(buf, "");

  // Less
  CopyString(str("a"), buf, sizeof(buf));
  EXPECT_STREQ(buf, "a");

  // exact
  CopyString(str("abc"), buf, sizeof(buf));
  EXPECT_STREQ(buf, "abc");

  // More
  CopyString(str("abcd"), buf, sizeof(buf));
  EXPECT_STREQ(buf, "abc");
}

TEST(StringViewTest, HasWord) {
  // Find flags at beginning, middle and end.
  EXPECT_TRUE(HasWord(str("first middle last"), "first"));
  EXPECT_TRUE(HasWord(str("first middle last"), "middle"));
  EXPECT_TRUE(HasWord(str("first middle last"), "last"));
  // Do not match partial flags
  EXPECT_FALSE(HasWord(str("first middle last"), "irst"));
  EXPECT_FALSE(HasWord(str("first middle last"), "mid"));
  EXPECT_FALSE(HasWord(str("first middle last"), "las"));
}

TEST(StringViewTest, GetAttributeKeyValue) {
  const StringView line = str(" key :   first middle last   ");
  StringView key, value;
  EXPECT_TRUE(GetAttributeKeyValue(line, &key, &value));
  EXPECT_EQ(key, str("key"));
  EXPECT_EQ(value, str("first middle last"));
}

TEST(StringViewTest, FailingGetAttributeKeyValue) {
  const StringView line = str("key  first middle last");
  StringView key, value;
  EXPECT_FALSE(GetAttributeKeyValue(line, &key, &value));
}

}  // namespace
}  // namespace cpu_features
