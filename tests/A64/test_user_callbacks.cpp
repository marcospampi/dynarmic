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

template <std::uint64_t N>
void test_callback_halt( A64::Jit *jit ) {
    auto value = jit->GetRegister(1);
    if ( value + 1 > N ) {
        jit->HaltExecution(HaltReason::UserDefined1);
    }
    else {
        jit->SetRegister(1, value + 1);
    }
}

TEST_CASE("A64: Test 0xDEADBEEF", "[a64]") {
    A64TestEnv env;
    const CallbackVector<u64, A64::Jit> callbacks = {
        {0x20, "deadbeef_test", test_callback_0xdeadbeef, true }
    };
    A64::UserConfig config{&env};
    config.user_callbacks = &callbacks;

    A64::Jit jit{config};

    env.code_mem.emplace_back(0xd63f0000);  // BLR X0
    env.code_mem.emplace_back(0x14000000);  // B .

    jit.SetRegister(0, 0x20);
    jit.SetRegister(1, 1);
    jit.SetPC(0);

    env.ticks_left = 2;
    jit.Run();

    REQUIRE(jit.GetRegister(1) == 0xDEADBEEF);
}


TEST_CASE("A64: Test increment", "[a64]") {
    // Increment N times register 1
    const auto N_times = 0xdead;

    A64TestEnv env;
    const CallbackVector<u64, A64::Jit> callbacks = {
        {0x100, "test_callback_increment", test_callback_increment, true }
    };
    A64::UserConfig config{&env};
    config.user_callbacks = &callbacks;

    A64::Jit jit{config};

    env.code_mem.emplace_back(0xd63f0000);  // BLR X0
    env.code_mem.emplace_back(0x17FFFFFF);  // B #-4

    jit.SetRegister(0, 0x100);
    jit.SetRegister(1, 0);
    jit.SetPC(0);

    env.ticks_left = N_times*2 - 1;
    jit.Run();

    REQUIRE(jit.GetRegister(1) == N_times);

}

TEST_CASE("A64: Test test_callback_halt!", "[a64]") {
    // Increment N times register 1
    constexpr auto N = 64;

    A64TestEnv env;
    A64::UserConfig config{&env};
    const CallbackVector<u64, A64::Jit> callbacks = {
        {0x100, "test_callback_halt", test_callback_halt<N>, true }
    };
    config.user_callbacks = &callbacks;

    A64::Jit jit{config};

    env.code_mem.emplace_back(0xd63f0000);  // BLR X0
    env.code_mem.emplace_back(0x17FFFFFF);  // B #-4

    jit.SetRegister(0, 0x100);
    jit.SetRegister(1, 0);
    jit.SetPC(0);

    env.ticks_left = N*4 ;
    auto result = jit.Run();

    REQUIRE(jit.GetRegister(1) == N);
    REQUIRE(result == HaltReason::UserDefined1);

}