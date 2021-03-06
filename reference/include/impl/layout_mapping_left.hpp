///////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2015-2016 Bryce Adelstein Lelbach aka wash
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#if !defined(STD_0F383687_D414_463B_9C79_74EA4545EEB5)
#define STD_0F383687_D414_463B_9C79_74EA4545EEB5

#include "impl/fwd.hpp"
#include "impl/dimensions.hpp"

#warning Add and use an embedded dimensions typedef

#warning Better coverage for striding() and padding()

namespace std { namespace experimental
{

template <
    typename Dimensions
  , typename Striding
  , typename Padding
    >
struct layout_mapping_left : Dimensions
{
    static_assert(
           (Dimensions::rank() == Striding::rank())
        && (Striding::rank()   == Padding::rank())
      , "The ranks of Dimensions, Striding and Padding are not equal."
    );

    ///////////////////////////////////////////////////////////////////////////
    // TYPES

    using size_type = std::size_t;

    ///////////////////////////////////////////////////////////////////////////
    // CONSTRUCTORS AND ASSIGNMENT OPERATORS

    constexpr layout_mapping_left() noexcept;

    constexpr layout_mapping_left(layout_mapping_left const& b) noexcept = default;
    constexpr layout_mapping_left(layout_mapping_left&& b) noexcept = default;
    layout_mapping_left& operator=(layout_mapping_left const& b) noexcept = default;
    layout_mapping_left& operator=(layout_mapping_left&& b) noexcept = default;

    template <typename... DynamicDims>
    constexpr layout_mapping_left(DynamicDims... ddims) noexcept;

    constexpr layout_mapping_left(
        Dimensions d
        ) noexcept;

    constexpr layout_mapping_left(
        Dimensions d, Striding striding, Padding pad
        ) noexcept;

    ///////////////////////////////////////////////////////////////////////////
    // DOMAIN SPACE

    static constexpr bool is_regular() noexcept;

    constexpr size_type stride(size_type rank) noexcept;

    constexpr size_type span() const noexcept;

    constexpr Striding striding() const noexcept;

    constexpr Padding padding() const noexcept;

    ///////////////////////////////////////////////////////////////////////////
    // INDEXING

    template <typename... Idx>
    size_type index(Idx... idx) const noexcept;

