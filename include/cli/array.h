#pragma once

#include "cli/cli.h"

namespace cli {

template<typename T>
struct Array_value {
  using array_type = T;
  using value_type = array_type;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  Array_value() = default;

  explicit Array_value(const std::vector<T>& values)
    : m_values(values) {}

  explicit Array_value(std::vector<T>&& values)
    : m_values(std::move(values)) {}

  iterator begin() { return m_values.begin(); }
  iterator end() { return m_values.end(); }
  const_iterator begin() const { return m_values.begin(); }
  const_iterator end() const { return m_values.end(); }
  const_iterator cbegin() const { return m_values.cbegin(); }
  const_iterator cend() const { return m_values.cend(); }

  const std::vector<T>& values() const { return m_values; }
  std::vector<T>& values() { return m_values; }

  size_t size() const { return m_values.size(); }
  bool empty() const { return m_values.empty(); }

  T& operator[](size_t index) { return m_values[index]; }
  const T& operator[](size_t index) const { return m_values[index]; }

  bool operator==(const Array_value& other) const = default;

  static std::optional<Array_value<T>> parse(const std::string& input) {
    try {
      std::string item;
      Array_value<T> result;
      std::istringstream iss(input);

      while (std::getline(iss, item, ',')) {
        /* Trim whitespace */
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);

        if constexpr (std::is_same_v<T, int>) {
          result.m_values.push_back(std::stoi(item));
        } else if constexpr (std::is_same_v<T, double>) {
          result.m_values.push_back(std::stod(item));
        } else if constexpr (std::is_same_v<T, bool>) {
          result.m_values.push_back(is_true(item));
        } else if constexpr (std::is_same_v<T, std::string>) {
          result.m_values.push_back(item);
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
    for (const auto& value : m_values) {
      if (!first) {
        oss << ",";
      }
      first = false;
      oss << value;
    }
    return oss.str();
  }

private:
  std::vector<T> m_values;
};

} // namespace cli
