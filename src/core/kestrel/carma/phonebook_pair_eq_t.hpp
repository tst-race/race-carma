//
// Copyright (C) Steaeqh Software Technologies, Inc.
//
// For the complete copyright information, please see the
// associated README file.
//

#ifndef KESTREL_CARMA_PHONEBOOK_PAIR_EQ_T_HPP
#define KESTREL_CARMA_PHONEBOOK_PAIR_EQ_T_HPP

#include <sst/catalog/SST_ASSERT.h>

#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {
namespace carma {

class phonebook_pair_eq_t final {

public:

  using is_transparent = void;

  bool operator()(phonebook_pair_t const * const & a,
                  psn_t const & b) const {
    SST_ASSERT((a));
    return a->first == b;
  }

  bool operator()(psn_t const & a,
                  phonebook_pair_t const * const & b) const {
    SST_ASSERT((b));
    return a == b->first;
  }

  bool operator()(phonebook_pair_t const * const & a,
                  phonebook_pair_t const * const & b) const {
    SST_ASSERT((a));
    SST_ASSERT((b));
    return a->first == b->first;
  }

}; //

} // namespace carma
} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_PHONEBOOK_PAIR_EQ_T_HPP
