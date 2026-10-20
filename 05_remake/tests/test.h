// Minimal test runner for the remake (no external dependency).
// TEST(name) { CHECK(cond); CHECK_EQ(a, b); }
#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace rt {

struct Case {
    const char* name;
    std::function<void()> fn;
};

inline std::vector<Case>& registry()
{
    static std::vector<Case> r;
    return r;
}

inline int& failures()
{
    static int f = 0;
    return f;
}

struct Reg {
    Reg(const char* n, std::function<void()> f) { registry().push_back({n, std::move(f)}); }
};

}  // namespace rt

#define TEST(name)                                   \
    static void name();                              \
    static rt::Reg reg_##name(#name, name);          \
    static void name()

#define CHECK(cond)                                                               \
    do {                                                                          \
        if (!(cond)) {                                                            \
            std::printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
            ++rt::failures();                                                     \
        }                                                                         \
    } while (0)

// Two std::vector<std::uint32_t> snapshots must be equal; on failure print the first differing index and values
// (at most 3 reports per test, so a failing loop does not flood the log).
#define CHECK_SNAP(a, b)                                                                                  \
    do {                                                                                                  \
        const auto& sa_ = (a);                                                                            \
        const auto& sb_ = (b);                                                                            \
        if (!(sa_ == sb_)) {                                                                              \
            static int shown_ = 0;                                                                        \
            if (shown_++ < 3) {                                                                           \
                std::size_t i_ = 0;                                                                       \
                while (i_ < sa_.size() && i_ < sb_.size() && sa_[i_] == sb_[i_]) ++i_;                     \
                std::printf("  FAIL %s:%d: snapshots differ at word %u of %u/%u: original %08x port %08x\n", \
                            __FILE__, __LINE__, static_cast<unsigned>(i_), static_cast<unsigned>(sa_.size()),  \
                            static_cast<unsigned>(sb_.size()), i_ < sa_.size() ? static_cast<unsigned>(sa_[i_]) : 0u, \
                            i_ < sb_.size() ? static_cast<unsigned>(sb_[i_]) : 0u);                        \
            }                                                                                             \
            ++rt::failures();                                                                             \
        }                                                                                                 \
    } while (0)

#define CHECK_EQ(a, b)                                                                            \
    do {                                                                                          \
        auto va_ = (a);                                                                           \
        auto vb_ = (b);                                                                           \
        if (!(va_ == vb_)) {                                                                      \
            std::printf("  FAIL %s:%d: %s == %s (0x%llx vs 0x%llx)\n", __FILE__, __LINE__, #a, #b, \
                        (unsigned long long)va_, (unsigned long long)vb_);                        \
            ++rt::failures();                                                                     \
        }                                                                                         \
    } while (0)
