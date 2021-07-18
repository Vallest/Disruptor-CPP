// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_WAITSTRATEGY_HEADER_
#define DISRUPTOR_CPP_WAITSTRATEGY_HEADER_

#include <Disruptor/Defines.h>

namespace vallest {
namespace disruptor {

	class WaitStrategy {
	public:
		virtual ~WaitStrategy() {}

		virtual seq_t waitFor(seq_t sequence, const class Sequencer& sequencer, const class SequenceGroup& dependentSequences) = 0;

		virtual void signalAll() = 0;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_WAITSTRATEGY_HEADER_

