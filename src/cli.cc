#include <algorithm>
#include <charconv>
#include <ranges>

#include "cli/cli.h"
#include "libudp/logger.h"

namespace cli {

bool Options::parse(int argc, char* argv[]) {
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
      if (desc.required && !has_value(name)) {
        log_error(std::format("Required option '{}' is missing\n", name));
        return false;
      }
    }

    return true;
  } catch (const std::exception& e) {
    log_error("Parsing arguments: ", e.what());
    return false;
  }
}

void Options::print_help(std::string_view program_name) const {
  log_info("Usage: ", program_name, " [OPTIONS] [ARGUMENTS]\n\n");
  log_info("Options:");

  // Collect and sort option descriptors
  std::vector<const Option_descriptor*> descriptors;
  descriptors.reserve(m_long_names.size());

  for (const auto& [name, desc] : m_long_names) {
    descriptors.push_back(&desc);
  }

  std::ranges::sort(descriptors, [](const auto* a, const auto* b) {
    return a->long_name < b->long_name;
  });

  /* Print each option */
  for (const auto* desc : descriptors) {
    std::string option_str = "  ";

    /* Add short name if available */
    if (!desc->short_name.empty()) {
      option_str += std::format("-{}, ", desc->short_name);
    } else {
      option_str += "    ";
    }

    /* Add long name */
    option_str += std::format("--{}", desc->long_name);

    /* Add required flag */
    if (desc->required) {
      option_str += " (required)";
    }

    /* Add default value if available */
    if (desc->default_value) {
      option_str += std::format(" [default: {}]", *desc->default_value);
    }

    /* Add environment variable if available */
    if (desc->env_var) {
      option_str += std::format(" [env: {}]", *desc->env_var);
    }

    /* Print option with description */
    log_info(std::format("{:<50} {}\n", option_str, desc->description));
  }
}

void Options::clear() {
  m_short_names.clear();
  m_long_names.clear();
  m_values.clear();
  m_validators.clear();
  m_positional_args.clear();
}

void Options::add_validation(const std::string& name, Validation_callback callback) {
  if (m_long_names.contains(name)) {
    m_validators[name] = std::move(callback);
  }
  else {
    throw Option_error(std::format("Cannot add validation for unknown option '{}'", name));
  }
}

bool Options::is_option(std::string_view arg) const {
  return arg.starts_with('-');
}

bool Options::handle_option(const std::string& name, const std::string& value, bool is_short) {
  const auto& names = is_short ? m_short_names : m_long_names;
  auto it = names.find(name);

  if (it == names.end()) {
    if (!m_allow_unrecognized) {
      log_error(std::format("Unknown option: {}\n", name));
      return false;
    }
    return true;
  }

  const auto& desc = it->second;
  const auto& option_name = desc.long_name;

  /* Validate value if validator exists */
  if (!validate_option(option_name, value)) {
    return false;
  }

  try {
    /* Store the value */
    if (!has_value(option_name)) {
	    int result{};

      /* Auto-detect type and store value */
      if (value == "true" || value == "false") {

        m_values[option_name] = (value == "true");

      } else if (value.find(',') != std::string::npos) {

        /* Might be an array or map value */
        if (value.find('=') != std::string::npos) {

          /* Try parsing as map */
          if (auto result = Map_value<std::string, std::string>::parse(value)) {

            m_values[option_name] = *result;

          } else {

            m_values[option_name] = value;
          }

        } else {

          /* Try parsing as array */
          if (auto result = Array_value<std::string>::parse(value)) {
            m_values[option_name] = *result;
          } else {
            m_values[option_name] = value;
          }
        }

      } else if (auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result); ec == std::errc()) {

        m_values[option_name] = std::stoi(value);

      } else if (value.find('.') != std::string::npos) {

        m_values[option_name] = std::stod(value);

      } else {
        m_values[option_name] = value;
      }
    }
    return true;
  } catch (const std::exception& e) {
    log_error(std::format("Invalid value for option '{}': {}\n", name, e.what()));
    return false;
  }
}

std::string Options::get_env_value(const std::string& name) const {
  if (const char* env = std::getenv(name.c_str())) {
    return env;
  } else {
    return {};
  }
}

bool Options::validate_option(const std::string& name, const std::string& value) const {
  if (auto it = m_validators.find(name); it != m_validators.end()) {
    if (!it->second(value)) {
      log_error(std::format("Validation failed for option '{}'\n", name));
      return false;
    }
  }
  return true;
}

const std::vector<std::string>& Options::positional_args() const {
  return m_positional_args;
}

bool Options::has_value(const std::string& name) const {
  return m_values.contains(name);
}

} // namespace cli
