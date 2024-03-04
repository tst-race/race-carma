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

#include <cstddef>
#include <utility>
#include <vector>

#include <sst/catalog/bignum.hpp>

#include <kestrel/graeffe_transform.hpp>

using sst::bignum;
using std::pair;
using std::size_t;
using std::vector;

void NumX::normalize() {

  auto n{rep.size() - 1};
  if (n == 0)
    return;
  bignum coeff{};
  GetCoeff(coeff, *this, n);

  while (n > 0 && coeff.is_zero()) {
    n--;
    GetCoeff(coeff, *this, n);
  }
  // TODO off by 1?
  rep.resize(n);
}

NumX diff(NumX const & a) {
  auto const dega{deg(a)};
  if (dega == 0) {
    return {a.GetModulus()};
  } else {
    vector<bignum> coeffs;
    coeffs.reserve(dega);
    for (size_t i = 0; i <= dega - 1; ++i) {
      coeffs.emplace_back(a.rep[i + 1]);
      coeffs[i] = coeffs[i].mul_mod(i + 1, a.GetModulus());
    }
    return {a.GetModulus(), coeffs};
  }
}

inline NumX & operator*=(NumX & x, bignum & b) {
  if (b.is_zero()) {
    x.rep.resize(1);
    SetCoeff(x, 0, bignum::zero());
  } else {
    for (auto & coeff : x.rep) {
      coeff = coeff.mul_mod(b, x.GetModulus());
    }
  }
  return x;
}

void SetCoeff(NumX & f, size_t i, bignum const & a) {
  auto degree{deg(f)};
  if (i > degree) {
    f.rep.resize(i + 1);
  }
  f.rep[i] = a;
}

void GetCoeff(bignum & x, NumX const & f, size_t i) {
  x = i > deg(f) ? 0 : f.rep[i];
}

void mul(NumX & x, NumX const & f, bignum const & a) {

  if (a.is_zero()) {
    x.rep.resize(1);
    SetCoeff(x, 0, bignum::zero());
  } else {
    x.rep.reserve(deg(f) + 1);
    bignum tmp{};
    for (size_t i{}; i <= deg(f); ++i) {
      GetCoeff(tmp, f, i);
      tmp = tmp.mul_mod(a, f.GetModulus());
      SetCoeff(x, i, tmp);
    }
  }
}

void mul(NumX & x, NumX const & f, NumX const & g) {

  auto const degf{deg(f)};
  auto const degg{deg(g)};

  if (degf == 0) {
    mul(x, g, f.rep[0]);
    return;
  }

  if (degf > degg) {
    mul(x, g, f);
    return;
  }

  // now 0 < deg(f) <= deg(g)

  auto const d{degf + degg};

  size_t imin{};
  size_t imax{};
  for (size_t i = 0; i <= d; ++i) {
    imin = i > degg ? i - degg : 0;
    imax = degf > i ? i : degf;

    auto tmp{bignum::zero()};
    bignum tmpf{};
    bignum tmpg{};
    for (size_t j = imin; j <= imax; ++j) {
      GetCoeff(tmpf, f, j);
      GetCoeff(tmpg, g, i - j);
      tmpf = tmpf.mul_mod(tmpg, f.GetModulus());
      tmp = tmp.add_mod(tmpf, f.GetModulus());
    }
    SetCoeff(x, i, tmp);
  }
  return;
}

void add(NumX & x, NumX const & f, NumX const & g) {
  auto const dega{deg(f)};
  auto const degb{deg(g)};

  if (dega >= degb) {
    x.rep.resize(dega);
    bignum x_coeff{};
    bignum f_coeff{};
    bignum g_coeff{};
    for (size_t i{}; i <= dega; ++i) {
      GetCoeff(f_coeff, f, i);

      x_coeff = f_coeff;
      if (i <= degb) {
        GetCoeff(g_coeff, g, i);
        x_coeff = x_coeff.add_mod(g_coeff, f.GetModulus());
      }
      SetCoeff(x, i, x_coeff);
    }
    return;
  } else {
    add(x, g, f);
  }
}

bignum eval(NumX const & f, bignum const & a) {
  if (deg(f) == 0) {
    return f.rep[0];
  }
  bignum res{};
  bignum f_coeff{};

  GetCoeff(res, f, deg(f));

  for (size_t i{deg(f)}; i--;) {
    res = res.mul_mod(a, f.GetModulus());
    GetCoeff(f_coeff, f, i);
    res = res.add_mod(f_coeff, f.GetModulus());
  }
  return res;
}

