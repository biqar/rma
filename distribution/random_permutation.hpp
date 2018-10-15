/*
 * random_permutation.hpp
 *
 *  Created on: Oct 27, 2017
 *      Author: dleo@cwi.nl
 */

#ifndef RANDOM_PERMUTATION_HPP_
#define RANDOM_PERMUTATION_HPP_

#include <cinttypes>
#include <cstddef>
#include <memory>
#include <utility>

#include "console_arguments.hpp"
#include "cbytearray.hpp"
#include "cbyteview.hpp"
#include "distribution.hpp" // KeyValue

namespace distribution {

// forward declarations
class RandomPermutationParallel;

/**
 * Common interface
 */
struct RandomPermutation {
    RandomPermutation();
    virtual ~RandomPermutation();
    virtual size_t size() const = 0;
    virtual KeyValue get(size_t index) const = 0;
};


/**
 * Previous manner, very memory consuming
 */
struct RandomPermutationLegacy : RandomPermutation {
    std::unique_ptr<std::pair<int64_t, int64_t>[]> el_ptr;
    const size_t sz;

    RandomPermutationLegacy(const size_t size);
    virtual ~RandomPermutationLegacy();

    virtual size_t size() const;
    virtual KeyValue get(size_t index) const;
};

/******************************************************************************
 *                                                                            *
 *  RandomPermutationCompressed                                               *
 *                                                                            *
 ******************************************************************************/
class RandomPermutationCompressed : public RandomPermutation {
    CByteArray array;
private:
    void do_permutation(uint64_t seed);

public:
    RandomPermutationCompressed(size_t size, uint64_t seed);
    ~RandomPermutationCompressed();

    virtual size_t size() const;
    virtual KeyValue get(size_t index) const;
};


/******************************************************************************
 *                                                                            *
 *  RandomPermutationParallel                                                 *
 *                                                                            *
 ******************************************************************************/
class RandomPermutationParallel : public RandomPermutation {
    std::shared_ptr<CByteArray> container; // the underlying container

public:
    RandomPermutationParallel();
    RandomPermutationParallel(size_t size, uint64_t seed);
    ~RandomPermutationParallel();

    void compute(size_t size, uint64_t seed = ARGREF(uint64_t, "seed_random_permutation"));

    /**
     * The number of elements generated
     */
    virtual size_t size() const;

    // [Old interface] Retrieve the key/value for the given index
    // New approach: access to the underlying container through a view
    virtual KeyValue get(size_t index) const;

    // [Old interface] Retrieve the single value stored in the permutation, in the interval [0, capacity() )
    // New approach: access to the underlying container through a view
    int64_t get_raw_key(size_t index) const;

    // New interface
    std::shared_ptr<CByteArray> get_container() const;
    std::unique_ptr<CByteView> get_view();
    std::unique_ptr<CByteView> get_view(size_t shift);
    std::unique_ptr<CByteView> get_view(size_t start, size_t length);
};

} // namespace distribution

#endif /* RANDOM_PERMUTATION_HPP_ */
