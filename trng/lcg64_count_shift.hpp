// Copyright (c) 2000-2022, Heiko Bauke
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution
//
//   * Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#if !(defined TRNG_LCG64_COUNT_SHIFT_HPP)

#define TRNG_LCG64_COUNT_SHIFT_HPP

#include <trng/trng_export.hpp>
#include <trng/cuda.hpp>
#include <trng/limits.hpp>
#include <trng/utility.hpp>
#include <trng/int_types.hpp>
#include <trng/generate_canonical.hpp>
#include <climits>
#include <stdexcept>
#include <ostream>
#include <istream>
#include <ciso646>

namespace trng {

  class lcg64_count_shift {
  public:
    // Uniform random number generator concept
    using result_type = uint64_t;
    TRNG_CUDA_ENABLE
    result_type operator()();

  private:
    static constexpr result_type min_ = 0;
    static constexpr result_type max_ = ~result_type(0);
    static constexpr result_type modulus = 2305843009213693951u;

  public:
    static constexpr result_type min() { return min_; }
    static constexpr result_type max() { return max_; }

  private:
    // compute floor(log_2(x))
    TRNG_CUDA_ENABLE
    static unsigned int log2_floor(result_type x);
    // compute pow(x, n)
    TRNG_CUDA_ENABLE
    static result_type pow(result_type x, result_type n);
    // compute prod(1+a^(2^i), i=0..l-1)
    TRNG_CUDA_ENABLE
    static result_type g(unsigned int l, result_type a);
    // compute sum(a^i, i=0..s-1)
    TRNG_CUDA_ENABLE
    static result_type f(result_type s, result_type a);
    // compute a * b mod m
    TRNG_CUDA_ENABLE
    static result_type mult_modulo(result_type a, result_type b);

  public:
    // Parameter and status classes
    class parameter_type {
      result_type a{0}, b{0}, inc{0};

    public:
      parameter_type() = default;
      explicit parameter_type(result_type a, result_type b, result_type inc)
          : a{a}, b{b}, inc{inc} {}

      friend class lcg64_count_shift;

      // Equality comparable concept
      friend bool operator==(const parameter_type &, const parameter_type &);
      friend bool operator!=(const parameter_type &, const parameter_type &);

      // Streamable concept
      template<typename char_t, typename traits_t>
      friend std::basic_ostream<char_t, traits_t> &operator<<(
          std::basic_ostream<char_t, traits_t> &out, const parameter_type &P) {
        std::ios_base::fmtflags flags(out.flags());
        out.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
        out << '(' << P.a << ' ' << P.b << ' ' << P.inc << ')';
        out.flags(flags);
        return out;
      }

      template<typename char_t, typename traits_t>
      friend std::basic_istream<char_t, traits_t> &operator>>(
          std::basic_istream<char_t, traits_t> &in, parameter_type &P) {
        parameter_type P_new;
        std::ios_base::fmtflags flags(in.flags());
        in.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
        in >> utility::delim('(') >> P_new.a >> utility::delim(' ') >> P_new.b >>
            utility::delim(' ') >> P_new.inc >> utility::delim(')');
        if (in)
          P = P_new;
        in.flags(flags);
        return in;
      }
    };

    class status_type {
      result_type r{0}, count{0};

    public:
      status_type() = default;
      explicit status_type(result_type r, result_type count) : r{r}, count{count} {}

      friend class lcg64_count_shift;

      // Equality comparable concept
      friend bool operator==(const status_type &, const status_type &);
      friend bool operator!=(const status_type &, const status_type &);

      // Streamable concept
      template<typename char_t, typename traits_t>
      friend std::basic_ostream<char_t, traits_t> &operator<<(
          std::basic_ostream<char_t, traits_t> &out, const status_type &S) {
        std::ios_base::fmtflags flags(out.flags());
        out.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
        out << '(' << S.r << ' ' << S.count << ')';
        out.flags(flags);
        return out;
      }

      template<typename char_t, typename traits_t>
      friend std::basic_istream<char_t, traits_t> &operator>>(
          std::basic_istream<char_t, traits_t> &in, status_type &S) {
        status_type S_new;
        std::ios_base::fmtflags flags(in.flags());
        in.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
        in >> utility::delim('(') >> S_new.r >> utility::delim(' ') >> S_new.count >>
            utility::delim(')');
        if (in)
          S = S_new;
        in.flags(flags);
        return in;
      }
    };

