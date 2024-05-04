#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <variant>

class ESFTBase {};

template <typename T>
class SharedPtr {
    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class EnableSharedFromThis;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    constexpr SharedPtr() noexcept : observed_(nullptr), block_(nullptr) {
    }

    constexpr SharedPtr(std::nullptr_t) noexcept : SharedPtr() {
    }

    explicit SharedPtr(T* ptr) noexcept : observed_(ptr), block_(new ControlBlockPointer<T>(ptr)) {
        AddRef();
        if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
            observed_->weak_this_ = *this;
        }
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) noexcept : observed_(ptr), block_(new ControlBlockPointer<Y>(ptr)) {
        AddRef();
        if constexpr (std::is_convertible_v<Y*, ESFTBase*>) {
            observed_->weak_this_ = *this;
        }
    }

    SharedPtr(const SharedPtr& other) noexcept : observed_(other.observed_), block_(other.block_) {
        AddRef();
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) noexcept
            : observed_(other.observed_), block_(other.block_) {
        AddRef();
    }

    SharedPtr(SharedPtr&& other) noexcept : observed_(other.observed_), block_(other.block_) {
        AddRef();
        other.Reset();
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept : observed_(other.observed_), block_(other.block_) {
        AddRef();
        other.Reset();
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) noexcept : observed_(ptr), block_(other.block_) {
        AddRef();
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other, T* ptr) noexcept : observed_(ptr), block_(other.block_) {
        AddRef();
        other.Reset();
    }

    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        observed_ = other.observed_;
        block_ = other.block_;
        AddRef();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    SharedPtr& operator=(const SharedPtr& other) noexcept {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~SharedPtr() {
        if (!block_) {
            return;
        }

        block_->Decrement();
        if (block_->GetReferenceCount() == 0 && block_->GetWeakCount() == 0) {
            block_->DeleteObject();
            delete block_;
            return;
        }
        if (block_->GetReferenceCount() == 0) {
            block_->DeleteObject();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Reset() noexcept {
        SharedPtr().Swap(*this);
    }

    void Reset(T* ptr) noexcept {
        SharedPtr<T>(ptr).Swap(*this);
    }

    template <typename Y>
    void Reset(Y* ptr) noexcept {
        SharedPtr(ptr).Swap(*this);
    }

    void Swap(SharedPtr& other) noexcept {
        std::swap(observed_, other.observed_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const noexcept {
        return observed_;
    }

    T& operator*() const noexcept {
        return *Get();
    }

    T* operator->() const noexcept {
        return Get();
    }

    size_t UseCount() const noexcept {
        if (!block_) {
            return 0;
        }
        return block_->GetReferenceCount();
    }

    bool Unique() const noexcept {
        return UseCount() == 1;
    }

    explicit operator bool() const noexcept {
        return block_ != nullptr;
    }

    template <class Y>
    bool OwnerBefore(const SharedPtr<Y>& other) const noexcept {
        return block_ == other.block_;
    }

    template <class Y>
    bool OwnerBefore(const WeakPtr<Y>& other) const noexcept {
        return block_ == other.block_;
    }

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

private:
    void AddRef() {
        if (block_) {
            block_->Increment();
        }
    }
    T* observed_;
    ControlBlockBase* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> shared_ptr;
    auto control_block_holder = new ControlBlockHolder<T>(std::forward<Args>(args)...);
    shared_ptr.block_ = control_block_holder;
    shared_ptr.observed_ = control_block_holder->GetPointer();
    shared_ptr.block_->Increment();
    if constexpr (std::is_convertible_v<T*, ESFTBase*>) {
        shared_ptr.observed_->weak_this_ = shared_ptr;
    }
    return shared_ptr;
}

template <typename T>
class EnableSharedFromThis : public ESFTBase {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<const T>(weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }
    WeakPtr<T> weak_this_;
};