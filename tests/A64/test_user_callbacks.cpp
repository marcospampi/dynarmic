/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#include <catch2/catch.hpp>
#include <iostream>
#include "./testenv.h"
#include "dynarmic/interface/A64/a64.h"

using namespace Dynarmic;

void test_callback_0xdeadbeef( A64::Jit *jit ) {
    jit->SetRegister(1, 0xDEADBEEF);
}

void test_callback_increment( A64::Jit *jit ) {
    auto value = jit->GetRegister(1);
    jit->SetRegister(1, value + 1);
}

TEST_CASE("A64: Test 0xDEADBEEF", "[a64]") {
    A64TestEnv env;
    A64::UserConfig config{&env};
    config.user_hook_callback = {
        {0x20, "deadbeef_test", test_callback_0xdeadbeef, true }
    };

    A64::Jit jit{config};

    env.code_mem.emplace_back(0xd63f0000);  // BLR X0
    env.code_mem.emplace_back(0x14000000);  // B .

    jit.SetRegister(0, 0x20);
    jit.SetRegister(1, 1);
    jit.SetPC(0);

    env.ticks_left = 2;
    jit.Run();

    //REQUIRE(jit.GetRegister(0) == 3);
    REQUIRE(jit.GetRegister(1) == 0xDEADBEEF);
    //REQUIRE(jit.GetRegister(2) == 2);
    //REQUIRE(jit.GetPC() == 4);
}

void test_callback0xdeadbeef( A64::Jit *jit ) {
    jit->SetRegister(1, 0xDEADBEEF);
}

TEST_CASE("A64: Test increment", "[a64]") {
    A64TestEnv env;
    A64::UserConfig config{&env};
    config.user_hook_callback = {
        {0x100, "test_callback_increment", test_callback_increment, true }
    };

    A64::Jit jit{config};

    env.code_mem.emplace_back(0xd63f0000);  // BLR X0
    env.code_mem.emplace_back(0x17FFFFFF);  // B .

    jit.SetRegister(0, 0x100);
    jit.SetRegister(1, 0);
    jit.SetPC(0);

    env.ticks_left = 20*2 - 1;
    jit.Run();

    //REQUIRE(jit.GetRegister(0) == 3);
    REQUIRE(jit.GetRegister(1) == 20);
    //REQUIRE(jit.GetRegister(2) == 2);
    //REQUIRE(jit.GetPC() == 4);
}