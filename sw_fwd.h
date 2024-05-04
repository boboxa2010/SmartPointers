#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ControlBlockBase {
public:
    void Increment() {
        ++reference_count_;
    }

    void Decrement() {
        --reference_count_;
    }

    void IncrementWeak() {
        ++weak_count_;
    }

    void DecrementWeak() {
        --weak_count_;
    }

    size_t GetReferenceCount() const {
        return reference_count_;
    }

    size_t GetWeakCount() const {
        return weak_count_;
    }

    virtual void DeleteObject() = 0;

    virtual ~ControlBlockBase() = default;

private:
    size_t reference_count_ = 0;
    size_t weak_count_ = 0;
};

template <typename T>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer() : ptr_(nullptr) {
    }

    ControlBlockPointer(T* ptr) : ptr_(ptr) {
    }

    void DeleteObject() override {
        delete ptr_;
    }

    ~ControlBlockPointer() override = default;

private:
    T* ptr_ = nullptr;
};

template <typename T>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) {
        ::new (GetPointer()) T(std::forward<Args>(args)...);
    }

    T* GetPointer() {
        return reinterpret_cast<T*>(&storage_);
    }

    void DeleteObject() override {
        GetPointer()->~T();
    }

    ~ControlBlockHolder() override = default;

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};