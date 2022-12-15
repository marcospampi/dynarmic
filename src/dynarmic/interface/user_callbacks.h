/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace Dynarmic {

using VAddr = std::uint64_t;


template <class Interface>
struct UserHookCallback {
    VAddr address;
    std::optional<std::string> name,
    void (*callback) (Interface*);
    bool jump_back;
};
//using UserHookCallback = std::tuple<Vaddr, std::optional<std::string>, std::uint64_t(*)(Interface*)>;

}