#pragma once

#include <concepts>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ut/logger.h"

namespace cli {

[[nodiscard]] static inline bool is_true(std::string_view value) {
  return value == "true" || value == "on" || value == "1" || value == "yes";
}

/* Error types */
struct Option_error : public std::runtime_error {
  explicit Option_error(const std::string& msg) : std::runtime_error(msg) {}
};

struct Parse_error : public Option_error {
  explicit Parse_error(const std::string& msg) : Option_error(msg) {}
};

struct Validation_error : public Option_error {
  explicit Validation_error(const std::string& msg) : Option_error(msg) {}
};

} // namespace cli

#include "cli/array.h"
#include "cli/map.h"
#include "cli/options.h"
