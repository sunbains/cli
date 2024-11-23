#pragma once

#include "cli/cli.h"

namespace cli {

template<typename K, typename V>
requires std::convertible_to<K, std::string>
struct Map_value {
  using key_type = K;
  using mapped_type = V;
  using value_type = std::pair<const K, V>;
  using iterator = typename std::map<K, V>::iterator;
  using const_iterator = typename std::map<K, V>::const_iterator;

  /* Constructors */
  Map_value() = default;

  explicit Map_value(const std::map<K, V>& values)
    : m_values(values) {}

  explicit Map_value(std::map<K, V>&& values)
    : m_values(std::move(values)) {}

  iterator begin() { return m_values.begin(); }
  iterator end() { return m_values.end(); }
  const_iterator begin() const { return m_values.begin(); }
  const_iterator end() const { return m_values.end(); }
  const_iterator cbegin() const { return m_values.cbegin(); }
  const_iterator cend() const { return m_values.cend(); }

  const std::map<K, V>& values() const { return m_values; }
  std::map<K, V>& values() { return m_values; }
  
  size_t size() const { return m_values.size(); }
  bool empty() const { return m_values.empty(); }
  
  V& operator[](const K& key) { return m_values[key]; }
  const V& at(const K& key) const { return m_values.at(key); }

  bool operator==(const Map_value& other) const = default;

  static std::optional<Map_value<K, V>> parse(const std::string& input) {
    try {
      std::string pair;
      Map_value<K, V> result;
      std::istringstream iss(input);

      while (std::getline(iss, pair, ',')) {
        auto sep_pos = pair.find('=');

        if (sep_pos == std::string::npos) {
          continue;
        }

        auto key = pair.substr(0, sep_pos);
        auto value = pair.substr(sep_pos + 1);

        /* Trim whitespace */
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if constexpr (std::is_same_v<V, int>) {
          result.m_values[key] = std::stoi(value);
        } else if constexpr (std::is_same_v<V, double>) {
          result.m_values[key] = std::stod(value);
        } else if constexpr (std::is_same_v<V, bool>) {
          result.m_values[key] = is_true(value);
        } else if constexpr (std::is_same_v<V, std::string>) {
          result.m_values[key] = value;
        }
      }

      return result;
    } catch (...) {
      return std::nullopt;
    }
  }

  [[nodiscard]] std::string to_string() const {
    std::ostringstream oss;

    bool first{true};
    for (const auto& [key, value] : m_values) {
      if (!first) {
        oss << ",";
      }
      first = false;
      oss << "{" << key << "=" << value << "}";
    }
    return oss.str();
  }

private:
  std::map<K, V> m_values;
};

} // namespace cli