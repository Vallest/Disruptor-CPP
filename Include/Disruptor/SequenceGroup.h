// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_SEQUENCEGROUP_HEADER_
#define DISRUPTOR_CPP_SEQUENCEGROUP_HEADER_

#include <Disruptor/Sequence.h>
#include <Disruptor/Defines.h>
#include <Disruptor/Memory.h>

#include <algorithm>
#include <numeric>

namespace vallest {
namespace disruptor {

	class SequenceGroup {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(SequenceGroup))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(SequenceGroup)

	private:
		typedef const Sequence* SequencePtr;

	public:
		SequenceGroup() : buffer_(0), size_(0)
		{
		}

		~SequenceGroup()
		{
			if (buffer_) {
				vallest::disruptor::deallocate(buffer_);
			}
		}

		bool empty() const
		{
			return size_ == 0;
		}

		seq_t getMinimumSequence() const
		{
			seq_t m = std::numeric_limits<seq_t>::max();

			for (SequencePtr* p = buffer_; p != buffer_ + size_; ++p)
			{
				m = std::min(m, (*p)->get());
			}

			return m;
		}

		void addUnique(const Sequence* sequence)
		{
			if (std::find(buffer_, buffer_ + size_, sequence) == buffer_ + size_)
			{
				SequencePtr* buffer = (SequencePtr*)vallest::disruptor::allocate(sizeof(SequencePtr) * (size_ + 1), alignof(SequencePtr));

				if (buffer_)
				{
					memcpy(buffer, buffer_, sizeof(SequencePtr) * size_);

					vallest::disruptor::deallocate(buffer_);
				}

				buffer_ = buffer;

				buffer_[size_++] = sequence;
			}
		}

	private:
		SequencePtr* buffer_;

		size_t size_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_SEQUENCEGROUP_HEADER_

