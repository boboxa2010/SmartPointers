#pragma once

#include "sw_fwd.h"  // Forward declaration

template <typename T>
class WeakPtr {
    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    constexpr WeakPtr() noexcept : observed_(nullptr), block_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) noexcept : observed_(other.Lock().Get()), block_(other.block_) {
        AddRef();
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) noexcept
            : observed_(other.Lock().Get()), block_(other.block_) {
        AddRef();
    }

    WeakPtr(const SharedPtr<T>& other) noexcept : observed_(other.observed_), block_(other.block_) {
        AddRef();
    }

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other) noexcept : observed_(other.observed_), block_(other.block_) {
        AddRef();
    }

    WeakPtr(WeakPtr&& other) noexcept : observed_(other.Lock().Get()), block_(other.block_) {
        AddRef();
        other.Reset();
    }

    template <typename Y>
    WeakPtr(WeakPtr&& other) noexcept : observed_(other.Lock().Get()), block_(other.block_) {
        AddRef();
        other.Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~WeakPtr() {
        if (!block_) {
            return;
        }
        block_->DecrementWeak();
        if (block_->GetReferenceCount() == 0 && block_->GetWeakCount() == 0) {
            delete block_;
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Reset() {
        WeakPtr().Swap(*this);
    }

    void Swap(WeakPtr& other) {
        std::swap(observed_, other.observed_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetReferenceCount();
    }

    bool Expired() const {
        if (!block_) {
            return true;
        }
        return block_->GetReferenceCount() == 0;
    }

    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

    template <class Y>
    bool OwnerBefore(const WeakPtr<Y>& other) const noexcept {
        return block_ == other.block_;
    }

    template <class Y>
    bool OwnerBefore(const SharedPtr<Y>& other) const noexcept {
        return block_ == other.block_;
    }

private:
    void AddRef() {
        if (block_) {
            block_->IncrementWeak();
        }
    }

    ControlBlockBase* block_;
    T* observed_;
};