// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#ifndef DISRUPTOR_CPP_ERROR_HEADER_
#define DISRUPTOR_CPP_ERROR_HEADER_

#include <exception>

#if !defined(DISRUPTOR_NO_EXCEPTIONS)
	#define DISRUPTOR_THROW(E, ...) throw E(__VA_ARGS__)
	#define DISRUPTOR_TRY try
	#define DISRUPTOR_CATCH(X) catch (X)
	#define DISRUPTOR_CATCH_ALL catch(...)
	#define DISRUPTOR_RETHROW throw;
#else
	#define DISRUPTOR_THROW(E, ...) DISRUPTOR_TERMINATE
	#define DISRUPTOR_TRY if (true)
	#define DISRUPTOR_CATCH(x) if (false)
	#define DISRUPTOR_CATCH_ALL if (false)
	#define DISRUPTOR_RETHROW ((void)0);
#endif

#if !defined(DISRUPTOR_TERMINATE)
	#define DISRUPTOR_TERMINATE std::terminate()
#endif

#define DISRUPTOR_FAULT(Name, ...) \
	DISRUPTOR_THROW(vallest::disruptor::Name##Exception, __VA_ARGS__)

#if defined(_MSC_VER) && _MSC_VER <= 1600
	#define DISRUPTOR_NOTHROW
#else
	#define DISRUPTOR_NOTHROW throw()
#endif

namespace vallest {
namespace disruptor {

	class Exception : public std::exception {
	public:
		virtual const char* what() const DISRUPTOR_NOTHROW
		{
			return "unknown";
		}
	};

	class LogicErrorException : public Exception {
	public:
		virtual const char* what() const DISRUPTOR_NOTHROW
		{
			return "logic error";
		}
	};

	class InvalidArgumentException : public LogicErrorException {
	public:
		virtual const char* what() const DISRUPTOR_NOTHROW
		{
			return "invalid argument";
		}
	};

	class OutOfMemoryException : public Exception {
	public:
		virtual const char* what() const DISRUPTOR_NOTHROW
		{
			return "out of memory";
		}
	};

} // disruptor
} // vallest

#endif // DISRUPTOR_CPP_ERROR_HEADER_

