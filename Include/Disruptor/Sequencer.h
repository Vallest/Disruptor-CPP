// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_SEQUENCER_HEADER_
#define DISRUPTOR_CPP_SEQUENCER_HEADER_

#include <Disruptor/Defines.h>

namespace vallest {
namespace disruptor {

	class Sequencer {
	public:
		virtual ~Sequencer() {}

		virtual seq_t next() = 0;

		virtual seq_t next(size_t n) = 0;

		virtual void publish(seq_t sequence) = 0;

		virtual void publish(seq_t lowSequence, seq_t highSequence) = 0;

		virtual seq_t getHighestSequence() const = 0;

		virtual seq_t getHighestPublishedSequence(seq_t sequence) const = 0;

		virtual seq_t getMinimumGatingSequence() const = 0;

		virtual void addGatingSequence(const class Sequence* sequence) = 0;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_SEQUENCER_HEADER_

