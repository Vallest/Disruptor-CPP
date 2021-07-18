// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_BLOCKINGWAITSTRATEGY_HEADER_
#define DISRUPTOR_CPP_BLOCKINGWAITSTRATEGY_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/WaitStrategy.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/SequenceGroup.h>

#include <condition_variable>
#include <thread>
#include <mutex>

namespace vallest {
namespace disruptor {

	class BlockingWaitStrategy : public WaitStrategy {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(BlockingWaitStrategy))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(BlockingWaitStrategy)

	private:
		class WaitGuard {
		DISRUPTOR_CLASS:
			DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(WaitGuard)

		public:
			WaitGuard(size_t& waitCount) : waitCount_(waitCount)
			{
				++waitCount_;
			}

			~WaitGuard()
			{
				--waitCount_;
			}

		private:
			size_t& waitCount_;
		};

	public:
		BlockingWaitStrategy() : waitingCount_(0)
		{
		}

		virtual seq_t waitFor(seq_t sequence, const Sequencer& sequencer, const SequenceGroup& dependentSequences) override
		{
			seq_t availableSequence = 0;

			if ((availableSequence = sequencer.getHighestPublishedSequence(sequence)) < sequence)
			{
				std::unique_lock<std::mutex> lock(m_);

				while ((availableSequence = sequencer.getHighestPublishedSequence(sequence)) < sequence)
				{
					WaitGuard g(waitingCount_);

					cv_.wait(lock);
				}
			}

			if (!dependentSequences.empty())
			{
				while ((availableSequence = dependentSequences.getMinimumSequence()) < sequence)
				{
					std::this_thread::yield();
				}
			}

			return availableSequence;
		}

		virtual void signalAll() override
		{
			std::unique_lock<std::mutex> lock(m_);

			if (waitingCount_ > 0) {
				cv_.notify_all();
			}
		}

	private:
		std::mutex m_;

		std::condition_variable cv_;

		size_t waitingCount_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_BLOCKINGWAITSTRATEGY_HEADER_

