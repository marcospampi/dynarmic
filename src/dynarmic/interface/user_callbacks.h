/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
namespace Dynarmic {

using VAddr = std::uint64_t;

template < class Interface >
using UserHookCallbackFn = void (*)(Interface*);

template <class VAddr, class Interface>
struct UserCallback {
    VAddr address;
    std::optional<const char*> name;
    UserHookCallbackFn<Interface> callback;
    bool return_back;
};
//using UserHookCallback = std::tuple<Vaddr, std::optional<std::string>, std::uint64_t(*)(Interface*)>;

}