#pragma once

#include <cstdint>

namespace RODOS {

class Interruptable_Uint64_Stm32L4_CortexM4_Impl {
  public:
    Interruptable_Uint64_Stm32L4_CortexM4_Impl() { this->storeImpl(0); }
    Interruptable_Uint64_Stm32L4_CortexM4_Impl(uint64_t value) { this->storeImpl(value); }

  protected:
    inline uint64_t loadImpl() const {
        uint64_t result;
        asm volatile(
          "ldrd %Q[pair], %R[pair], %[addr]"
          : [pair] "=r"(result)
          : [addr] "m"(m_value)
          : "memory");
        return result;
    }

    inline void storeImpl(uint64_t value) {
        asm volatile(
          "strd %Q[pair], %R[pair], %[addr]"
          : [addr] "=m"(m_value)
          : [pair] "r"(value)
          : "memory");
    }

  private:
    volatile uint64_t m_value;
};


class Interruptable_Int64_Stm32L4_CortexM4_Impl {
  public:
    Interruptable_Int64_Stm32L4_CortexM4_Impl() { this->storeImpl(0); }
    Interruptable_Int64_Stm32L4_CortexM4_Impl(int64_t value) { this->storeImpl(value); }

  protected:
    inline int64_t loadImpl() const {
        int64_t result;
        asm volatile(
          "ldrd %Q[pair], %R[pair], %[addr]"
          : [pair] "=r"(result)
          : [addr] "m"(m_value)
          : "memory");
        return result;
    }

    inline void storeImpl(int64_t value) {
        asm volatile(
          "strd %Q[pair], %R[pair], %[addr]"
          : [addr] "=m"(m_value)
          : [pair] "r"(value)
          : "memory");
    }

  private:
    volatile int64_t m_value;
};

using Interruptable_Uint64_Arch_Impl = Interruptable_Uint64_Stm32L4_CortexM4_Impl;
using Interruptable_Int64_Arch_Impl  = Interruptable_Int64_Stm32L4_CortexM4_Impl;

} // namespace RODOS
