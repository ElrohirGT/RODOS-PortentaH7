/**
 * @file
 * @date 2023/10/24
 * @author Michael Zehrer
 * @brief custom 64-bit atomic implementation
 */
#ifndef HW_ATOMIC_H
#define HW_ATOMIC_H
// will be included in rodos-atomic.h, iff requested
// => no include of "rodos-atomic.h" here, to prevent circular dependency

#if defined(ATOMIC_VARIANT) && ATOMIC_VARIANT == ATOMIC_VARIANT_STD_FALLBACK_CUSTOM
#include "misc-rodos-funcs.h"
namespace RODOS {
/**
 * @brief simple atomic "uint64_t" specialization
 * @note atomicity is achieved by briefly disabling global interrupts
 * @warning not save on multi-core systems
 */
template<> struct Atomic<uint64_t> {
  public:
    Atomic() = default;
    constexpr Atomic(uint64_t val) noexcept : data(val) { }
    Atomic(const Atomic&) = delete;
    ~Atomic() noexcept = default;

    uint64_t loadFromISR() const noexcept { return this->load(); }
    void storeFromISR(uint64_t val) noexcept { this->store(val); }

    void store(uint64_t val) noexcept {
        hwDisableInterrupts();
        this->data = val;
        hwEnableInterrupts();
    }
    uint64_t load() const noexcept {
        uint64_t tmp;
        {
            hwDisableInterrupts();
            tmp = this->data;
            hwEnableInterrupts();
        }
        return tmp;
    }

    uint64_t fetch_add(uint64_t val) noexcept {
        uint64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data += val;
            hwEnableInterrupts();
        }
        return old;
    }
    uint64_t fetch_sub(uint64_t val) noexcept {
        uint64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data -= val;
            hwEnableInterrupts();
        }
        return old;
    }
    uint64_t fetch_and(uint64_t val) noexcept {
        uint64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data &= val;
            hwEnableInterrupts();
        }
        return old;
    }

    uint64_t fetch_or(uint64_t val) noexcept {
        uint64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data |= val;
            hwEnableInterrupts();
        }
        return old;
    }
    uint64_t fetch_xor(uint64_t val) noexcept {
        uint64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data ^= val;
            hwEnableInterrupts();
        }
        return old;
    }

    bool is_lock_free() const noexcept { return false; }

    Atomic& operator=(const Atomic&) = delete;
    Atomic& operator=(const Atomic&) volatile = delete;

    operator uint64_t()             noexcept { return this->load(); }
    uint64_t operator=(uint64_t v)  noexcept { this->store(v); return v;  }

    uint64_t operator++(int)        noexcept { return this->fetch_add(1); }
    uint64_t operator++()           noexcept { return this->fetch_add(1); }

    uint64_t operator--(int)        noexcept { return this->fetch_sub(1); }
    uint64_t operator--()           noexcept { return this->fetch_sub(1); }

    uint64_t operator+=(uint64_t v) noexcept { return this->fetch_add(v); }
    uint64_t operator-=(uint64_t v) noexcept { return this->fetch_sub(v); }
    uint64_t operator&=(uint64_t v) noexcept { return this->fetch_and(v); }
    uint64_t operator|=(uint64_t v) noexcept { return this->fetch_or(v);  }
    uint64_t operator^=(uint64_t v) noexcept { return this->fetch_xor(v); }

  private:
    uint64_t data;
};

/**
 * @brief simple atomic "int64_t" specialization
 * @note atomicity is achieved by briefly disabling global interrupts
 * @warning not save on multi-core systems
 */
