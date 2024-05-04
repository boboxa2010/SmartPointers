#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template<typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;

    template<typename U>
    DefaultDeleter(const DefaultDeleter<U> &other) {
    }

    void operator()(T *ptr) {
        delete ptr;
    }
};

template<typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;

    template<typename U>
    DefaultDeleter(const DefaultDeleter<U> &other) {
    }

    void operator()(T *ptr) {
        delete[] ptr;
    }
};

// Primary template
template<typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T *ptr = nullptr) noexcept: ptr_(ptr, Deleter()) {
    }

    UniquePtr(T *ptr, Deleter deleter) : ptr_(ptr, std::forward<decltype(deleter)>(deleter)) {
    }

    template<typename V, typename D>
    UniquePtr(UniquePtr<V, D> &&other) noexcept
            : ptr_(other.Release(), std::forward<D>(other.GetDeleter())) {
    }

    UniquePtr(std::nullptr_t) : ptr_(nullptr, Deleter()) {
    }

    UniquePtr(const UniquePtr &) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template<typename U, typename E>
    UniquePtr &operator=(UniquePtr<U, E> &&other) noexcept {
        GetDeleter() = std::forward<E>(other.GetDeleter());
        Reset(other.Release());
        return *this;
    }

    UniquePtr &operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr &operator=(const UniquePtr &) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~UniquePtr() {
        if (Get() != nullptr) {
            GetDeleter()(Get());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    T *Release() noexcept {
        T *old = Get();
        ptr_.GetFirst() = nullptr;
        return old;
    }

    void Reset(T *ptr = nullptr) noexcept {
        T *old_ptr = Get();
        ptr_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    void Swap(UniquePtr &other) noexcept {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T *Get() const noexcept {
        return ptr_.GetFirst();
    }

    Deleter &GetDeleter() noexcept {
        return ptr_.GetSecond();
    }

    const Deleter &GetDeleter() const noexcept {
        return ptr_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    std::add_lvalue_reference_t<T> operator*() const noexcept {
        return *Get();
    }

    T *operator->() const noexcept {
        return Get();
    }

private:
    CompressedPair<T *, Deleter> ptr_;
};

// Specialization for arrays
template<typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    explicit UniquePtr(T *ptr = nullptr) noexcept: ptr_(ptr, Deleter()) {
    }

    UniquePtr(T *ptr, Deleter deleter) : ptr_(ptr, std::forward<decltype(deleter)>(deleter)) {
    }

    template<typename V, typename D>
    UniquePtr(UniquePtr<V, D> &&other) noexcept
            : ptr_(other.Release(), std::forward<D>(other.GetDeleter())) {
    }

    UniquePtr(std::nullptr_t) : ptr_(nullptr, Deleter()) {
    }

    UniquePtr(const UniquePtr &) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    template<typename U, typename E>
    UniquePtr &operator=(UniquePtr<U, E> &&other) noexcept {
        GetDeleter() = std::forward<E>(other.GetDeleter());
        Reset(other.Release());
        return *this;
    }

    UniquePtr &operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr &operator=(const UniquePtr &) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~UniquePtr() {
        if (Get() != nullptr) {
            GetDeleter()(Get());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    T *Release() noexcept {
        T *old = Get();
        ptr_.GetFirst() = nullptr;
        return old;
    }

    void Reset(T *ptr = nullptr) noexcept {
        T *old_ptr = Get();
        ptr_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    void Swap(UniquePtr &other) noexcept {
        std::swap(ptr_.GetFirst(), other.ptr_.GetFirst());
        std::swap(ptr_.GetSecond(), other.ptr_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T *Get() const noexcept {
        return ptr_.GetFirst();
    }

    Deleter &GetDeleter() noexcept {
        return ptr_.GetSecond();
    }

    const Deleter &GetDeleter() const noexcept {
        return ptr_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    T &operator[](size_t index) const {
        return Get()[index];
    }

    T &operator[](size_t index) {
        return Get()[index];
    }

private:
    CompressedPair<T*, Deleter> ptr_;
};