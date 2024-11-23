#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <optional>
#include <concepts>

namespace cli {

using String_to_int_map = std::map<std::string, int>;
using String_to_double_map = std::map<std::string, double>;
using String_to_string_map = std::map<std::string, std::string>;

template<typename T>
concept Parseable = requires(std::string s) {
  { std::stoi(s) } -> std::convertible_to<T>; }
  || std::same_as<T, std::string>
  || std::same_as<T, bool>
  || std::same_as<T, double>
  || std::same_as<T, String_to_int_map::value_type>
  || std::same_as<T, String_to_double_map::value_type>
  || std::same_as<T, String_to_string_map::value_type>;

template<typename T>
requires Parseable<T>
struct Array_value {
  using value_type = T;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  /* Constructors */
  Array_value() = default;
  explicit Array_value(const std::vector<T>& values);
  explicit Array_value(std::vector<T>&& values);

  /* Iterators */
  iterator begin() { return m_values.begin(); }
  iterator end() { return m_values.end(); }
  const_iterator begin() const { return m_values.begin(); }
  const_iterator end() const { return m_values.end(); }
  const_iterator cbegin() const { return m_values.cbegin(); }
  const_iterator cend() const { return m_values.cend(); }

  /* Access */
  const std::vector<T>& values() const { return m_values; }
  std::vector<T>& values() { return m_values; }

  size_t size() const { return m_values.size(); }
  bool empty() const { return m_values.empty(); }

  T& operator[](size_t index) { return m_values[index]; }
  const T& operator[](size_t index) const { return m_values[index]; }

  /* Parsing */
  static std::optional<Array_value<T>> parse(const std::string& input);
  std::string to_string() const;

  bool operator==(const Array_value& other) const = default;

private:
  std::vector<T> m_values;
};

template<typename T>
requires Parseable<T>
Array_value<T>::Array_value(const std::vector<T>& values)
  : m_values(values) {}

template<typename T>
requires Parseable<T>
Array_value<T>::Array_value(std::vector<T>&& values)
  : m_values(std::move(values)) {}

template<typename T>
requires Parseable<T>
std::optional<Array_value<T>> Array_value<T>::parse(const std::string& input) {
  try {
    std::string item;
    Array_value<T> result;
    std::istringstream iss(input);

    while (std::getline(iss, item, ',')) {
      // Trim whitespace
      item.erase(0, item.find_first_not_of(" \t"));
      item.erase(item.find_last_not_of(" \t") + 1);

      if constexpr (std::is_same_v<T, int>) {
        result.m_values.push_back(std::stoi(item));
      } else if constexpr (std::is_same_v<T, double>) {
        result.m_values.push_back(std::stod(item));
      } else if constexpr (std::is_same_v<T, bool>) {
        result.m_values.push_back(item == "true" || item == "1");
      } else if constexpr (std::is_same_v<T, std::string>) {
        result.m_values.push_back(item);
      }
    }
    return result;
  } catch (...) {
    return std::nullopt;
  }
}

template<typename T>
requires Parseable<T>
std::string Array_value<T>::to_string() const {
  std::ostringstream oss;

  for (size_t i = 0; i < m_values.size(); ++i) {
    if (i > 0) {
      oss << ",";
    }
    if constexpr (std::is_same_v<T, std::string>) {
      oss << m_values[i];
    } else {
      oss << std::to_string(m_values[i]);
    }
  }
  return oss.str();
}

} // namespace cli