void PlainDiv(NumX & x, NumX const & a, NumX const & b) {
  auto const dega{deg(a)};
  auto const degb{deg(b)};
  auto const prime = a.GetModulus();

  if (dega < degb) {
    x.rep.resize(1);
    SetCoeff(x, 0, bignum::zero());
    return;
  }

  if (degb == 0) {
    x = a;
    bignum tmp;
    for (size_t i{}; i <= deg(a); ++i) {
      GetCoeff(tmp, x, i);
      tmp = tmp.mul_mod(b.rep.front(), prime);
      SetCoeff(x, i, tmp);
    }
    return;
  }

  // TODO
}

bool divides(NumX & q, NumX const & a, NumX const & b) {
  auto const dega{deg(a)};
  auto const degb{deg(b)};
  auto const prime = a.GetModulus();

  if (dega < degb) {
    return false;
  }

  if (degb == 0) {
    if (!(bignum::is_zero(b.rep[degb]))) {
      // TODO: is this a copy assignment?
      q = a;
      // do we care if this is monic?
      return true;
    }
    return false;
  }

  bool LCIsOne{true};
  bignum LCInv{1};

  if (b.rep[degb] == bignum(1))
    LCIsOne = true;
  else {
    LCIsOne = false;
    LCInv = inv_mod(b.rep[degb], prime);
  }

  vector<bignum> working_remainder(dega + 1);
  q.rep.resize(dega - degb + 1);

  for (size_t i = 0; i < dega + 1; i++) {
    GetCoeff(working_remainder[i], a, i);
  }

  for (long i = dega - degb; i >= 0; i--) {
    auto coeff_quotient = working_remainder[i + degb];
    coeff_quotient = coeff_quotient.mul_mod(LCInv, prime);
    SetCoeff(q, i, coeff_quotient);
    for (long j = i + degb; j >= i; j--) {
      working_remainder[j] = working_remainder[j].add_mod(
          prime - mul_mod(b.rep[j - i], coeff_quotient, prime),
          prime);
    }
  }

  for (size_t i = 0; i < degb; i++) {
    if (!(bignum::is_zero(working_remainder[i]))) {
      return false;
    }
  }
  return true;
}

pair<NumX, NumX> initial_linear_expansion(NumX const & f,
                                          bignum const & tau) {
  auto const & modulus{f.GetModulus()};
  auto const degree{deg(f)};
  auto h{NumX(modulus)};
  auto const tau_neg{modulus - tau};

  SetCoeff(h, 0L, eval(f, tau_neg));

  auto hbar{NumX(modulus)};

  bignum factorial{1};
  auto poly_derived{diff(f)};
  for (size_t i{1}; i <= degree; ++i) {
    // factorial = (i-1)!
    auto const taylor_eval{eval(poly_derived, tau_neg)};

    // hbar[i] = f^{(i-1)}(-\tau) / (i-1)!

    auto factorialinv {inv_mod(factorial, modulus)};
    SetCoeff(hbar, i - 1L, mul_mod(taylor_eval, factorialinv, modulus));

    factorial = bignum::mul_mod(factorial,
                                bignum(i),
                                modulus); // now factorial = i!

    factorialinv = inv_mod(factorial, modulus);
    // h[i] = f^{(i)}(-\tau) / i!
    SetCoeff(h, i, mul_mod(taylor_eval, factorialinv, modulus));

    // take next derivative
    poly_derived = diff(poly_derived);
  }

  return {h, hbar};
}

void update_linear_expansion(NumX & h, NumX & hbar) {
  //hneg := h(-x)

  //std::cout << "update_linear_expansion\n";

  bignum tmp_coeff{};
  auto const hneg = [&h, &tmp_coeff] {
    NumX tmp{h.GetModulus()};
    for (size_t i{}; i <= deg(h); ++i) {
      GetCoeff(tmp_coeff, h, i);
      SetCoeff(tmp,
               i,
               ((i % 2L) == 0) ? tmp_coeff :
                                 h.GetModulus() - tmp_coeff);
    }
    return tmp;
  }();

  //  hbarneg := hbar(-x)
  //  auto const hbarneg([&hbar, &tmp_coeff] {
  NumX hbarneg{hbar.GetModulus()};
  for (size_t i{}; i <= deg(hbar); ++i) {
    GetCoeff(tmp_coeff, hbar, i);
    SetCoeff(hbarneg,
             i,
             ((i % 2L) == 0) ? tmp_coeff :
                               hbar.GetModulus() - tmp_coeff);
  }
  //    return tmp;
  //  }());

  // std::cout << "hneg modulus " << (int64_t)hneg.GetModulus()
  //           << " hbarneg modulus " << (int64_t)hbarneg.GetModulus()
  //           << "\n";

  auto const a([&h, &hneg] {
    NumX tmp{h.GetModulus()};
    mul(tmp, h, hneg);
    return tmp;
  }());

  auto const b = [&h, &hneg, &hbar, &hbarneg] {
    NumX b1{h.GetModulus()};
    NumX b2{h.GetModulus()};
    mul(b1, h, hbarneg);
    mul(b2, hbar, hneg);
    return b1 + b2;
  }();

  // a and b are even polynomials, and thus all of the odd
  // coefficients must be zero. Update h and hbar with the even
  // coefficients from a and b, respectively.
  for (size_t i{}; i <= deg(h); ++i) {
    GetCoeff(tmp_coeff, a, 2L * i);
    SetCoeff(h, i, tmp_coeff);
  }

  for (size_t i{}; i <= deg(hbar); ++i) {
    GetCoeff(tmp_coeff, b, 2L * i);
    SetCoeff(hbar, i, tmp_coeff);
  }
}

