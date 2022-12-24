/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#include <catch2/catch.hpp>
#include <iostream>
#include "./testenv.h"
#include "dynarmic/interface/A32/a32.h"

using namespace Dynarmic;

void test_callback_set_reg_0xdeadbef( A32::Jit *jit ) {
    jit->Regs()[1] = 0xDEADBEEF ;
}
void test_callback_increment( A32::Jit *jit ) {
    auto value = jit->Regs()[1];
    jit->Regs()[1] = value + 1;
}

template <std::uint32_t N>
void test_callback_halt( A32::Jit *jit ) {
    auto value = jit->Regs()[1];
    std::cout << fmt::format("R1= 0x{:08X}", value) << std::endl;

    if ( value + 1 > N ) {
        
        std::cout << fmt::format("Halt!") << std::endl;
        jit->HaltExecution(HaltReason::UserDefined1);
    }
    else {
        std::cout << fmt::format("Increment!") << std::endl;
        jit->Regs()[1] = value + 1;
    }
}

TEST_CASE("A32: Test 0xDEADBEEF", "[a32]") {
    ArmTestEnv test_env;
    A32::UserConfig config{&test_env};
    config.thunk_vector = {
        {0x20, "test_callback_set_reg_0xdeadbef", test_callback_set_reg_0xdeadbef, true }
    };

    A32::Jit jit{config};
    
    test_env.code_mem = {
        0xe12fff30,
        0xeafffffe  // b +#0
    };

    jit.Regs()[0] = 0x20;
    jit.Regs()[15]  = 0;
    jit.SetCpsr(0x000001d0);  // User-mode

    test_env.ticks_left = 4;
    jit.Run();

    REQUIRE(jit.Regs()[1]== 0xDEADBEEF);
}


TEST_CASE("A32: Test increment", "[a32]") {
    // Increment N times register 1
    const auto N_times = 0xdead;

    ArmTestEnv test_env;
    A32::UserConfig config{&test_env};
    config.thunk_vector = {
        {0x20, "test_callback_increment", test_callback_increment, true }
    };

    A32::Jit jit{config};
    
    test_env.code_mem = {
        0xe12fff30, // blx r0
        0xeafffffd  // b -#4 
    };

    jit.Regs()[0] = 0x20;
    jit.Regs()[15]  = 0;
    jit.SetCpsr(0x000001d0);  // User-mode

    test_env.ticks_left = N_times*2 - 1;
    jit.Run();

    REQUIRE(jit.Regs()[1]== N_times);

}

TEST_CASE("A32: Test test_callback_halt!", "[a32]") {
    constexpr auto N = 64;

    ArmTestEnv test_env;
    A32::UserConfig config{&test_env};
    config.thunk_vector = {
        {0x20, "test_callback_increment", test_callback_halt<N>, true }
    };

    A32::Jit jit{config};
    
    test_env.code_mem = {
        0xe12fff30, // blx r0
        0xeafffffd  // b -#4 
    };

    jit.Regs()[0] = 0x20;
    jit.Regs()[15]  = 0;
    jit.SetCpsr(0x000001d0);  // User-mode

    test_env.ticks_left = N*4;
    auto result = jit.Run();

    REQUIRE(jit.Regs()[1]== N);
    REQUIRE( result== HaltReason::UserDefined1 );

}