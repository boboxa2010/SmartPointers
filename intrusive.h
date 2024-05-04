#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }

    size_t DecRef() {
        --count_;
        return count_;
    }

    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    RefCounted() = default;

    RefCounted& operator=(const RefCounted& other) {
        return *this;
    }

    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    IntrusivePtr() : ptr_(nullptr) {
    }

    IntrusivePtr(std::nullptr_t) : IntrusivePtr() {
    }

    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.Get()) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.Get()) {
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.Get()) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) : ptr_(other.Get()) {
        other.ptr_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // operator=-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        IntrusivePtr(other).Swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        IntrusivePtr(std::move(other)).Swap(*this);
        return *this;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~IntrusivePtr() {
        if (!ptr_) {
            return;
        }
        ptr_->DecRef();
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Reset() {
        IntrusivePtr().Swap(*this);
    }

    void Reset(T* ptr) {
        IntrusivePtr(ptr).Swap(*this);
    }

    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return ptr_;
    }

    T& operator*() const {
        return *Get();
    }

    T* operator->() const {
        return Get();
    }

    size_t UseCount() const {
        if (!ptr_) {
            return 0;
        }
        return ptr_->RefCount();
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

    template <typename X, typename... Args>
    friend IntrusivePtr<X> MakeIntrusive(Args&&... args);

private:
    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> intrusive_ptr;
    T* ptr = new T(std::forward<Args>(args)...);
    intrusive_ptr.ptr_ = ptr;
    ptr->IncRef();
    return intrusive_ptr;
}