    static TRNG4_EXPORT const parameter_type Default;
    static TRNG4_EXPORT const parameter_type LEcuyer1;
    static TRNG4_EXPORT const parameter_type LEcuyer2;
    static TRNG4_EXPORT const parameter_type LEcuyer3;

    // Random number engine concept
    explicit lcg64_count_shift(parameter_type = Default);
    explicit lcg64_count_shift(unsigned long, parameter_type = Default);
    explicit lcg64_count_shift(unsigned long long, parameter_type = Default);

    template<typename gen>
    explicit lcg64_count_shift(gen &g, parameter_type P = Default) : P{P} {
      seed(g);
    }

    void seed();
    void seed(unsigned long);
    template<typename gen>
    void seed(gen &g) {
      // LCG part
      result_type r{0};
      for (int i{0}; i < 2; ++i) {
        r <<= 32u;
        r += g();
      }
      S.r = r;
      // counting part
      r = 0;
      for (int i{0}; i < 2; ++i) {
        r <<= 32u;
        r += g();
      }
      S.count = r % modulus;
    }
    void seed(unsigned long long);

    // Equality comparable concept
    friend bool operator==(const lcg64_count_shift &, const lcg64_count_shift &);
    friend bool operator!=(const lcg64_count_shift &, const lcg64_count_shift &);

    // Streamable concept
    template<typename char_t, typename traits_t>
    friend std::basic_ostream<char_t, traits_t> &operator<<(
        std::basic_ostream<char_t, traits_t> &out, const lcg64_count_shift &R) {
      std::ios_base::fmtflags flags(out.flags());
      out.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
      out << '[' << lcg64_count_shift::name() << ' ' << R.P << ' ' << R.S << ']';
      out.flags(flags);
      return out;
    }

    template<typename char_t, typename traits_t>
    friend std::basic_istream<char_t, traits_t> &operator>>(
        std::basic_istream<char_t, traits_t> &in, lcg64_count_shift &R) {
      lcg64_count_shift::parameter_type P_new;
      lcg64_count_shift::status_type S_new;
      std::ios_base::fmtflags flags(in.flags());
      in.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
      in >> utility::ignore_spaces();
      in >> utility::delim('[') >> utility::delim(lcg64_count_shift::name()) >>
          utility::delim(' ') >> P_new >> utility::delim(' ') >> S_new >> utility::delim(']');
      if (in) {
        R.P = P_new;
        R.S = S_new;
      }
      in.flags(flags);
      return in;
    }

    // Parallel random number generator concept
    TRNG_CUDA_ENABLE
    void split(unsigned int, unsigned int);
    TRNG_CUDA_ENABLE
    void jump2(unsigned int);
    TRNG_CUDA_ENABLE
    void jump(unsigned long long);
    TRNG_CUDA_ENABLE
    void discard(unsigned long long);

    // Other useful methods
    static const char *name();
    TRNG_CUDA_ENABLE
    long operator()(long);

  private:
    parameter_type P;
    status_type S;
    static const char *const name_str;

    TRNG_CUDA_ENABLE
    void backward();
    TRNG_CUDA_ENABLE
    void step();
  };

  // Inline and template methods

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::step() {
    S.r = P.a * S.r + P.b;
    S.count += P.inc;
    if (S.count >= modulus)
      S.count -= modulus;
  }

  TRNG_CUDA_ENABLE
  inline lcg64_count_shift::result_type lcg64_count_shift::operator()() {
    step();
    result_type t{S.r + S.count};
    t ^= (t >> 17u);
    t ^= (t << 31u);
    t ^= (t >> 8u);
    return t;
  }

  TRNG_CUDA_ENABLE
  inline long lcg64_count_shift::operator()(long x) {
    return static_cast<long>(utility::uniformco<double, lcg64_count_shift>(*this) * x);
  }

  // compute floor(log_2(x))
  TRNG_CUDA_ENABLE
  inline unsigned int lcg64_count_shift::log2_floor(lcg64_count_shift::result_type x) {
    unsigned int y{0};
    while (x > 0) {
      x >>= 1u;
      ++y;
    }
    --y;
    return y;
  }

