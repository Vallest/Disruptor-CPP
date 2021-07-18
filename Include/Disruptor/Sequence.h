// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_SEQUENCE_HEADER_
#define DISRUPTOR_CPP_SEQUENCE_HEADER_

#include <Disruptor/Defines.h>

#include <atomic>
#include <string.h>
#include <assert.h>

namespace vallest {
namespace disruptor {

	class Sequence {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(DISRUPTOR_CACHE_LINE_SIZE)

	private:
		Sequence(seq_t initValue) : value_(initValue)
		{
			assert(sizeof(Sequence) == DISRUPTOR_CACHE_LINE_SIZE);

			memset(padding_, 0, sizeof(padding_));
		}

	public:
		seq_t get() const
		{
			return value_.load(std::memory_order_acquire);
		}

		void set(seq_t value)
		{
			value_.store(value, std::memory_order_release);
		}

		bool compareAndSet(seq_t& expectedValue, seq_t newValue)
		{
			return value_.compare_exchange_strong(expectedValue, newValue,
				std::memory_order_release, std::memory_order_relaxed);
		}

	public:
		static Sequence* create(seq_t initValue = (seq_t)(-1))
		{
			return new Sequence(initValue);
		}

	private:
		std::atomic<seq_t> value_;

		char padding_[DISRUPTOR_CACHE_LINE_SIZE - sizeof(std::atomic<seq_t>)];
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_SEQUENCE_HEADER_