template<> struct Atomic<int64_t> {
  public:
    Atomic() = default;
    constexpr Atomic(int64_t val) noexcept : data(val) { }

    Atomic(const Atomic&) = delete;
    ~Atomic() noexcept = default;

    int64_t loadFromISR() const noexcept { return this->load(); }
    void storeFromISR(int64_t val) noexcept { this->store(val); }

    void store(int64_t val) noexcept {
        hwDisableInterrupts();
        this->data = val;
        hwEnableInterrupts();
    }
    int64_t load() const noexcept {
        int64_t tmp;
        {
            hwDisableInterrupts();
            tmp = this->data;
            hwEnableInterrupts();
        }
        return tmp;
    }

    int64_t fetch_add(int64_t val) noexcept {
        int64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data += val;
            hwEnableInterrupts();
        }
        return old;
    }
    int64_t fetch_sub(int64_t val) noexcept {
        int64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data -= val;
            hwEnableInterrupts();
        }
        return old;
    }
    int64_t fetch_and(int64_t val) noexcept {
        int64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data &= val;
            hwEnableInterrupts();
        }
        return old;
    }

    int64_t fetch_or(int64_t val) noexcept {
        int64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data |= val;
            hwEnableInterrupts();
        }
        return old;
    }
    int64_t fetch_xor(int64_t val) noexcept {
        int64_t old;
        {
            hwDisableInterrupts();
            old = this->data;
            this->data ^= val;
            hwEnableInterrupts();
        }
        return old;
    }

    bool is_lock_free() const noexcept { return false; }

    Atomic& operator=(const Atomic&) = delete;
    Atomic& operator=(const Atomic&) volatile = delete;

    operator int64_t()            noexcept { return this->load(); }
    int64_t operator=(int64_t v)  noexcept { this->store(v); return v;  }

    int64_t operator++(int)       noexcept { return this->fetch_add(1); }
    int64_t operator++()          noexcept { return this->fetch_add(1); }

    int64_t operator--(int)       noexcept { return this->fetch_sub(1); }
    int64_t operator--()          noexcept { return this->fetch_sub(1); }

    int64_t operator+=(int64_t v) noexcept { return this->fetch_add(v); }
    int64_t operator-=(int64_t v) noexcept { return this->fetch_sub(v); }
    int64_t operator&=(int64_t v) noexcept { return this->fetch_and(v); }
    int64_t operator|=(int64_t v) noexcept { return this->fetch_or(v);  }
    int64_t operator^=(int64_t v) noexcept { return this->fetch_xor(v); }

  private:
    int64_t data;
};



template<> struct AtomicRO<uint64_t> {
  public:
    AtomicRO() = default;
    constexpr AtomicRO(uint64_t val) noexcept : data(val) { }
    AtomicRO(const AtomicRO&) = delete;
    ~AtomicRO() noexcept = default;

    uint64_t load() const noexcept {
        uint64_t tmp;
        __asm__ volatile(
            "ldrd %Q[pair], %R[pair], %[addr]"
            : [pair] "=r"(tmp)
            : [addr] "m"(this->data)
            : "memory");
        return tmp;
    }

    uint64_t loadFromISR() const noexcept { return this->load(); }
    /**
     * @brief store without protection
     * @warning don't use in multiple ISRs (if they have different priorities)
     */
    void storeFromISR(uint64_t val) noexcept { this->data = val; }

    bool is_lock_free() const noexcept { return true; }

    AtomicRO& operator=(const AtomicRO&) = delete;
    AtomicRO& operator=(const AtomicRO&) volatile = delete;

    operator uint64_t() const noexcept { return this->load(); }

  private:
    alignas(8) uint64_t data;
};

template<> struct AtomicRO<int64_t> {
  public:
    AtomicRO() = default;
    constexpr AtomicRO(int64_t val) noexcept : data(val) { }
    AtomicRO(const AtomicRO&) = delete;
    ~AtomicRO() noexcept = default;

    int64_t load() const noexcept {
        int64_t tmp;
        __asm__ volatile(
            "ldrd %Q[pair], %R[pair], %[addr]"
            : [pair] "=r"(tmp)
            : [addr] "m"(this->data)
            : "memory");
        return tmp;
    }

    int64_t loadFromISR() const noexcept { return this->load(); }
    /**
     * @brief store without protection
     * @warning don't use in multiple ISRs (if they have different priorities)
     */
    void storeFromISR(int64_t val) noexcept { this->data = val; }

    bool is_lock_free() const noexcept { return true; }

    AtomicRO& operator=(const AtomicRO&) = delete;
    AtomicRO& operator=(const AtomicRO&) volatile = delete;

    operator int64_t() const noexcept { return this->load(); }

  private:
    alignas(8) int64_t data;
};
} /* namespace RODOS */
#endif /* ATOMIC_VARIANT == ATOMIC_VARIANT_STD_FALLBACK_CUSTOM */

#endif /* HW_ATOMIC_H */
