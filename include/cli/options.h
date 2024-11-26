#pragma once

#include "cli/cli.h"

namespace cli {

/* Value type concepts */
template<typename T>
concept Basic_value = std::same_as<T, bool> || std::same_as<T, int> || std::same_as<T, double> || std::same_as<T, std::string>;

template<typename T>
concept Option_value = Basic_value<T> ||
  requires(T) {
    typename T::value_type;
    requires std::same_as<T, Array_value<typename T::array_type>> ||
             std::same_as<T, Map_value<std::string, typename T::mapped_type>>;
  };

using Value_variant = std::variant<
    bool,
    int,
    double,
    std::string,
    Array_value<int>,
    Array_value<bool>,
    Array_value<double>,
    Array_value<std::string>,
    Map_value<std::string, int>,
    Map_value<std::string, bool>,
    Map_value<std::string, double>,
    Map_value<std::string, std::string>
  >;

struct Option_descriptor {
  /* Short option name (e.g., 'v' for -v) */
  std::string m_short_name{};

  /* Long option name (e.g., 'verbose' for --verbose) */
  std::string m_long_name{};

  /* Help text description */
  std::string m_description{};

  /* Whether the option is required */
  bool m_required{};

  /* Default value if none provided */
  std::optional<Value_variant> m_default_value{};

  /* Environment variable name */
  std::optional<std::string> m_env_var{};
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
  inline void add_option(const Option_descriptor& desc);

  /* Parse command line arguments */
  [[nodiscard]] inline bool parse(int argc, char* argv[]);

  /* Get option value with type checking */
  template<Option_value T>
  [[nodiscard]] std::optional<T> get(const std::string& name) const;

  /* Check if option exists */
   [[nodiscard]] inline bool has_value(const std::string& name) const {
    return m_values.contains(name) && m_values.at(name).has_value();
  }

  /* Print help message */
  inline void print_help(std::string_view program_name) const;

  /* Clear all options and values */
  inline void clear();

  /* Validation callback type */
  using Validation_callback = std::function<bool(const Value_variant&)>;

  /* Add validation for an option */
  inline void add_validation(const std::string& name, Validation_callback callback);

  /* Check if value is a boolean */
  [[nodiscard]] static inline bool is_boolean(std::string_view value) {
    return value == "true" || value == "false" || value == "1" || value == "0" || value == "yes" || value == "no" || value == "on" || value == "off";
  }

  /* Get all positional arguments */
  [[nodiscard]] inline const std::vector<std::string>& positional_args() const {
    return m_positional_args;
  }

private:
  [[nodiscard]] inline bool is_option(std::string_view arg) const;

  [[nodiscard]] inline bool handle_option(const std::string& name, const std::string& value, bool is_short = false);

  template<typename T>
  inline void handle_value(const std::string& name, const Value_variant& value);

  [[nodiscard]] inline std::string get_env_value(const std::string& name) const {
    if (const char* env = std::getenv(name.c_str())) {
      return env;
    } else {
      return {};
    }
  }

  [[nodiscard]] inline bool validate_option(const std::string& name, const std::string& value) const {
    if (auto it = m_validators.find(name); it != m_validators.end()) {
      if (!it->second(value)) {
        log_error(std::format("Validation failed for option '{}'", name));
        return false;
      }
    }
    return true;
  }