pair<NumX, NumX> tangent_graeffe_transform(NumX const & f,
                                           bignum const & rho,
                                           bignum const & tau) {

  auto hs{initial_linear_expansion(f, tau)};
  auto & h{hs.first};
  auto & hbar{hs.second};
  auto rho_tmp{rho};
  while (rho_tmp > 1L) {
    update_linear_expansion(h, hbar);
    rho_tmp = rho_tmp >> 1L;
  }
  return {h, hbar};
}

vector<bignum> find_roots(NumX const & f,
                          bignum const & zeta,
                          long const m,
                          long const M) {
  vector<bignum> Z;
  auto const & modulus{f.GetModulus()};
  if (deg(f) <= 1) {

    // TODO
    // NTL::FindRoots(Z, f);
    if (deg(f) == 1) {
      bignum ret{f.rep[0].inv_mod(modulus)};
      ret = ret.mul_mod(f.rep[1].inv_mod(modulus), modulus);
      Z.emplace_back(ret);
    }
    return Z;
  } else {
    auto const chi{M * 4};
    // TODO

    bignum rho_tmp{1};

    for (size_t i = 0; i < m - 2L; i++) {
      rho_tmp *= bignum(2);
    }

    auto const rho{rho_tmp};
    auto const rho_zz_p{
        bignum::pow_mod(bignum(2), bignum(m - 2), modulus)};
    auto const z{bignum::pow_mod(zeta, rho, modulus)};
    //bignum const rho{NTL::power_ZZ(2L, m - 2L)};
    //bignum const rho_zz_p{NTL::power(ZZ_p(2L), m - 2L)};
    //bignum const z{NTL::power(zeta, rho)};

    auto tau{bignum::rand_range(modulus)};
    auto const hs{tangent_graeffe_transform(f, rho, tau)};

    auto const & h{hs.first};
    auto const & hbar{hs.second};
    auto const hprime{diff(h)};

    bignum y{1};
    for (size_t i{}; i < chi; ++i) {
      //h(y) == 0 and // hbar(y) != 0
      if (eval(h, y).is_zero() && !(eval(hbar, y).is_zero())) {
        Z.push_back((modulus
                     + (mul_mod(mul_mod(mul_mod(rho_zz_p, y, modulus),
                                        eval(hprime, y),
                                        modulus),
                                inv_mod(eval(hbar, y), modulus),
                                modulus)
                        - tau))
                    % modulus);
      }
      y = y.mul_mod(z, modulus);
    }
    if (eval(f, tau).is_zero()) {
      Z.push_back(tau);
    }

    if (Z.size() < deg(f)) {

      auto const f2{build_from_roots(modulus, Z)};
      NumX f3{modulus};
      if (divides(f3, f, f2)) {
        auto Z2{find_roots(f3, zeta, m, M)};
        Z.insert(Z.end(), Z2.begin(), Z2.end());
      } // NOTE: this should always return true; if the polynomial does not divide, we have problems
    }
    return Z;
  }
}

NumX build_from_roots(bignum const & modulus,
                      vector<bignum> const & roots) {
  NumX f{modulus};

  auto const n{roots.size()};

  if (n == 0) {
    f.rep.resize(1);
    SetCoeff(f, 0, bignum(1));
    return f;
  }

  f.rep.resize(n + 1);
  f.rep[0] = 1;
  for (size_t i = 0; i < n; i++) {
    for (size_t j = i + 1; j > 0; j--) {
      f.rep[j] = (f.rep[j]).mul_mod(modulus - roots[i], modulus);
      f.rep[j] = (f.rep[j]).add_mod(f.rep[j - 1], modulus);
    }
    f.rep[0] = (f.rep[0]).mul_mod(modulus - roots[i], modulus);
  }
  //TODO: re-implement with FFTs

  return f;
}

