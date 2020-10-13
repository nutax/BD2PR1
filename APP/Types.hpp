#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "SF.hpp"
#include "EH.hpp"

template<std::size_t size>
struct ComparableCharArray{
  char value[size];
  bool operator < (const ComparableCharArray<size>& other) const {
    return std::string{value} < std::string{other.value};
  }
  bool operator > (const ComparableCharArray<size>& other) const {
    return std::string{value} > std::string{other.value};
  }
  bool operator == (const ComparableCharArray<size>& other) const {
    return std::string{value} == std::string{other.value};
  }

  std::uint32_t operator%(const std::uint32_t& number)const{
    int sum = 0; for (std::size_t i = 0; i <size; i++) sum += value[i];
    return sum%number;
  }
};

template<std::size_t size>
std::ostream& operator<<(std::ostream& os, const ComparableCharArray<size>& data)
{
    os << data.value;
    return os;
}

template<typename T>
struct Numeric{
  T value;
  bool operator < (const Numeric<T>& other) const {
    return value < other.value;
  }
  bool operator > (const Numeric<T>& other) const {
    return value > other.value;
  }
  bool operator == (const Numeric<T>& other) const {
    return value == other.value;
  }

  std::uint32_t operator%(const std::uint32_t& number)const{
    return value%number;
  }
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Numeric<T>& data)
{
    os << data.value;
    return os;
}



template<std::size_t size>
using Text = ComparableCharArray<size>;


using INumber = Numeric<std::int32_t>;
using UINumber = Numeric<std::uint32_t>;
using FNumber = Numeric<float>;
using LINumber = Numeric<std::int64_t>;