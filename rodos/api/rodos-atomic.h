/**
 * @file
 * @date 2023/10/19
 * @author Michael Zehrer
 * @brief very basic atomic data structure
 * @warning might be unsafe on multi-core systems
 * @warning not save in combination with DMA, MMIO, etc.
 */
#ifndef RODOS_ATOMIC_H
#define RODOS_ATOMIC_H

// use std::atomic
#define ATOMIC_VARIANT_STD 0

// use std::atomic, but fallback to port implementation ("hw_atomic.h")
#define ATOMIC_VARIANT_STD_FALLBACK_CUSTOM 1

// don't use std::atomics at all => use port implementation ("hw_atomic.h")
#define ATOMIC_VARIANT_CUSTOM 2


#ifndef ATOMIC_VARIANT
#define ATOMIC_VARIANT ATOMIC_VARIANT_STD
#endif


#if ATOMIC_VARIANT == ATOMIC_VARIANT_STD || \
    ATOMIC_VARIANT == ATOMIC_VARIANT_STD_FALLBACK_CUSTOM
#include <stdint.h>
#include <atomic>

namespace RODOS {
/**
 * @brief generic atomic data structure
 *
 * can be accessed by arbitrary many threads and / or ISRs
 *
 * @warning only single-core accesses are guaranteed to be atomic
 */
template<typename T> struct Atomic : protected std::atomic<T> {
  public:
    Atomic() = default;
    constexpr Atomic(T val) noexcept : std::atomic<T>(val) { }
    Atomic(const Atomic&) = delete;
    ~Atomic() noexcept = default;

    T load() const noexcept { return std::atomic<T>::load(); }
    void store(T val) noexcept { std::atomic<T>::store(val); }

    T loadFromISR() const noexcept { return this->load(); }
    void storeFromISR(T val) noexcept { this->store(val); }

    bool is_lock_free() const noexcept { return std::atomic<T>::is_lock_free(); }

    Atomic& operator=(const Atomic&) = delete;
    Atomic& operator=(const Atomic&) volatile = delete;

    operator T() const noexcept { return this->load(); }
    T operator=(T v)  noexcept { this->store(v); return v;  }

    T operator++(int) noexcept { return this->fetch_add(1); }
    T operator++()    noexcept { return this->fetch_add(1); }

    T operator--(int) noexcept { return this->fetch_sub(1); }
    T operator--()    noexcept { return this->fetch_sub(1); }

    T operator+=(T v) noexcept { return this->fetch_add(v); }
    T operator-=(T v) noexcept { return this->fetch_sub(v); }
    T operator&=(T v) noexcept { return this->fetch_and(v); }
    T operator|=(T v) noexcept { return this->fetch_or(v);  }
    T operator^=(T v) noexcept { return this->fetch_xor(v); }
};

/**
 * @brief generic atomic data structure (read only)
 *
 * arbitrarily many threads can read (load) the value,
 * but writing (store) is only allowed from a single ISR
 *
 * @warning only single-core accesses are guaranteed to be atomic
 */
template<typename T> struct AtomicRO : protected std::atomic<T> {
  public:
    AtomicRO() = default;
    constexpr AtomicRO(T val) noexcept : std::atomic<T>(val) { }
    AtomicRO(const AtomicRO&) = delete;
    ~AtomicRO() noexcept = default;

    T load() const noexcept { return std::atomic<T>::load(); }

    T loadFromISR() const noexcept { return this->load(); }
    void storeFromISR(T val) noexcept { std::atomic<T>::store(val); }

    bool is_lock_free() const noexcept { return std::atomic<T>::is_lock_free(); }

    AtomicRO& operator=(const AtomicRO&) = delete;
    AtomicRO& operator=(const AtomicRO&) volatile = delete;

    operator T() const noexcept { return this->load(); }
};
} /* namespace RODOS */
#endif /* ATOMIC_VARIANT == ATOMIC_VARIANT_STD{*} */

#if ATOMIC_VARIANT == ATOMIC_VARIANT_CUSTOM || \
    ATOMIC_VARIANT == ATOMIC_VARIANT_STD_FALLBACK_CUSTOM
#include "hw_atomic.h"
#endif

#endif /* RODOS_ATOMIC_H */
