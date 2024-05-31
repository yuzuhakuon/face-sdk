#pragma once
// reference: https://github.com/google/mediapipe/blob/master/mediapipe/framework/deps/aligned_malloc_and_free.h

#include <cstddef>
#include <stdlib.h>

#if defined(__ANDROID__) || defined(_WIN32)
#include <malloc.h> // for memalign() on Android, _aligned_alloc() on Windows
#endif

namespace gawrs_face
{
inline void* alignedMalloc(size_t size, int minimumAlignment) noexcept
{
#if defined(__ANDROID__) || defined(OS_ANDROID)
    return memalign(minimumAlignment, size);
#elif _WIN32
    return ::_aligned_malloc(size, minimumAlignment);
#else // !__ANDROID__ && !OS_ANDROID && !_WIN32
    void* ptr = nullptr;
    // posix_memalign requires that the requested alignment be at least
    // sizeof(void*). In this case, fall back on malloc which should return memory
    // aligned to at least the size of a pointer.
    if (minimumAlignment < sizeof(void*))
        return malloc(size);
    if (::posix_memalign(&ptr, static_cast<size_t>(minimumAlignment), size) != 0)
        return nullptr;
    else
        return ptr;
#endif
}

template <size_t minimumAlignment>
inline void* alignedMalloc(size_t size) noexcept
{
#if defined(__ANDROID__) || defined(OS_ANDROID)
    return memalign(minimumAlignment, size);
#elif _WIN32
    return ::_aligned_malloc(size, minimumAlignment);
#else // !__ANDROID__ && !OS_ANDROID && !_WIN32
    void* ptr = nullptr;
    // posix_memalign requires that the requested alignment be at least
    // sizeof(void*). In this case, fall back on malloc which should return memory
    // aligned to at least the size of a pointer.
    if constexpr (minimumAlignment < sizeof(void*))
        return malloc(size);
    if (::posix_memalign(&ptr, static_cast<size_t>(minimumAlignment), size) != 0)
        return nullptr;
    else
        return ptr;
#endif
}

inline void alignedFree(void* alignedMemory)
{
#ifdef _WIN32
    ::_aligned_free(alignedMemory);
#else
    ::free(alignedMemory);
#endif // _WIN32
}

template <class T>
class AlignedAllocator
{
    // Strict restriction.
    static_assert(!(alignof(T) & (alignof(T) - 1)), "alignof(T) must be a power of two");

public:
    inline AlignedAllocator() noexcept = default;
    template <typename U>
    inline explicit AlignedAllocator(const AlignedAllocator<U>&) noexcept
    {
    }

    inline T* allocate(size_t n) noexcept { return (T*)alignedMalloc(n * sizeof(T), alignof(T)); }

    inline void deallocate(T* p) { alignedFree(p); }

    // Stateless allocators.
    template <typename U>
    bool operator==(const AlignedAllocator<U>& rhs) const noexcept
    {
        return true;
    }

    template <typename U>
    bool operator!=(const AlignedAllocator<U>& rhs) const noexcept
    {
        return false;
    }

    inline ~AlignedAllocator() noexcept = default;
};
} // namespace gawrs_face