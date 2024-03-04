//
// Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS
// IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef GRAEFFE_TRANSFORM_HPP
#define GRAEFFE_TRANSFORM_HPP

#include <cstddef>
#include <vector>

#include <sst/catalog/bignum.hpp>

class NumX final {
public:
  NumX(sst::bignum const & modulus, sst::bignum const & a)
      : modulus_(modulus) {
    auto b = a;
    b %= modulus;
    rep.push_back(b);
  }

  NumX(sst::bignum const & modulus)
      : NumX(modulus, sst::bignum::zero()) {
  }

  //TODO: handle the case where input coeffs is empty.
  NumX(sst::bignum const & modulus,
       std::vector<sst::bignum> const & coeffs)
      : modulus_(modulus),
        rep(coeffs) {
  }

  ~NumX() noexcept = default;

  NumX(NumX const & other) = default;

  NumX(NumX && other) noexcept = default;

  NumX & operator=(NumX const & other) {
    if (this == &other) {
      return *this;
    }
    rep = other.rep;
    return *this;
  }

  NumX & operator=(NumX && other) noexcept {
    if (this == &other) {
      return *this;
    }
    rep = std::move(other.rep);
    return *this;
  }

  inline bool operator==(NumX const & b) const {
    return modulus_ == b.GetModulus() && rep == b.rep;
  };

  void normalize();

  sst::bignum const & GetModulus() const {
    return modulus_;
  }

  std::vector<sst::bignum> rep;

protected:
private:
  sst::bignum const modulus_;
};

void SetCoeff(NumX & f, std::size_t i, sst::bignum const & a);

void GetCoeff(sst::bignum & a, NumX const & f, std::size_t i);

inline NumX & operator*=(NumX & x, sst::bignum & b);

inline std::size_t deg(NumX const & f) {
  return f.rep.size() - 1;
}

//Horner's method
inline sst::bignum eval(NumX const & f, sst::bignum const & a);

inline NumX diff(NumX const & a);

void add(NumX & x, NumX const & f, NumX const & g);

inline NumX operator+(NumX const & f, NumX const & g) {
  NumX x(f.GetModulus());
  add(x, f, g);
  return x;
}

void mul(NumX & x, NumX const & f, sst::bignum const & g);
void mul(NumX & x, NumX const & f, NumX const & g);

extern NumX build_from_roots(sst::bignum const & modulus,
                             std::vector<sst::bignum> const & roots);

extern void PlainDiv(NumX & x, NumX const & a, NumX const & b);

extern bool divides(NumX const & a, NumX const & b);

/**
 * Compute (h, hbar) where f(x-tau + e) = h + e*hbar (where e^2 = 0).
 *
 * @param f a NumX, i.e. a polynomial with coefficients in
 * sst::bignum.
 * @param tau an element of sst::bignum.
 * @return a std::pair of polynomials (h, hbar)
 */
extern std::pair<NumX, NumX>
initial_linear_expansion(NumX const & f, sst::bignum const & tau);

/**
 * Perform an iteration of the tangent Graeffe transform step. This is
 * done by computing (h(x) + e*hbar(x))(h(-x) + e*hbar(-x)) =
 * h(x)h(-x) + e*(h(x)hbar(-x) + h(-x)hbar(-x)), and halving the
 * degree of these two terms by replacing x^2 with x.
 *
 * @param h a polynomial with coefficients in sst::bignum.
 * @param hbar a polynomial with coefficients in sst::bignum,
 * corresponding to the constant term in the tangent space parameter e.
 *
 * @return nothing, h and hbar are modified in place.
 */
extern void update_linear_expansion(NumX & h, NumX & hbar);

/**
 * Perform the tangent Graeffe transform h(x) + e*hbar(x) =
 * G_rho(f(x-tau + e)) of order rho, where tau is a shift by an
 * element of sst::bignum and e is a tangent number.
 *
 * @param f a polynomial with coefficients in sst::bignum.
 * @param rho a power of 2, the log of which indicates the number of
 * times the transformation will be performed.
 * @param tau an element of sst::bignum, used as a shift by a random
 * element.
 *
 * @return a std::pair of polynomials (h, hbar).
 */
extern std::pair<NumX, NumX>
tangent_graeffe_transform(NumX const & f,
                          sst::bignum const & rho,
                          sst::bignum const & tau);

/**
 * Find the roots of a polynomial with coefficients in sst::bignum,
 * where p = M*2^m + 1, using the tangent Graeffe transform. This is
 * an implementation of algorithm 11 in 'Randomized root finding over
 * finite fields using tangent Graeffe transforms. 2015', by Grenet,
 * van der Hoeven, and Lecerf.
 *
 * @param f a polynomial with coefficients in sst::bignum.
 * @param zeta a primitive root of unity for sst::bignum.
 * @param M a parameter of the prime p.
 * @param m a parameter of the prime p.
 *
 * @return the roots of f, as a std:vector<sst::bignum>.
 */
extern std::vector<sst::bignum> find_roots(NumX const & f,
                                           sst::bignum const & zeta,
                                           long const m,
                                           long const M);

/**
 * Given a collection of values x_j, j=1..degree, construct the newton
 * sums, that is the values Sum (x_j)^i mod p, for i = 1..degree.
 *
 * @param output a vector of values in sst::bignum and size "degree",
   initialized elsewhere.
 * @param values a vector of values in sst::bignum.
 * @param degree the number of newton sums to compute.
 *
 * @return nothing, output is modified in place to hold the newton
 * sums.
 */
extern void values_to_newton(std::vector<sst::bignum> & output,
                             std::vector<sst::bignum> const & values,
                             std::size_t const degree,
                             sst::bignum const & modulus);

/**
 * Given a collection of newton sums, construct a polynomial
 * whose roots are the underlying values, by transforming the
 * newton sums into elementary symmetric polynomials via newton
 * identities.
 *
 * @param ouput a polynomial with coefficients in sst::bignum and degree
   "degree", already initialized.
 * @param newton_sums a vector of values in sst::bignum. The ith value
   is equal to the sum of (x_j)^i mod p.
 * @param degree the degree of the polynomial.
 *
 * @return nothing, output is modified in place to hold the desired
 * polynomial.
 */
extern void
newton_to_polynomial(NumX & output,
                     std::vector<sst::bignum> const & newton_sums,
                     std::size_t const degree);

#endif /* GRAEFFE_TRANSFORM_HPP */
