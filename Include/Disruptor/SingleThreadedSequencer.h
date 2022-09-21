// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_SINGLETHREADEDSEQUENCER_HEADER_
#define DISRUPTOR_CPP_SINGLETHREADEDSEQUENCER_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/Sequence.h>
#include <Disruptor/SequenceGroup.h>
#include <Disruptor/Error.h>

#include <thread>
#include <memory>

namespace vallest {
namespace disruptor {

	class SingleThreadedSequencer : public Sequencer {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(SingleThreadedSequencer))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(SingleThreadedSequencer)

	private:
		 
	public:
		explicit SingleThreadedSequencer(size_t bufferSize) :
			bufferSize_(bufferSize), current_(-1), cachedGatingSequence_(-1),
			sequence_(Sequence::create())
		{
			if (bufferSize_ == 0 || (bufferSize_ & (bufferSize_ - 1)) != 0) {
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

			seq_t current = current_;
			seq_t next = current + n;
			seq_t wrapPoint = next - bufferSize_;

			if (wrapPoint > cachedGatingSequence_)
			{
				seq_t gatingSequence;

				while (wrapPoint > (gatingSequence = gatingSequences_.getMinimumSequence()))
				{
					std::this_thread::yield();
				}

				cachedGatingSequence_ = gatingSequence;
			}

			current_ = next;

			return next;
		}

		virtual void publish(seq_t sequence) override
		{
			sequence_->set(sequence);
		}

		virtual void publish(seq_t, seq_t highSequence) override
		{
			sequence_->set(highSequence);
		}

		virtual seq_t getHighestSequence() const override
		{
			return sequence_->get();
		}

		virtual seq_t getHighestPublishedSequence(seq_t sequence) const override
		{
			seq_t upperBound = sequence_->get();
			seq_t lowerBound = upperBound - bufferSize_ + 1;

			if (sequence > upperBound || sequence < lowerBound) {
				return sequence - 1;
			}

			return upperBound;
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
		size_t bufferSize_;

		seq_t current_;

		seq_t cachedGatingSequence_;

		std::unique_ptr<Sequence> sequence_;

		SequenceGroup gatingSequences_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_SINGLETHREADEDSEQUENCER_HEADER_

