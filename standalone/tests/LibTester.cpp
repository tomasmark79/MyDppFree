// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include "../src/AppCore.hpp"
#include <gtest/gtest.h>

TEST (AppLogic, HandlesArguments) {
  const char* argv[] = { "DotNameStandalone", "--help" };
  EXPECT_EQ (runApp (2, argv), 0);
}

TEST (AppLogic, HandlesArgumentsNoLibrary) {
  const char* argv[] = { "DotNameStandalone", "--omit" };
  EXPECT_EQ (runApp (2, argv), 0);
}

TEST (AppLogic, HandlesArgumentsLog2File) {
  const char* argv[] = { "DotNameStandalone", "--log2file" };
  EXPECT_EQ (runApp (2, argv), 0);
}
