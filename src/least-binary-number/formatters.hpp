#pragma once

#include <vector>

#include "spdlog/spdlog.h"

#include "helpers.hpp"

template <>
struct fmt::formatter<std::vector<bool>>
{
    constexpr auto parse(format_parse_context& ctx) {
        // auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) // c++11
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range. In this example
        // the formatter should parse the 'f' specifier and return an iterator
        // pointing to '}'.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const std::vector<bool>& p, FormatContext& ctx) {
        // ctx.out() is an output iterator to write to.
        auto out = ctx.out();
        for (auto b : reverse(p))
        {
            out = format_to(out, "{}", b ? "1" : "0");
        }
        return out;
    }
};

template <typename T>
struct fmt::formatter<std::set<T>>
{
    constexpr auto parse(format_parse_context& ctx) {
        // auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) // c++11
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range. In this example
        // the formatter should parse the 'f' specifier and return an iterator
        // pointing to '}'.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const std::set<T>& p, FormatContext& ctx) {
        // ctx.out() is an output iterator to write to.
        auto out = ctx.out();
        auto size = p.size();
        for (auto t : p)
        {
            out = format_to(out, size != 1 ? "{}, " : "{}", t);
            size -= 1;
        }
        return out;
    }
};

template <typename T>
struct fmt::formatter<std::vector<T>>
{
    constexpr auto parse(format_parse_context& ctx) {
        // auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) // c++11
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:f} - point of interest", point{1, 2});
        //
        // the range will contain "f} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range. In this example
        // the formatter should parse the 'f' specifier and return an iterator
        // pointing to '}'.

        // Parse the presentation format and store it in the formatter:
        auto it = ctx.begin(), end = ctx.end();

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const std::vector<T>& p, FormatContext& ctx) {
        // ctx.out() is an output iterator to write to.
        auto out = ctx.out();
        auto size = p.size();
        for (auto t : p)
        {
            out = format_to(out, size != 1 ? "{}, " : "{}", t);
            size -= 1;
        }
        return out;
    }
};