  // compute pow(x, n)
  TRNG_CUDA_ENABLE
  inline lcg64_count_shift::result_type lcg64_count_shift::pow(
      lcg64_count_shift::result_type x, lcg64_count_shift::result_type n) {
    lcg64_count_shift::result_type result{1};
    while (n > 0) {
      if ((n & 1u) > 0)
        result *= x;
      x *= x;
      n >>= 1u;
    }
    return result;
  }

  // compute prod(1+a^(2^i), i=0..l-1)
  TRNG_CUDA_ENABLE
  inline lcg64_count_shift::result_type lcg64_count_shift::g(unsigned int l,
                                                             lcg64_count_shift::result_type a) {
    lcg64_count_shift::result_type p = a, res = 1;
    for (unsigned i{0}; i < l; ++i) {
      res *= 1 + p;
      p *= p;
    }
    return res;
  }

  // compute sum(a^i, i=0..s-1)
  TRNG_CUDA_ENABLE
  inline lcg64_count_shift::result_type lcg64_count_shift::f(lcg64_count_shift::result_type s,
                                                             lcg64_count_shift::result_type a) {
    if (s == 0)
      return 0;
    const unsigned int e{log2_floor(s)};
    lcg64_count_shift::result_type y{0}, p{a};
    for (unsigned int l{0}; l <= e; ++l) {
      if (((1ull << l) & s) > 0) {
        y = g(l, a) + p * y;
      }
      p *= p;
    }
    return y;
  }

  // compute a * b mod m
  TRNG_CUDA_ENABLE
  inline lcg64_count_shift::result_type lcg64_count_shift::mult_modulo(
      lcg64_count_shift::result_type a, lcg64_count_shift::result_type b) {
    if (a > b)
      std::swap(a, b);
    lcg64_count_shift::result_type result{0}, x{b};
    while (a > 0) {
      if ((a & 1u) == 1u) {
        result += x;
        if (result >= modulus)
          result -= modulus;
      }
      x += x;
      if (x >= modulus)
        x -= modulus;
      a >>= 1u;
    }
    return result;
  }

  // Parallel random number generator concept

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::split(unsigned int s, unsigned int n) {
#if !(defined __CUDA_ARCH__)
    if (s < 1 or n >= s)
      utility::throw_this(
          std::invalid_argument("invalid argument for trng::lcg64_count_shift::split"));
#endif
    if (s > 1) {
      // LCG part
      jump(n + 1ull);
      P.b *= f(s, P.a);
      P.a = pow(P.a, s);
      // counting part
      P.inc = mult_modulo(s, P.inc);
      backward();
    }
  }

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::jump2(unsigned int s) {
    // LCG part
    S.r = S.r * pow(P.a, 1ull << s) + f(1ull << s, P.a) * P.b;
    // counting part
    result_type powers_of_2{1};
    for (unsigned int i{1}; i <= s; ++i)
      powers_of_2 = mult_modulo(2, powers_of_2);
    S.count += mult_modulo(powers_of_2, P.inc);
    if (S.count >= modulus)
      S.count -= modulus;
  }

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::jump(const unsigned long long s) {
    if (s < 16) {
      for (unsigned int i{0}; i < s; ++i)
        step();
    } else {
      // LCG part
      unsigned int i{0};
      unsigned long long s_l{s};
      while (s_l > 0) {
        if (s_l % 2 == 1)
          S.r = S.r * pow(P.a, 1ull << i) + f(1ull << i, P.a) * P.b;  // jump2(i)
        ++i;
        s_l >>= 1u;
      }
      // counting part
      S.count += mult_modulo(P.inc, s);
      if (S.count >= modulus)
        S.count -= modulus;
    }
  }

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::discard(unsigned long long n) { jump(n); }

  TRNG_CUDA_ENABLE
  inline void lcg64_count_shift::backward() {
    for (unsigned int i{0}; i < 64; ++i)
      S.r = S.r * pow(P.a, 1ull << i) + f(1ull << i, P.a) * P.b;
    S.count += modulus - P.inc;
    if (S.count >= modulus)
      S.count -= modulus;
  }

}  // namespace trng

#endif
