#include <catch2/catch.hpp>
#include <iostream>
#include "./testenv.h"
#include "dynarmic/interface/A64/a64.h"

using namespace Dynarmic;
constexpr size_t STACK_SIZE = 4096;


void exit_address_halt( A64::Jit *jit ) {
    jit->HaltExecution(HaltReason::ExitAddress);
}

TEST_CASE("A64: Test vmcall return 0x7", "[a64]") {
    
    A64FastmemOnlyTestEnv test_env(0);
    A64::UserConfig config{ &test_env };
    // from devkitA64 example, jit
    u32 return_7[2] = {0xd2800000 | (0x7<<5), 0xd65f03c0};//"mov x0, #0x7" "ret"

    config.fastmem_address_space_bits = 64;
    config.fastmem_pointer = (void*)0;

    const auto stack = new (std::align_val_t(STACK_SIZE)) u8[STACK_SIZE];
    const auto stack_top = (uintptr_t)stack + STACK_SIZE;

    A64::Jit jit{config};

    jit.SetPC(stack_top);

    u64 result = jit.VMCall<u64>(reinterpret_cast<u64>(return_7));

    REQUIRE(result == 0x7);
}

TEST_CASE("A64: Test vmcall add", "[a64]") {
    
    A64FastmemOnlyTestEnv test_env(0);
    A64::UserConfig config{ &test_env };
    u8 add[] = {
        0xff,0x43,0x0,0xd1, // sub     sp, sp, #16
        0xe0,0xf,0x0,0xb9,  // str     w0, [sp, 12]
        0xe1,0xb,0x0,0xb9,  // str     w1, [sp, 8]
        0xe1,0xf,0x40,0xb9, // ldr     w1, [sp, 12]
        0xe0,0xb,0x40,0xb9, // ldr     w0, [sp, 8]
        0x20,0x0,0x0,0xb,   // add     w0, w1, w0
        0xff,0x43,0x0,0x91, // add     sp, sp, 16
        0xc0,0x3,0x5f,0xd6  // ret
    };
    config.fastmem_address_space_bits = 64;
    config.fastmem_pointer = (void*)0;
    config.fastmem_allow_zero_base = true;

    u8 *stack = new (std::align_val_t(STACK_SIZE)) u8[STACK_SIZE];
    uintptr_t stack_top = (uintptr_t)stack + STACK_SIZE;
    printf("stack: %p, stack_top: %p\n", stack, (u8*)stack_top);

    A64::Jit jit{config};

    jit.SetSP(stack_top);

    u64 result = jit.VMCall<u64>(reinterpret_cast<u64>(add),10,6);

    REQUIRE(result == 16);
}


int fib ( int n ) {
    if ( n <= 2 ) {
        return 1;
    }
    else {
        return fib(n-1) + fib(n-2);
    }
}
/*
 * This tests a recursive function call, source https://godbolt.org/z/Y1deY5h3P

*/
TEST_CASE("A64: Test fibonacci", "[a64]") {
    constexpr auto N = 12;
    A64FastmemOnlyTestEnv test_env(0);
    A64::UserConfig config{ &test_env };
    
    u8 fibonacci[] = {
                                // fib:
        0x1f,0x08,0x00,0x71,    //     cmp     w0, 2
        0x6c,0x00,0x00,0x54,    //     bgt     .L8
        0x20,0x00,0x80,0x52,    //     mov     w0, 1
        0xc0,0x03,0x5f,0xd6,    //     ret
                                // .L8:
        0xfd,0x7b,0xbe,0xa9,    //     stp     x29, x30, [sp, -32]!
        0xfd,0x03,0x00,0x91,    //     mov     x29, sp
        0xf3,0x53,0x01,0xa9,    //     stp     x19, x20, [sp, 16]
        0xf3,0x03,0x00,0x2a,    //     mov     w19, w0
        0x00,0x04,0x00,0x51,    //     sub     w0, w0, #1
        0xf7,0xff,0xff,0x97,    //     bl      fib
        0xf4,0x03,0x00,0x2a,    //     mov     w20, w0
        0x60,0x0a,0x00,0x51,    //     sub     w0, w19, #2
        0xf4,0xff,0xff,0x97,    //     bl      fib
        0x80,0x02,0x00,0x0b,    //     add     w0, w20, w0
        0xf3,0x53,0x41,0xa9,    //     ldp     x19, x20, [sp, 16]
        0xfd,0x7b,0xc2,0xa8,    //     ldp     x29, x30, [sp], 32
        0xc0,0x03,0x5f,0xd6     //     ret   
    };

    // quite a bit of setup, tho it's awesome
    config.fastmem_address_space_bits = 64;
    config.fastmem_pointer = (void*)0;
    config.fastmem_allow_zero_base = true;
    config.enable_cycle_counting = false;


    u8 *stack = new (std::align_val_t(STACK_SIZE)) u8[STACK_SIZE*2];
    uintptr_t stack_top = (uintptr_t)stack + STACK_SIZE*2;
    printf("stack: %p, stack_top: %p\n", stack, (u8*)stack_top);

    A64::Jit jit{config};

    jit.SetSP(stack_top);
    
    u64 result = jit.VMCall<u64>(reinterpret_cast<u64>(fibonacci),N);

    REQUIRE(result == fib(N));

}