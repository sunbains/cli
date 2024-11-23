#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>
#include <variant>
#include <functional>
#include <concepts>
#include <format>
#include <ranges>
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include "cli/array.h"
#include "cli/map.h"

namespace cli {

/* Value type concepts */
template<typename T>
concept Basic_value = std::same_as<T, bool> || std::same_as<T, int> || std::same_as<T, double> || std::same_as<T, std::string>;

template<typename T>
concept Option_value = Basic_value<T> ||
  requires(T) {
    typename T::value_type;
    requires std::same_as<T, Array_value<typename T::value_type>> ||
      std::same_as<T, Map_value<std::string, typename T::mapped_type>>;
  };

// Option descriptor with environment variable support
struct Option_descriptor {
  /* Short option name (e.g., 'v' for -v) */
  std::string short_name;

  /* Long option name (e.g., 'verbose' for --verbose) */
  std::string long_name;

  /* Help text description */
  std::string description;

  /* Whether the option is required */
  bool required{};

  /* Default value if none provided */
  std::optional<std::string> default_value;

  /* Environment variable name */
  std::optional<std::string> env_var;
};

// Error types
struct Option_error : public std::runtime_error {
  explicit Option_error(const std::string& msg) : std::runtime_error(msg) {}
};

struct Parse_error : public Option_error {
  explicit Parse_error(const std::string& msg) : Option_error(msg) {}
};

struct Validation_error : public Option_error {
  explicit Validation_error(const std::string& msg) : Option_error(msg) {}
};

struct Options {
  Options() = default;
  ~Options() = default;

  /* Disable copy operations due to internal state */
  Options(const Options&) = delete;
  Options& operator=(const Options&) = delete;

  /* Move operations */
  Options(Options&&) noexcept = default;
  Options& operator=(Options&&) noexcept = default;

  /* Add an option with type deduction */
  template<Option_value T>
  void add_option(const Option_descriptor& desc);

  /* Parse command line arguments */
  bool parse(int argc, char* argv[]);

  /* Get option value with type checking */
  template<Option_value T>
  std::optional<T> get(const std::string& name) const;

  /* Check if option exists */
  bool has_value(const std::string& name) const;

  /* Get all positional arguments */
  const std::vector<std::string>& positional_args() const;

  /* Print help message */
  void print_help(std::string_view program_name) const;

  /* Clear all options and values */
  void clear();

  /* Validation callback type */
  using Validation_callback = std::function<bool(const std::string&)>;

  /* Add validation for an option */
  void add_validation(const std::string& name, Validation_callback callback);

private:
  bool is_option(std::string_view arg) const;
  bool handle_option(const std::string& name, const std::string& value, bool is_short = false);

  template<typename T>
  void handle_value(const std::string& name, const std::string& value);

  std::string get_env_value(const std::string& name) const;
  bool validate_option(const std::string& name, const std::string& value) const;
  void apply_default_values();

private:
  using Value_variant = std::variant<
    bool,
    int,
    double,
    std::string,
    Array_value<bool>,
    Array_value<int>,
    Array_value<double>,
    Array_value<std::string>,
    Map_value<std::string, bool>,
    Map_value<std::string, int>,
    Map_value<std::string, double>,
    Map_value<std::string, std::string>
  >;

  bool m_allow_unrecognized{};
  std::unordered_map<std::string, Option_descriptor> m_short_names;
  std::unordered_map<std::string, Option_descriptor> m_long_names;
  std::unordered_map<std::string, Value_variant> m_values;
  std::unordered_map<std::string, Validation_callback> m_validators;
  std::vector<std::string> m_positional_args;
};

/* Implementation of template methods */
template<Option_value T>
void Options::add_option(const Option_descriptor& desc) {
  if (desc.short_name.empty() && desc.long_name.empty()) {
    throw Option_error("Option must have either short or long name");
  }

  /* Store option descriptors */
  if (!desc.short_name.empty()) {
    m_short_names[desc.short_name] = desc;
  }

  if (!desc.long_name.empty()) {
    m_long_names[desc.long_name] = desc;
  }

  /* Check environment variable first */
  if (desc.env_var) {
    if (auto env_value = get_env_value(*desc.env_var); !env_value.empty()) {
      handle_value<T>(desc.long_name, env_value);
      return;
    }
  }

  /* Apply default value if provided */
  if (desc.default_value) {
    handle_value<T>(desc.long_name, *desc.default_value);
  }
}

template<Option_value T>
std::optional<T> Options::get(const std::string& name) const {
  if (auto it = m_values.find(name); it != m_values.end()) {
    try {
      return std::get<T>(it->second);
    } catch (const std::bad_variant_access&) {
      return std::nullopt;
    }
  }
  return std::nullopt;
}

template<typename T>
void Options::handle_value(const std::string& name, const std::string& value) {
  try {
    if constexpr (std::same_as<T, bool>) {
      /* Handle boolean values */
      std::string lower_value = value;
      std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

      m_values[name] = (lower_value == "true" || lower_value == "1" || lower_value == "yes" || lower_value == "on");
    } else if constexpr (std::same_as<T, int>) {
      /* Handle integer values */
      size_t pos = 0;
      int result = std::stoi(value, &pos);

      if (pos != value.length()) {
        throw Parse_error(std::format("Invalid integer value: '{}'", value));
      }

      m_values[name] = result;

    } else if constexpr (std::same_as<T, double>) {
      /* Handle floating-point values */
      size_t pos = 0;
      double result = std::stod(value, &pos);

      if (pos != value.length()) {
        throw Parse_error(std::format("Invalid floating-point value: '{}'", value));
      }

      m_values[name] = result;

    } else if constexpr (std::same_as<T, std::string>) {
      /* Handle string values directly */
      m_values[name] = value;

    } else if constexpr (requires { typename T::value_type; }) {

      if constexpr (std::same_as<T, Array_value<typename T::value_type>>) {
        /* Handle Array_value types */
        auto result = T::parse(value);

        if (!result) {
          throw Parse_error(std::format("Failed to parse array value: '{}'", value));
        }

        m_values[name] = *result;

      } else if constexpr (std::same_as<T, Map_value<std::string, typename T::mapped_type>>) {
        /* Handle Map_value types */
        auto result = T::parse(value);

        if (!result) {
          throw Parse_error(std::format("Failed to parse map value: '{}'", value));
        }

        m_values[name] = *result;
      } else {
        throw Option_error(std::format("Unsupported option type for '{}'", name));
      }

    } else {
      throw Option_error(std::format("Unsupported option type for '{}'", name));
    }

    /* Validate the parsed value if a validator exists */
    if (auto it = m_validators.find(name); it != m_validators.end()) {
      if (!it->second(value)) {
        throw Validation_error(std::format("Validation failed for option '{}' with value '{}'", name, value));
      }
    }
  } catch (const Parse_error&) {
    /* Re-throw parsing errors */
    throw;
  } catch (const Validation_error&) {
    /* Re-throw validation errors */
    throw;
  } catch (const std::exception& e) {
    throw Parse_error(std::format("Failed to parse option '{}' with value '{}': {}", name, value, e.what()));
  }
}

} // namespace cli
