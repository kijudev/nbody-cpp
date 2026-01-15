// ==============================================================================
// radix.hpp
// Simple radix sort implementation and helpers for sorting integer keys.
// Provides stable, byte-wise radix sort for random access containers.
// ==============================================================================

#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/assert.hpp"
#include "type.hpp"

namespace nbody::base {
using namespace nbody::base::type;

namespace impl {

// Key by which radix sort sorts elements.
template <typename T>
concept RadixKey = IntT<T> || UintT<T>;

template <typename Iterator, typename KeyExtractor>
using RadixKeyType = std::decay_t<std::invoke_result_t<
    KeyExtractor, typename std::iterator_traits<Iterator>::value_type>>;

// Performs one pass of the radix sort for a specific byte.
template <typename Iterator, typename OutIterator, typename KeyExtractor>
void radix_sort_pass_byte(Iterator source_begin, Iterator source_end,
                          OutIterator dest_begin, USize byte_shift,
                          KeyExtractor key_fn) {
    using Key  = RadixKeyType<Iterator, KeyExtractor>;
    using UKey = std::make_unsigned_t<Key>;

    std::array<USize, 256> counts = {0};
    for (auto it = source_begin; it != source_end; ++it) {
        UKey key = static_cast<UKey>(key_fn(*it));

        if constexpr (std::is_signed_v<Key>) {
            key ^= (static_cast<UKey>(1) << (sizeof(Key) * 8 - 1));
        }

        U8 byte = (key >> byte_shift) & 0xFF;
        counts[byte]++;
    }

    std::array<USize, 256> offsets = {0};
    offsets[0]                     = 0;
    for (USize i = 1; i < 256; ++i) {
        offsets[i] = offsets[i - 1] + counts[i - 1];
    }

    for (auto it = source_begin; it != source_end; ++it) {
        UKey key = static_cast<UKey>(key_fn(*it));

        if constexpr (std::is_signed_v<Key>) {
            key ^= (static_cast<UKey>(1) << (sizeof(Key) * 8 - 1));
        }

        U8 byte                     = (key >> byte_shift) & 0xFF;
        dest_begin[offsets[byte]++] = std::move(*it);
    }
}

}  // namespace impl

// Stable radix sort. Sorts elements by key in ascending order, one byte at a time.
template <typename Iterator, typename KeyExtractor>
void radix_sort(Iterator first, Iterator last, KeyExtractor key_fn) {
    using T   = typename std::iterator_traits<Iterator>::value_type;
    using Key = impl::RadixKeyType<Iterator, KeyExtractor>;

    STATIC_ASSERT(impl::RadixKey<Key>,
                  "Radix sort key must be an integer type");
    STATIC_ASSERT(
        (std::is_base_of_v<
            std::random_access_iterator_tag,
            typename std::iterator_traits<Iterator>::iterator_category>),
        "Radix sort requires random access iterators");

    const USize length = static_cast<USize>(std::distance(first, last));
    if (length < 2) {
        return;
    }

    std::vector<T> buffer(length);
    bool           is_original = true;

    for (USize i = 0; i < sizeof(Key); ++i) {
        USize bit_shift = i * 8;

        if (is_original) {
            impl::radix_sort_pass_byte(first, last, buffer.begin(), bit_shift,
                                       key_fn);
        } else {
            impl::radix_sort_pass_byte(buffer.begin(), buffer.end(), first,
                                       bit_shift, key_fn);
        }

        is_original = !is_original;
    }

    if (!is_original) {
        std::move(buffer.begin(), buffer.end(), first);
    }
}

// Stable radix sort. Sorts elements by key in ascending order, one byte at a time.
template <typename Iterator>
void radix_sort(Iterator first, Iterator last) {
    radix_sort(first, last, [](const auto& val) { return val; });
}

}  // namespace nbody::base
