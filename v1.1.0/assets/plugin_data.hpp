/**
 * \file plugin_data.hpp
 * \brief Automatically generated using poly-scribe-code-gen.
 * \author Max Mustermann dummy@mail.com
 * \copyright
 * Copyright (c) 2025-present Max Mustermann
 * Distributed under the MIT licence.
 */

#pragma once

#include <poly-scribe/poly-scribe.hpp>

// NOLINTBEGIN

namespace plugin_namespace {

    // Forward declarations
    struct PluginBase;
    struct PluginA;
    struct PluginB;
    struct PluginSystem;

    enum class Enumeration {
        value1,
        value2
    };

    using Vector = std::array<double, 3>;

    using PluginBase_t = rfl::TaggedUnion<"type", PluginBase, PluginA, PluginB>;

    struct PluginBase {
        std::string name;
        std::string description;
    };

    struct PluginA {
        // Inherited from PluginBase
        std::optional<int> paramA = 42;
        std::optional<Vector> paramVector;
        std::string name;
        std::string description;
    };

    struct PluginB {
        // Inherited from PluginBase
        std::optional<float> paramB;
        std::optional<Enumeration> paramEnum;
        std::string name;
        std::string description;
    };

    struct PluginSystem {
        std::optional<std::unordered_map<std::string, PluginBase_t>> plugin_map;
    };

}  // namespace plugin_namespace

// NOLINTEND