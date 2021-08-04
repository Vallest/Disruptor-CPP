// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_CACHEALIGNEDARRAY_HEADER_
#define DISRUPTOR_CPP_CACHEALIGNEDARRAY_HEADER_

#include <Disruptor/Defines.h>
#include <Disruptor/Memory.h>
#include <Disruptor/Error.h>

#include <assert.h>

namespace vallest {
namespace disruptor {

	template<typename T>
	class CacheAlignedArray {
	DISRUPTOR_CLASS:
		DISRUPTOR_CLASS_OVERLOAD_NEW(alignof(CacheAlignedArray))
		DISRUPTOR_CLASS_DISALLOW_COPY_AND_ASSIGN(CacheAlignedArray)

	public:
		explicit CacheAlignedArray(size_t size)
		{
			data_ = (T*)vallest::disruptor::allocate(sizeof(T) * size, DISRUPTOR_CACHE_LINE_SIZE);
			size_ = size;

			DISRUPTOR_TRY
			{
				vallest::disruptor::construct(data_, size_);
			}
			DISRUPTOR_CATCH_ALL
			{
				vallest::disruptor::deallocate(data_);
				DISRUPTOR_RETHROW
			}
		}

		template<typename U>
		explicit CacheAlignedArray(size_t size, const U& initValue)
		{
			data_ = (T*)vallest::disruptor::allocate(sizeof(T) * size, DISRUPTOR_CACHE_LINE_SIZE);
			size_ = size;

			DISRUPTOR_TRY
			{
				vallest::disruptor::construct(data_, size_, initValue);
			}
			DISRUPTOR_CATCH_ALL
			{
				vallest::disruptor::deallocate(data_);
				DISRUPTOR_RETHROW
			}
		}

		bool empty() const
		{
			return size_ == 0;
		}

		size_t size() const
		{
			return size_;
		}

		T* begin()
		{
			return data_;
		}

		const T* begin() const
		{
			return data_;
		}

		T* end()
		{
			return data_ + size_;
		}

		const T* end() const
		{
			return data_ + size_;
		}

		T* data()
		{
			return data_;
		}

		const T* data() const
		{
			return data_;
		}

		T& at(size_t index)
		{
			assert(index < size_);
			return data_[index];
		}

		const T& at(size_t index) const
		{
			assert(index < size_);
			return data_[index];
		}

		T& operator[](size_t index)
		{
			assert(index < size_);
			return data_[index];
		}

		const T& operator[](size_t index) const
		{
			assert(index < size_);
			return data_[index];
		}

		~CacheAlignedArray()
		{
			vallest::disruptor::destroy(data_, size_);

			vallest::disruptor::deallocate(data_);
		}

	private:
		T* data_;

		size_t size_;
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_CACHEALIGNEDARRAY_HEADER_

