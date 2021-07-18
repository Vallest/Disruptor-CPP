// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_MEMORY_HEADER_
#define DISRUPTOR_CPP_MEMORY_HEADER_

#include <Disruptor/Error.h>

#include <stddef.h>
#include <assert.h>

#if !defined(DISRUPTOR_ALLOC)

	#if defined(_MSC_VER)

		#include <malloc.h>

		#define DISRUPTOR_ALLOC(Size, Align) _aligned_malloc(Size, Align)
		#define DISRUPTOR_FREE(Ptr) _aligned_free(Ptr)

	#elif defined(__APPLE__) || defined(__linux__)

		#include <stdlib.h>

		inline void* vallest_disruptor_aligned_malloc(size_t size, size_t alignment)
		{
			void* ptr;

			if (posix_memalign(&ptr, alignment, size) != 0) {
				return 0;
			}

			return ptr;
		}

		inline void vallest_disruptor_aligned_free(void* ptr)
		{
			free(ptr);
		}

		#define DISRUPTOR_ALLOC(Size, Align) vallest_disruptor_aligned_malloc(Size, Align)
		#define DISRUPTOR_FREE(Ptr) vallest_disruptor_aligned_free(Ptr)

	#else

		#include <stdlib.h>
		#include <stdint.h>

		inline void* vallest_disruptor_aligned_malloc(size_t size, size_t alignment)
		{
			size_t offset = sizeof(void*) + alignment - 1;

			void* ptr = malloc(size + offset);

			if (!ptr) {
				return 0;
			}

			void* alignedPtr = (void*)(((uintptr_t)ptr + offset) & ~(alignment - 1));

			*((void**)alignedPtr - 1) = ptr;

			return alignedPtr;
		}

		inline void vallest_disruptor_aligned_free(void* ptr)
		{
			free(*((void**)ptr - 1));
		}

		#define DISRUPTOR_ALLOC(Size, Align) vallest_disruptor_aligned_malloc(Size, Align)
		#define DISRUPTOR_FREE(Ptr) vallest_disruptor_aligned_free(Ptr)

	#endif

#endif // DISRUPTOR_ALLOC

namespace vallest {
namespace disruptor {

	inline void* allocate(size_t size, size_t alignment)
	{
		assert((alignment & (alignment - 1)) == 0);

		void* ptr = DISRUPTOR_ALLOC(size, alignment);

		if (!ptr) {
			DISRUPTOR_FAULT(OutOfMemory);
		}

		return ptr;
	}

	inline void deallocate(void* ptr)
	{
		DISRUPTOR_FREE(ptr);
	}

	template<typename T>
	inline void destroy(T* ptr)
	{
		(void)ptr;
		ptr->~T();
	}

	template<typename T>
	inline void destroy(T* ptr, size_t n)
	{
		while (n > 0) {
			destroy(ptr + --n);
		}
	}

	template<typename T>
	inline void construct(T* ptr, size_t n)
	{
		T* p = ptr;

		DISRUPTOR_TRY
		{
			for (; p != ptr + n; ++p) {
				new(p) T;
			}
		}
		DISRUPTOR_CATCH_ALL
		{
			destroy(ptr, p - ptr);
			DISRUPTOR_RETHROW
		}
	}

	template<typename T, typename U>
	inline void construct(T* ptr, size_t n, const U& initValue)
	{
		T* p = ptr;

		DISRUPTOR_TRY
		{
			for (; p != ptr + n; ++p) {
				new(p) T(initValue);
			}
		}
		DISRUPTOR_CATCH_ALL
		{
			destroy(ptr, p - ptr);
			DISRUPTOR_RETHROW
		}
	}

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_MEMORY_HEADER_

