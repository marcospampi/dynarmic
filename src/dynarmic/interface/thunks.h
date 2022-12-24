/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

#include <cstddef>
#include <vector>
#include <cstdint>
#include <optional>
namespace Dynarmic {


template < class Interface >
using ThunkFn = void (*)(Interface*);

template <class VAddr, class Interface>
struct Thunk {
    VAddr address;
    std::optional<const char*> name;
    ThunkFn<Interface> thunk;
    bool return_back;
};

template <class VAddr, class Interface>
using ThunkVector = std::vector<Thunk<VAddr, Interface>>;

}