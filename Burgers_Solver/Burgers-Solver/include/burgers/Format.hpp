// Format.hpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#pragma once

#include <charconv>
#include <string>

namespace burgers {

// Shortest text that reads back as exactly the same double
// (e.g. "0.3", not "0.29999999999999999"). Lossless and fast.
inline std::string format_double(double v)
{
    char buf[32];
    const auto result = std::to_chars(buf, buf + sizeof buf, v);
    return std::string(buf, result.ptr);
}

} // namespace burgers