  private:
    Striding stride_;
    Padding pad_;
};

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr
layout_mapping_left<Dimensions, Striding, Padding>::layout_mapping_left() noexcept
  : Dimensions()
  , stride_()
  , pad_()
{}

template <typename Dimensions, typename Striding, typename Padding>
template <typename... DynamicDims>
inline constexpr
layout_mapping_left<Dimensions, Striding, Padding>::layout_mapping_left(
    DynamicDims... ddims
    ) noexcept
  : Dimensions(ddims...)
  , stride_()
  , pad_()
{}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr
layout_mapping_left<Dimensions, Striding, Padding>::layout_mapping_left(
    Dimensions d
    ) noexcept
  : Dimensions(d)
  , stride_()
  , pad_()
{}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr
layout_mapping_left<Dimensions, Striding, Padding>::layout_mapping_left(
    Dimensions d, Striding striding, Padding pad
    ) noexcept
  : Dimensions(d)
  , stride_(striding)
  , pad_(pad)
{}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr bool
layout_mapping_left<Dimensions, Striding, Padding>::is_regular() noexcept
{
    return true;
}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr
typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
layout_mapping_left<Dimensions, Striding, Padding>::stride(
    size_type rank
    ) const noexcept
{
    #warning I think this is wrong, it should be the actual stride, right?
    return stride_[rank];
}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr
typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
layout_mapping_left<Dimensions, Striding, Padding>::span() const noexcept
{
    return detail::dims_ternary_reduction<
        detail::span_by_value
      , detail::multiplies_by_value
      , detail::static_sentinel<1>
      , 0
      , Dimensions::rank()
    >()(*static_cast<Dimensions const*>(this), stride_, pad_);
}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr Striding
layout_mapping_left<Dimensions, Striding, Padding>::striding() const noexcept
{
    return stride_;
}

template <typename Dimensions, typename Striding, typename Padding>
inline constexpr Padding
layout_mapping_left<Dimensions, Striding, Padding>::padding() const noexcept
{
    return pad_;
}

template <typename Dimensions, typename Striding, typename Padding>
template <typename... Idx>
inline typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
layout_mapping_left<Dimensions, Striding, Padding>::index(
    Idx... idx
    ) const noexcept
{
    // TODO: These static asserts need to actually live in array_ref. The
    // first one is particularly important, otherwise it will silently do the
    // wrong thing.
    static_assert(
        Dimensions::rank() == sizeof...(idx)
      , "Incorrect number of indices passed to layout_mapping_left."
    );

    detail::layout_mapping_left_indexer<Dimensions, Striding, Padding, 0> indexer;
    auto i = detail::make_filled_dims_t<Dimensions::rank(), dyn>(idx...);

    return indexer(*static_cast<Dimensions const*>(this), stride_, pad_, i);
}

///////////////////////////////////////////////////////////////////////////////

namespace detail
{

// Recursive column-major layout implementation.
//
// Three initial cases:
// * First index, 1 < rank()
// * First index, 1 == rank()
// * 0 == rank()
//
// The first case (1 < rank()) recurses, with cases:
// * Nth index
// * Final index

// Nth index
template <
    typename Dimensions, typename Striding, typename Padding
  , std::size_t N
  , typename enable
    >
struct layout_mapping_left_indexer
{
    template <std::size_t... IdxDims>
    typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
    operator()(
        Dimensions d
      , Striding stride
      , Padding pad
      , dimensions<IdxDims...> i
        ) const noexcept
    {
        static_assert(
            0                  < N
          , "Dimension index is out of bounds in layout_mapping_left."
        );
        static_assert(
            Dimensions::rank() > N
          , "Dimension index is out of bounds in layout_mapping_left."
        );
        layout_mapping_left_indexer<Dimensions, Striding, Padding, N + 1> const
            next;
        return (d[N - 1] * stride[N - 1] + pad[N - 1])
             * (stride[N] * i[N] + next(d, stride, pad, i));
    }
};

// First index, 1 < rank()
template <
    typename Dimensions
  , typename Striding
  , typename Padding
    >
struct layout_mapping_left_indexer<
    Dimensions
  , Striding
  , Padding
  , 0                                                // First index
  , typename enable_if<1 < Dimensions::rank()>::type // 1 < rank()
>
{
    template <std::size_t... IdxDims>
    typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
    operator()(
        Dimensions d
      , Striding stride
      , Padding pad
      , dimensions<IdxDims...> i
        ) const noexcept
    {
        layout_mapping_left_indexer<Dimensions, Striding, Padding, 1>
            const next;
        return stride[0] * i[0] + next(d, stride, pad, i);
    }
};

// First index, 1 == rank()
template <
    typename Dimensions
  , typename Striding
  , typename Padding
    >
struct layout_mapping_left_indexer<
    Dimensions
  , Striding
  , Padding
  , 0                                                 // First index
  , typename enable_if<1 == Dimensions::rank()>::type // 1 == rank()
>
{
    template <std::size_t... IdxDims>
    typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
    operator()(
        Dimensions d
      , Striding stride
      , Padding pad
      , dimensions<IdxDims...> i
        ) const noexcept
    {
        return stride[0] * i[0];
    }
};

// 0 == rank()
template <
    typename Dimensions
  , typename Striding
  , typename Padding
  , std::size_t N
    >
struct layout_mapping_left_indexer<
    Dimensions
  , Striding
  , Padding
  , N
  , typename enable_if<0 == Dimensions::rank()>::type // 0 == rank()
>
{
    template <std::size_t... IdxDims>
    typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
    operator()(
        Dimensions d
      , Striding stride
      , Padding pad
      , dimensions<IdxDims...> i
        ) const noexcept
    {
        return 0;
    }
};

// Final index
template <
    typename Dimensions
  , typename Striding
  , typename Padding
  , std::size_t N
    >
struct layout_mapping_left_indexer<
    Dimensions
  , Striding
  , Padding
  , N
  , typename enable_if<
        (1 < Dimensions::rank())
     && (N == (Dimensions::rank() - 1)) // Final index
    >::type
>
{
    template <std::size_t... IdxDims>
    typename layout_mapping_left<Dimensions, Striding, Padding>::size_type
    operator()(
        Dimensions d
      , Striding stride
      , Padding pad
      , dimensions<IdxDims...> i
        ) const noexcept
    {
        static_assert(
            0                  < N
          , "Dimension index is out of bounds in layout_mapping_left."
        );
        static_assert(
            Dimensions::rank() > N
          , "Dimension index is out of bounds in layout_mapping_left."
        );
        return (d[N - 1] * stride[N - 1] + pad[N - 1])
             * (stride[N] * i[N]);
    }
};

}}} // std::experimental::detail

#endif // STD_0F383687_D414_463B_9C79_74EA4545EEB5

