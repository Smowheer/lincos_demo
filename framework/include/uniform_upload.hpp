#ifndef UNIFORM_VALUE_HPP
#define UNIFORM_VALUE_HPP

#include <vector>
#include <array>
#include <initializer_list>

#include <glm/gtc/type_precision.hpp>

// support for single value
template<typename T>
void glUniform(int loc, T const& i);
// support for contigous container of values
template<typename T>
void glUniform(int loc, std::vector<T> const& i);

template<typename T, std::size_t N>
void glUniform(int loc, std::array<T, N> const& i);

template<typename T>
void glUniform(int loc, std::initializer_list<T> const& i);
// upload a number of values
void glUniform(int loc, int const* i, unsigned cnt);
void glUniform(int loc, glm::uint const* i, unsigned cnt);
void glUniform(int loc, float const* i, unsigned cnt);
void glUniform(int loc, glm::fvec3 const* i, unsigned cnt);
void glUniform(int loc, glm::fvec4 const* i, unsigned cnt);
void glUniform(int loc, glm::uvec2 const* i, unsigned cnt);
void glUniform(int loc, glm::fmat4 const* i, unsigned cnt);

template<typename T>
void glUniform(int loc, T const& i) {
  glUniform(loc, &i, 1);
}

template<typename T>
void glUniform(int loc, std::vector<T> const& i) {
  glUniform(loc, i.data(), i.size());
}

template<typename T, std::size_t N>
void glUniform(int loc, std::array<T, N> const& i) {
  glUniform(loc, i.data(), N);
}

template<typename T>
void glUniform(int loc, std::initializer_list<T> const& i) {
  glUniform(loc, i.begin(), i.size());
}

// specialisation for bool, uploaded as int 
template<>
void glUniform(int loc, bool const& i);

#endif
