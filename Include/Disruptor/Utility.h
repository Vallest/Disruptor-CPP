// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_UTILITY_HEADER_
#define DISRUPTOR_CPP_UTILITY_HEADER_

namespace vallest {
namespace disruptor {

	template<typename Int>
	inline size_t countTailingZeros(Int value)
	{
		size_t n = 0;

		while ((value >>= 1) != 0) {
			++n;
		}

		return n;
	}

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_UTILITY_HEADER_

