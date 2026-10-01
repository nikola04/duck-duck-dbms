#pragma once

#include <optional>
#include <string>

namespace duck {

struct ColumnRef {
    ColumnRef(std::string name) : name(std::move(name)) {};
    ColumnRef(std::string qualifier, std::string name) : qualifier(std::move(qualifier)), name(std::move(name)) {};

    std::optional<std::string> qualifier;
    std::string name;
};

struct TableRef {
    TableRef(std::string name) : name(std::move(name)), alias(std::nullopt) {};
    TableRef(std::string name, std::string alias) : name(std::move(name)), alias(std::move(alias)) {};

    std::string name;
    std::optional<std::string> alias;
};

} // namespace duck