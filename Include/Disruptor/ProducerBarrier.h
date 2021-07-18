// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_PRODUCERBARRIER_HEADER_
#define DISRUPTOR_CPP_PRODUCERBARRIER_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/Sequencer.h>
#include <Disruptor/WaitStrategy.h>

#include <atomic>

namespace vallest {
namespace disruptor {

	class ProducerBarrier {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(ProducerBarrier))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(ProducerBarrier)

	public:
		explicit ProducerBarrier(Sequencer& sequencer, WaitStrategy& waitStrategy) :
			sequencer_(sequencer), waitStrategy_(waitStrategy)
		{
		}

		seq_t next()
		{
			return sequencer_.next();
		}

		seq_t next(size_t n)
		{
			return sequencer_.next(n);
		}

		void produce(seq_t sequence)
		{
			sequencer_.publish(sequence);

			waitStrategy_.signalAll();
		}

		void produce(seq_t lowSequence, seq_t highSequence)
		{
			sequencer_.publish(lowSequence, highSequence);

			waitStrategy_.signalAll();
		}

	private:
		Sequencer& sequencer_;

		WaitStrategy& waitStrategy_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_PRODUCERBARRIER_HEADER_

