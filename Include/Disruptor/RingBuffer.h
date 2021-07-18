// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_RINGBUFFER_HEADER_
#define DISRUPTOR_CPP_RINGBUFFER_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/CacheAlignedArray.h>
#include <Disruptor/Error.h>

#include <assert.h>

namespace vallest {
namespace disruptor {

	template<typename T>
	class RingBuffer {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(RingBuffer))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(RingBuffer)

	public:
		explicit RingBuffer(size_t size) : buffer_(size), indexMask_(size - 1)
		{
			if (size == 0 || (size & indexMask_) != 0) {
				DISRUPTOR_FAULT(InvalidArgument);
			}
		}

		T& operator[](seq_t sequence)
		{
			assert(sequence >= 0);
			return buffer_[(size_t)(sequence & indexMask_)];
		}

		const T& operator[](seq_t sequence) const
		{
			assert(sequence >= 0);
			return buffer_[(size_t)(sequence & indexMask_)];
		}

		T& at(seq_t sequence)
		{
			assert(sequence >= 0);
			return buffer_[(size_t)(sequence & indexMask_)];
		}

		const T& at(seq_t sequence) const
		{
			assert(sequence >= 0);
			return buffer_[(size_t)(sequence & indexMask_)];
		}

	private:
		CacheAlignedArray<T> buffer_;

		size_t indexMask_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_RINGBUFFER_HEADER_

