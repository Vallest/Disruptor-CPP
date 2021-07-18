// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_YIELDINGWAITSTRATEGY_HEADER_
#define DISRUPTOR_CPP_YIELDINGWAITSTRATEGY_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/WaitStrategy.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/SequenceGroup.h>

#include <thread>

namespace vallest {
namespace disruptor {

	class YieldingWaitStrategy : public WaitStrategy {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(YieldingWaitStrategy))
			DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(YieldingWaitStrategy)

	public:
		explicit YieldingWaitStrategy(size_t maxSpinCount = 100) : maxSpinCount_(maxSpinCount)
		{
		}

		virtual seq_t waitFor(seq_t sequence, const Sequencer& sequencer, const SequenceGroup& dependentSequences) override
		{
			int64_t availableSequence = 0;

			size_t counter = maxSpinCount_;

			while ((availableSequence = sequencer.getHighestPublishedSequence(sequence)) < sequence)
			{
				wait(counter);
			}

			if (!dependentSequences.empty())
			{
				counter = maxSpinCount_;

				while ((availableSequence = dependentSequences.getMinimumSequence()) < sequence)
				{
					wait(counter);
				}
			}

			return availableSequence;
		}

		virtual void signalAll() override
		{
		}

	private:
		void wait(size_t& counter)
		{
			if (counter > maxSpinCount_) {
				--counter;
			}
			else {
				std::this_thread::yield();
			}
		}

	private:
		size_t maxSpinCount_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_YIELDINGWAITSTRATEGY_HEADER_