  void apply_default_values();

private:
  bool m_allow_unrecognized{};
  std::vector<std::string> m_positional_args;
  std::unordered_map<std::string, Option_descriptor> m_short_names;
  std::unordered_map<std::string, Option_descriptor> m_long_names;
  std::unordered_map<std::string, Validation_callback> m_validators;
  std::unordered_map<std::string, std::optional<Value_variant>> m_values;
};

template<Option_value T>
inline void Options::add_option(const Option_descriptor& desc) {
  if (desc.m_short_name.empty() && desc.m_long_name.empty()) {
    throw Option_error("Option must have either short or long name");
  }

  auto default_value = desc.m_default_value;

  Option_descriptor option = desc;

  if (!default_value) {
    option.m_default_value = T{};
  }

  /* Store option descriptors */
  if (!option.m_short_name.empty()) {
    m_short_names[option.m_short_name] = option;
  }

  if (!option.m_long_name.empty()) {
    m_long_names[option.m_long_name] = option;
  }

  /* Check environment variable first */
  if (option.m_env_var) {
    if (auto env_value = get_env_value(*option.m_env_var); !env_value.empty()) {
      if (!default_value) {
        handle_value<T>(option.m_long_name, env_value);
      } else {
        handle_value<T>(option.m_long_name, *default_value);
      }
      return;
    }
  }
  /* Apply default value if provided */
  if (default_value) {
    handle_value<T>(option.m_long_name, *default_value);
  }
}

template<Option_value T>
inline std::optional<T> Options::get(const std::string& name) const {
  if (auto it = m_values.find(name); it != m_values.end()) {
    try {
      if (it->second.has_value()) {
        return std::get<T>(*it->second);
      } else {
        return std::nullopt;
      }
    } catch (const std::bad_variant_access&) {
      return std::nullopt;
    }
  }
  return std::nullopt;
}

template<typename T>
inline void Options::handle_value(const std::string& name, const Value_variant& value) {
  try {
    std::visit([&name, &value, this](auto&& v) -> auto {
      if constexpr (std::same_as<T, bool>) {

        m_values[name] = v;

      } else if constexpr (std::same_as<T, int>) {

        m_values[name] = v;

      } else if constexpr (std::same_as<T, double>) {

        m_values[name] = v;

      } else if constexpr (std::same_as<T, std::string>) {

        m_values[name] = v;

      } else if constexpr (requires { typename T::array_type; }) {

        m_values[name] = v;

      } else if constexpr (requires { typename T::mapped_type; }) {

        m_values[name] = v;

      } else {
        throw Option_error(std::format("Unsupported option type for '{}'", name));
      }

      /* Validate the parsed value if a validator exists */
      if (auto it = m_validators.find(name); it != m_validators.end()) {
        if (!it->second(value)) {
          throw Validation_error(std::format("Validation failed for option '{}'", name));
        }
      }

    }, value);

  } catch (const Parse_error&) {
    /* Re-throw parsing errors */
    throw;
  } catch (const Validation_error&) {
    /* Re-throw validation errors */
    throw;
  } catch (const std::exception& e) {
    throw Parse_error(std::format("Failed to parse option '{}' : {}", name, e.what()));
  }
}

inline bool Options::parse(int argc, char* argv[]) {
  try {
    std::vector<std::string_view> args(argv + 1, argv + argc);

    for (auto it = args.begin(); it != args.end(); ++it) {
      std::string_view arg = *it;

      if (arg.starts_with("--")) {
        /* Long option */
        auto name = std::string(arg.substr(2));
        auto value_pos = name.find('=');

        if (value_pos != std::string::npos) {
          /* Format: --name=value */
          auto value = name.substr(value_pos + 1);
          name = name.substr(0, value_pos);

          if (!handle_option(name, std::string(value))) {
            return false;
          }
        } else {
          /* Format: --name value or --flag */
          if (it + 1 != args.end() && !is_option(*(it + 1))) {
            if (!handle_option(name, std::string(*(++it)))) {
              return false;
            }
          } else {
            if (!handle_option(name, "true")) {
              return false;
            }
          }
        }
      } else if (arg.starts_with('-') && arg.length() > 1) {
        /* Short option */
        auto name = std::string(arg.substr(1));

        if (it + 1 != args.end() && !is_option(*(it + 1))) {
          if (!handle_option(name, std::string(*(++it)), true)) {
            return false;
          }
        } else {
          if (!handle_option(name, "true", true)) {
            return false;
          }
        }
      } else {
        /* Positional argument */
        m_positional_args.push_back(std::string(arg));
      }
    }

    /* Check required options */
    for (const auto& [name, desc] : m_long_names) {
      if (desc.m_required && !has_value(name)) {
        log_error(std::format("Required option '{}' is missing", name));
        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    log_error("Parsing arguments: ", e.what());
    return false;
  }
}

inline void Options::print_help(std::string_view program_name) const {
  log_info("Usage: ", program_name, " [OPTIONS] [ARGUMENTS]");
  log_info("Options:");

  /* Collect and sort option descriptors */
  std::vector<const Option_descriptor*> descriptors;
  descriptors.reserve(m_long_names.size());

  for (const auto& [name, desc] : m_long_names) {
    descriptors.push_back(&desc);
  }

  std::ranges::sort(descriptors, [](const auto* a, const auto* b) {
    return a->m_long_name < b->m_long_name;
  });

  /* Print each option */
  for (const auto* desc : descriptors) {
    std::string option_str = "  ";

    /* Add short name if available */
    if (!desc->m_short_name.empty()) {
      option_str += std::format("-{}, ", desc->m_short_name);
    } else {
      option_str += "    ";
    }

    /* Add long name */
    option_str += std::format("--{}", desc->m_long_name);

    /* Add required flag */
    if (desc->m_required) {
      option_str += " (required)";
    }

    /* We should always have a default valuei, even if it's an empty value
    because that's how we capture the exact type of the option. */
    assert(desc->m_default_value);

    std::visit([&](auto&& v) {  
      using T = std::decay_t<decltype(v)>;

      if constexpr (Basic_value<T>) {
        option_str += std::format(" [default: {}]", v);
      } else if constexpr (requires { typename T::array_type; }) {
        option_str += std::format(" [default: {}]", v.to_string());
      } else if constexpr (requires { typename T::mapped_type; }) {
        option_str += std::format(" [default: {}]", v.to_string());
      } else {
        throw Option_error(std::format("Unsupported option type for '{}'", desc->m_long_name));
      }
    }, *desc->m_default_value);

    /* Add environment variable if available */
    if (desc->m_env_var) {
      option_str += std::format(" [env: {}]", *desc->m_env_var);
    }

    /* Print option with description */
    log_info(std::format("{:<50} {}", option_str, desc->m_description));
  }
}

inline void Options::clear() {
  m_short_names.clear();
  m_long_names.clear();
  m_values.clear();
  m_validators.clear();
  m_positional_args.clear();
}

inline void Options::add_validation(const std::string& name, Validation_callback callback) {
  if (m_long_names.contains(name)) {
    m_validators[name] = std::move(callback);
  } else {
    throw Option_error(std::format("Cannot add validation for unknown option '{}'", name));
  }
}

inline bool Options::is_option(std::string_view arg) const {
  return arg.starts_with('-');
}

inline bool Options::handle_option(const std::string& name, const std::string& value, bool is_short) {
  const auto& names = is_short ? m_short_names : m_long_names;
  auto it = names.find(name);

  if (it == names.end()) {
    if (!m_allow_unrecognized) {
      log_error(std::format("Unknown option: {}", name));
      return false;
    } else {
      return true;
    }
  }

  const auto& desc = it->second;
  const auto& option_name = desc.m_long_name;

  /* Validate value if validator exists */
  if (!validate_option(option_name, value)) {
    return false;
  }

  bool parsed{true};

  try {

    std::visit([&](auto&& v) {
      using T = std::decay_t<decltype(v)>;

      if constexpr (Basic_value<T>) {
        if constexpr (std::same_as<T, bool>) {
          /* Auto-detect type and store value */
          if (is_boolean(value)) {
            m_values[option_name] = is_true(value);
          } else {
            parsed = false;
          }
        } else if constexpr (std::same_as<T, int>) {
          m_values[option_name] = std::stoi(value);
        } else if constexpr (std::same_as<T, double>) {
          m_values[option_name] = std::stod(value);
        } else if constexpr (std::same_as<T, std::string>) {
          m_values[option_name] = value;
        }
      } else {
        auto result = T::parse(value);

        if (result) {
          m_values[option_name] = result;
        } else {
          parsed = false;
        }
      }
    }, *desc.m_default_value);

  } catch (const std::exception& e) {
    log_error(std::format("Invalid value for option '{}'", name, e.what()));
    return false;
  }
  return parsed;
}

} // namespace cli
