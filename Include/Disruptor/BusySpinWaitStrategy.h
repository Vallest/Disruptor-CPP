// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_BUSYSPINWAITSTRATEGY_HEADER_
#define DISRUPTOR_CPP_BUSYSPINWAITSTRATEGY_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/WaitStrategy.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/SequenceGroup.h>

namespace vallest {
namespace disruptor {

	class BusySpinWaitStrategy : public WaitStrategy {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(BusySpinWaitStrategy))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(BusySpinWaitStrategy)

	public:
		BusySpinWaitStrategy()
		{
		}

		virtual seq_t waitFor(seq_t sequence, const Sequencer& sequencer, const SequenceGroup& dependentSequences) override
		{
			int64_t availableSequence = 0;

			while ((availableSequence = sequencer.getHighestPublishedSequence(sequence)) < sequence)
			{
			}

			if (!dependentSequences.empty())
			{
				while ((availableSequence = dependentSequences.getMinimumSequence()) < sequence)
				{
				}
			}

			return availableSequence;
		}

		virtual void signalAll() override
		{
		}
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_BUSYSPINWAITSTRATEGY_HEADER_

