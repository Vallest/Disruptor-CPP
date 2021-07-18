// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_DEFINES_HEADER_
#define DISRUPTOR_CPP_DEFINES_HEADER_

#include <Disruptor/Memory.h>

#include <stdint.h>
#include <assert.h>

#define DISRUPTOR_CLASS private

#define DISRUPTOR_CLASS_OVERLOAD_NEW(Align)																	\
	public:																									\
	void* operator new(size_t sz) { return vallest::disruptor::allocate(sz, Align); }						\
	void operator delete(void* ptr) { vallest::disruptor::deallocate(ptr); }								\
	void* operator new[](size_t sz) { return vallest::disruptor::allocate(sz, Align); }						\
	void operator delete[](void* ptr) { vallest::disruptor::deallocate(ptr); }								\
	void* operator new(size_t, void* ptr) { assert(((uintptr_t)ptr & (Align - 1)) == 0); return ptr; }		\
	void operator delete(void*, void*) {}

#define DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	private:												\
	TypeName(const TypeName&);								\
	TypeName& operator=(const TypeName&);

#if !defined(DISRUPTOR_CACHE_LINE_SIZE)
	#define DISRUPTOR_CACHE_LINE_SIZE 64
#endif

namespace vallest {
namespace disruptor {

	typedef int64_t seq_t;

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_DEFINES_HEADER_