void newton_to_polynomial(NumX & output,
                          vector<bignum> const & newton_sums,
                          size_t const degree) {
  auto const modulus{output.GetModulus()};

  SetCoeff(output, degree, 1);
  SetCoeff(output,
           degree - 1,
           newton_sums[0].mul_mod(modulus - 1, modulus));
  bignum tmp{};
  bignum tmp2{};

  for (long i = degree - 2; i >= 0; i--) {
    //std::cout << "Line 496 i is: " << i << "\n";
    //tmp = bignum::zero();
    tmp = newton_sums[degree - 1 - i];
    for (size_t j{}; j < (degree - 1) - i; j++) {
      GetCoeff(tmp2, output, i + j + 1);
      tmp2 = tmp2.mul_mod(newton_sums[j], modulus);
      tmp = tmp.add_mod(tmp2, modulus);
    }
    //tmp = tmp.inv_mod(modulus);
    tmp = tmp.mul_mod(modulus - 1, modulus);

    tmp2 = degree - i;
    tmp2 = tmp2.inv_mod(modulus);
    tmp = tmp.mul_mod(tmp2, modulus);
    SetCoeff(output, i, tmp); //newton_sums[degree - 1 - i]);
    // TODO
    // output[i] *= NTL::inv(NTL::ZZ_p(degree - i));

    //output[i] = output[i].mul_mod(, modulus);
  }
}

/*

void newton_to_polynomial(NTL::ZZ_pX & output,
                          NTL::vec_ZZ_p const & newton_sums,
                          const long degree) {
  output[degree - 1] = newton_sums[0] * -1;

  for (long i = degree - 2; i >= 0; i--) {
    output[i] = newton_sums[degree - 1 - i];
    for (long j = 0; j < (degree - 1) - i; j++) {
      output[i] += newton_sums[j] * output[i + j + 1];
    }
    output[i] *= (-1);
    output[i] *= NTL::inv(NTL::ZZ_p(degree - i));
  }
}

*/

void values_to_newton(vector<bignum> & output,
                      vector<bignum> const & values,
                      size_t const degree,
                      bignum const & modulus) {
  //vector<bignum> powers_of_roots = std::copy(values, degree);
  vector<bignum> powers_of_roots(degree);
  // TODO: copy?
  //std::copy(values.begin() , values.begin() + degree, powers_of_roots);
  for (size_t i{}; i < degree; i++) {
    powers_of_roots[i] = values[i];
  }

  output[0] = 0;
  for (size_t i{}; i < degree; i++) {
    output[0] = output[0].add_mod(values[i], modulus);
  }

  for (size_t i{1}; i < degree; i++) {
    output[i].set_zero();

    for (size_t j{}; j < degree; j++) {
      powers_of_roots[j] =
          powers_of_roots[j].mul_mod(values[j], modulus);
      output[i] = output[i].add_mod(powers_of_roots[j], modulus);
    }
  }
}

