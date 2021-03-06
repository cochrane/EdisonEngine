#pragma once

#include "serialization/serialization.h"

#include <bitset>

namespace serialization
{
template<size_t N>
void save(std::bitset<N>& data, const Serializer& ser)
{
  ser.tag("bitset");
  std::string tmp;
  tmp.reserve(N);
  for(size_t i = 0; i < N; ++i)
  {
    tmp += data.test(i) ? '1' : '0';
  }
  ser.node << tmp;
}

template<size_t N>
void load(std::bitset<N>& data, const Serializer& ser)
{
  ser.tag("bitset");
  data.reset();
  std::string tmp;
  ser.node >> tmp;
  Expects(tmp.length() == N);
  for(size_t i = 0; i < N; ++i)
  {
    Expects(tmp[i] == '0' || tmp[i] == '1');
    if(tmp[i] == '1')
      data.set(i);
  }
}
} // namespace serialization
