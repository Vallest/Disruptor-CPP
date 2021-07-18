// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_CONSUMERBARRIER_HEADER_
#define DISRUPTOR_CPP_CONSUMERBARRIER_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/WaitStrategy.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/SequenceGroup.h>

#include <initializer_list>
#include <memory>

namespace vallest {
namespace disruptor {

	class ConsumerBarrier {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(ConsumerBarrier))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(ConsumerBarrier)

	public:
		explicit ConsumerBarrier(Sequencer& sequencer, WaitStrategy& waitStrategy) :
			sequencer_(sequencer), waitStrategy_(waitStrategy),
			sequence_(Sequence::create())
		{
			sequencer_.addGatingSequence(sequence_.get());
		}

		template<typename... Args>
		explicit ConsumerBarrier(Sequencer& sequencer, WaitStrategy& waitStrategy, const Args&... dependencies) :
			sequencer_(sequencer), waitStrategy_(waitStrategy),
			sequence_(Sequence::create())
		{
			sequencer_.addGatingSequence(sequence_.get());

			addDependency(dependencies...);
		}

		seq_t waitFor(seq_t sequence)
		{
			return waitStrategy_.waitFor(sequence, sequencer_, dependentSequences_);
		}

		void consume(seq_t sequence)
		{
			sequence_->set(sequence);
		}

	private:
		void addDependency(const ConsumerBarrier& dependency)
		{
			dependentSequences_.addUnique(dependency.sequence_.get());
		}

		template<typename... Args>
		void addDependency(const ConsumerBarrier& dependency, const Args&... dependencies)
		{
			dependentSequences_.addUnique(dependency.sequence_.get());

			addDependency(dependencies...);
		}

	private:
		Sequencer& sequencer_;

		WaitStrategy& waitStrategy_;

		std::unique_ptr<Sequence> sequence_;

		SequenceGroup dependentSequences_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_CONSUMERBARRIER_HEADER_