/**


pair<ZZ_pX, ZZ_pX> initial_linear_expansion(ZZ_pX const & f,
                                            ZZ_p const & tau) {

  auto const degree{deg(f)};
  auto h{ZZ_pX()};
  auto const tau_neg{-tau};
  SetCoeff(h, 0L, eval(f, tau_neg));

  auto hbar{ZZ_pX()};

  auto factorial{ZZ_p(1L)};
  auto poly_derived{diff(f)};
  for (long i = 1L; i <= degree; ++i) {
    // factorial = (i-1)!
    auto const taylor_eval{eval(poly_derived, tau_neg)};

    // hbar[i] = f^{(i-1)}(-\tau) / (i-1)!
    SetCoeff(hbar, i - 1L, taylor_eval / factorial);

    factorial *= ZZ_p(i); // now factorial = i!

    // h[i] = f^{(i)}(-\tau) / i!
    SetCoeff(h, i, taylor_eval / factorial);

    // take next derivative
    poly_derived = diff(poly_derived);
  }
  return {h, hbar};
}

void update_linear_expansion(ZZ_pX & h, ZZ_pX & hbar) {

  //hneg := h(-x)
  auto const hneg = [&h] {
    auto tmp = ZZ_pX();
    for (long i = 0L; i <= deg(h); ++i) {
      SetCoeff(tmp, i, ((i % 2L) == 0) ? coeff(h, i) : -coeff(h, i));
    }
    return tmp;
  }();

  //  hbarneg := hbar(-x)
  auto const hbarneg = [&hbar] {
    auto tmp = ZZ_pX();
    for (long i = 0L; i <= deg(hbar); ++i) {
      SetCoeff(tmp,
               i,
               ((i % 2L) == 0) ? coeff(hbar, i) : -coeff(hbar, i));
    }
    return tmp;
  }();

  auto const a = [&h, &hneg] {
    ZZ_pX tmp;
    FFTMul(tmp, h, hneg);
    return tmp;
  }();

  auto const b = [&h, &hneg, &hbar, &hbarneg] {
    ZZ_pX b1;
    ZZ_pX b2;
    FFTMul(b1, h, hbarneg);
    FFTMul(b2, hbar, hneg);
    return b1 + b2;
  }();

  // a and b are even polynomials, and thus all of the odd
  // coefficients must be zero. Update h and hbar with the even
  // coefficients from a and b, respectively.
  for (long i = 0L; i <= deg(h); ++i) {
    SetCoeff(h, i, coeff(a, 2L * i));
  }

  for (long i = 0L; i <= deg(hbar); ++i) {
    SetCoeff(hbar, i, coeff(b, 2L * i));
  }
}

pair<ZZ_pX, ZZ_pX> tangent_graeffe_transform(ZZ_pX const & f,
                                             ZZ const & rho,
                                             ZZ_p const & tau) {

  auto hs{initial_linear_expansion(f, tau)};
  auto & h{hs.first};
  auto & hbar{hs.second};
  auto rho_tmp{rho};
  while (rho_tmp > 1L) {
    update_linear_expansion(h, hbar);
    rho_tmp = rho_tmp >> 1L;
  }
  return {h, hbar};
}

vec_ZZ_p find_roots(ZZ_pX const & f,
                    ZZ_p const & zeta,
                    long const m,
                    long const M) {
  vec_ZZ_p Z;

  if (deg(f) <= 1) {
    NTL::FindRoots(Z, f);
    return Z;
  } else {
    auto const chi{M * 4};
    ZZ const rho{NTL::power_ZZ(2L, m - 2L)};
    ZZ_p const rho_zz_p{NTL::power(ZZ_p(2L), m - 2L)};
    ZZ_p const z{NTL::power(zeta, rho)};



    auto const tau{random_ZZ_p()};
    auto const hs{tangent_graeffe_transform(f, rho, tau)};



    auto const & h{hs.first};
    auto const & hbar{hs.second};
    auto const hprime{diff(h)};

    auto y{ZZ_p(1L)};
    for (long i = 0L; i < chi; ++i) {
      //h(y) == 0 and // hbar(y) != 0
      if ((IsZero(eval(h, y)) == 1L) && (IsZero(eval(hbar, y)) == 0L)) {
        Z.append(((rho_zz_p * y * eval(hprime, y)) / eval(hbar, y))
                 - tau);
      }
      y *= z;
    }
    if (IsZero(eval(f, tau)) == 1L) {
      Z.append(tau);
    }


    if (Z.length() < deg(f)) {
      auto f2{NTL::BuildFromRoots(Z)};
      ZZ_pX f3;
      if (divide(f3, f, f2) == 1L) {
        Z.append(find_roots(f3, zeta, m, M));
      }
    }
    return Z;
  }
}

void newton_to_polynomial(NTL::ZZ_pX & output,
                          NTL::vec_ZZ_p const & newton_sums,
                          const long degree) {
  output[degree - 1] = newton_sums[0] * -1;

  for (long i = degree - 2; i >= 0; i--) {
    output[i] = newton_sums[degree - 1 - i];
    for (long j = 0; j < (degree - 1) - i; j++) {
      output[i] += newton_sums[j] * output[i + j + 1];
    }
    output[i] *= (-1);
    output[i] *= NTL::inv(NTL::ZZ_p(degree - i));
  }
}

void values_to_newton(NTL::vec_ZZ_p & output,
                      NTL::vec_ZZ_p const & values,
                      const long degree) {
  NTL::vec_ZZ_p powers_of_roots = NTL::VectorCopy(values, degree);

  output[0] = 0;
  for (long i = 0; i < degree; i++) {
    output[0] += values[i];
  }

  for (long i = 1; i < degree; i++) {
    output[i] = 0;

    for (long j = 0; j < degree; j++) {
      mul(powers_of_roots[j], powers_of_roots[j], values[j]);
      output[i] += powers_of_roots[j];
    }
  }
}
**/
