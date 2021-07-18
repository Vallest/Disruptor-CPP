// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_SLEEPINGWAITSTRATEGY_HEADER_
#define DISRUPTOR_CPP_SLEEPINGWAITSTRATEGY_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/WaitStrategy.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/SequenceGroup.h>

#include <thread>

namespace vallest {
namespace disruptor {

	class SleepingWaitStrategy : public WaitStrategy {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(SleepingWaitStrategy))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(SleepingWaitStrategy)

	public:
		explicit SleepingWaitStrategy(size_t maxSpinCount = 100, size_t maxYieldCount = 100, long long sleepTimeInNanoseconds = 100) :
			maxSpinCount_(maxSpinCount), maxYieldCount_(maxYieldCount), sleepTimeInNanoseconds_(sleepTimeInNanoseconds)
		{
		}

		virtual seq_t waitFor(seq_t sequence, const Sequencer& sequencer, const SequenceGroup& dependentSequences) override
		{
			int64_t availableSequence = 0;

			size_t counter = maxSpinCount_ + maxYieldCount_;

			while ((availableSequence = sequencer.getHighestPublishedSequence(sequence)) < sequence)
			{
				sleep(counter);
			}

			if (!dependentSequences.empty())
			{
				counter = maxSpinCount_ + maxYieldCount_;

				while ((availableSequence = dependentSequences.getMinimumSequence()) < sequence)
				{
					sleep(counter);
				}
			}

			return availableSequence;
		}

		virtual void signalAll() override
		{
		}

	private:
		void sleep(size_t& counter)
		{
			if (counter > maxYieldCount_)
			{
				--counter;
			}
			else if (counter > 0)
			{
				std::this_thread::yield();

				--counter;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(sleepTimeInNanoseconds_));
			}
		}

	private:
		size_t maxSpinCount_;

		size_t maxYieldCount_;

		long long sleepTimeInNanoseconds_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_SLEEPINGWAITSTRATEGY_HEADER_

