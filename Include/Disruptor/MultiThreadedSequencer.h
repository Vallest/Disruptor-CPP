// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_MULTITHREADEDSEQUENCER_HEADER_
#define DISRUPTOR_CPP_MULTITHREADEDSEQUENCER_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/CacheAlignedArray.h>
#include <Disruptor/Sequence.h>
#include <Disruptor/SequenceGroup.h>
#include <Disruptor/Utility.h>
#include <Disruptor/Error.h>

#include <atomic>
#include <thread>
#include <memory>

namespace vallest {
namespace disruptor {

	class MultiThreadedSequencer : public Sequencer {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(MultiThreadedSequencer))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(MultiThreadedSequencer)

	private:
		typedef CacheAlignedArray<std::atomic_int> AvailableBuffer;

	public:
		explicit MultiThreadedSequencer(size_t bufferSize) :
			bufferSize_(bufferSize), indexMask_(bufferSize - 1), indexShift_(vallest::disruptor::countTailingZeros(bufferSize)),
			sequence_(Sequence::create()), availableBuffer_(bufferSize, -1),
			cachedGatingSequence_(Sequence::create())
		{
			if (bufferSize_ == 0 || (bufferSize_ & indexMask_) != 0) {
				DISRUPTOR_FAULT(InvalidArgument);
			}
		}

		virtual seq_t next() override
		{
			return next(1);
		}

		virtual seq_t next(size_t n) override
		{
			assert(n > 0 && n <= bufferSize_);

			for (;;)
			{
				seq_t current = sequence_->get();
				seq_t next = current + n;
				seq_t wrapPoint = next - bufferSize_;
				seq_t cachedGatingSequence = cachedGatingSequence_->get();

				if (wrapPoint > cachedGatingSequence)
				{
					seq_t gatingSequence = gatingSequences_.getMinimumSequence();

					if (wrapPoint > gatingSequence)
					{
						std::this_thread::yield();
						continue;
					}

					cachedGatingSequence_->compareAndSet(cachedGatingSequence, gatingSequence);
				}
				else if (sequence_->compareAndSet(current, next))
				{
					return next;
				}
			}
		}

		virtual void publish(seq_t sequence) override
		{
			setAvailable(sequence);
		}

		virtual void publish(seq_t lowSequence, seq_t highSequence) override
		{
			for (; lowSequence <= highSequence; ++lowSequence) {
				setAvailable(lowSequence);
			}
		}

		virtual seq_t getHighestSequence() const override
		{
			return sequence_->get();
		}

		virtual seq_t getHighestPublishedSequence(seq_t sequence) const override
		{
			seq_t current = sequence_->get();

			for (; sequence <= current; ++sequence)
			{
				if (!isAvailable(sequence)) {
					break;
				}
			}

			return sequence - 1;
		}

		virtual seq_t getMinimumGatingSequence() const override
		{
			return gatingSequences_.getMinimumSequence();
		}

		virtual void addGatingSequence(const Sequence* sequence) override
		{
			gatingSequences_.addUnique(sequence);
		}

	private:
		void setAvailable(seq_t sequence)
		{
			size_t index = (size_t)(sequence & indexMask_);
			int flag = (int)(sequence >> indexShift_);

			availableBuffer_[index].store(flag, std::memory_order_release);
		}

		bool isAvailable(seq_t sequence) const
		{
			size_t index = (size_t)(sequence & indexMask_);
			int flag = (int)(sequence >> indexShift_);

			return availableBuffer_[index].load(std::memory_order_acquire) == flag;
		}

	private:
		size_t bufferSize_;
		size_t indexMask_;
		size_t indexShift_;

		std::unique_ptr<Sequence> sequence_;

		SequenceGroup gatingSequences_;

		AvailableBuffer availableBuffer_;

		std::unique_ptr<Sequence> cachedGatingSequence_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_MULTITHREADEDSEQUENCER_HEADER_

