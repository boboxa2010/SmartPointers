#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>

template <typename T, size_t Index,
        bool is_compressed_v = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedElement;

template <typename T, size_t Index>
class CompressedElement<T, Index, true> : public T {
public:
    CompressedElement() = default;

    template <typename V>
    CompressedElement(V&& item) : T(std::forward<V>(item)) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <typename T, size_t Index>
class CompressedElement<T, Index, false> {
public:
    CompressedElement() = default;

    template <typename V>
    CompressedElement(V&& item) : item_(std::forward<V>(item)) {
    }

    T& Get() {
        return item_;
    }

    const T& Get() const {
        return item_;
    }

private:
    T item_ = T();
};
template <typename F, typename S>
class CompressedPair : public CompressedElement<F, 0>, public CompressedElement<S, 1> {
public:
    CompressedPair() = default;

    template <typename T1, typename T2>
    CompressedPair(T1&& first, T2&& second)
            : CompressedElement<F, 0>(std::forward<T1>(first)),
              CompressedElement<S, 1>(std::forward<T2>(second)) {
    }

    F& GetFirst() {
        return CompressedElement<F, 0>::Get();
    }

    S& GetSecond() {
        return CompressedElement<S, 1>::Get();
    }

    const F& GetFirst() const {
        return CompressedElement<F, 0>::Get();
    }

    const S& GetSecond() const {
        return CompressedElement<S, 1>::Get();
    }
};
