// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#include "UnitTesting.h"

#include <Disruptor/RingBuffer.h>
#include <Disruptor/ProducerBarrier.h>
#include <Disruptor/ConsumerBarrier.h>
#include <Disruptor/MultiThreadedSequencer.h>
#include <Disruptor/SingleThreadedSequencer.h>
#include <Disruptor/BlockingWaitStrategy.h>
#include <Disruptor/TimeoutWaitStrategy.h>
#include <Disruptor/SleepingWaitStrategy.h>
#include <Disruptor/YieldingWaitStrategy.h>
#include <Disruptor/BusySpinWaitStrategy.h>

#include <iostream>
#include <memory>
#include <thread>
#include <list>
#include <vector>

using namespace vallest;

enum WaitStrategy {
	wait_strategy_blocking,
	wait_strategy_timeout,
	wait_strategy_busyspin,
	wait_strategy_sleeping,
	wait_strategy_yielding,
	wait_strategy_max,
};

const char* toString(WaitStrategy waitStrategy)
{
	switch (waitStrategy)
	{
	case wait_strategy_blocking:
		return "blocking";

	case wait_strategy_timeout:
		return "timeout";

	case wait_strategy_busyspin:
		return "busyspin";

	case wait_strategy_sleeping:
		return "sleeping";

	case wait_strategy_yielding:
		return "yielding";

	default:
		assert("false");
		return "";
	}
}

bool sequencerTest(size_t bufferSize, size_t numberOfProducers, size_t numberOfMessages, WaitStrategy waitStrategy)
{
	getOutStream() << "bufferSize=" << bufferSize << ", numberOfProducers=" << numberOfProducers
		<< ", numberOfMessages=" << numberOfMessages << ", waitStrategy=" << toString(waitStrategy) << std::endl;

	disruptor::RingBuffer<int> rb(bufferSize);

	std::unique_ptr<disruptor::Sequencer> sequencer;

	if (numberOfProducers > 1) {
		sequencer.reset(new disruptor::MultiThreadedSequencer(bufferSize));
	}
	else {
		sequencer.reset(new disruptor::SingleThreadedSequencer(bufferSize));
	}

	std::unique_ptr<disruptor::WaitStrategy> ws;

	switch (waitStrategy)
	{
	case wait_strategy_blocking:
		ws.reset(new disruptor::BlockingWaitStrategy());
		break;

	case wait_strategy_timeout:
		ws.reset(new disruptor::TimeoutWaitStrategy());
		break;

	case wait_strategy_busyspin:
		ws.reset(new disruptor::BusySpinWaitStrategy());
		break;

	case wait_strategy_sleeping:
		ws.reset(new disruptor::SleepingWaitStrategy());
		break;

	case wait_strategy_yielding:
		ws.reset(new disruptor::YieldingWaitStrategy());
		break;

	default:
		assert(false);
	}

	disruptor::ProducerBarrier pb(*sequencer, *ws);
	disruptor::ConsumerBarrier cb(*sequencer, *ws);

	uint64_t result;

	std::chrono::steady_clock::time_point endTime;

	std::thread consumer([&rb, &cb, numberOfProducers, &result, &endTime]()
	{
		uint64_t sum = 0;

		size_t producers = numberOfProducers;

		disruptor::seq_t sequence = 0;

		for (;;)
		{
			disruptor::seq_t availableSequence = cb.waitFor(sequence);

			if (availableSequence >= sequence)
			{
				for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
				{
					int msg = rb.at(i);
					
					if (msg < 0)
					{
						if (--producers == 0)
						{
							endTime = std::chrono::steady_clock::now();
							result = sum;
							return;
						}
					}
					else
					{
						sum += msg;
					}
				}

				cb.consume(availableSequence);

				sequence = availableSequence + 1;
			}
		}
	});

	std::vector<std::chrono::steady_clock::time_point> startTimeTable(numberOfProducers);

	std::list<std::shared_ptr<std::thread>> producers;

	for (size_t producerIdx = 0; producerIdx < numberOfProducers; ++producerIdx)
	{
		producers.push_back(std::make_shared<std::thread>([&rb, &pb, producerIdx, &startTimeTable, numberOfMessages, numberOfProducers]()
		{
			int m = (int)(numberOfMessages / numberOfProducers);
			int n = (int)(numberOfMessages - (m * numberOfProducers));

			int begin = m * (int)producerIdx;
			int end = begin + m + ((producerIdx == numberOfProducers - 1) ? n : 0);

			startTimeTable[producerIdx] = std::chrono::steady_clock::now();

			for (int i = begin; i < end; ++i)
			{
				disruptor::seq_t sequence = pb.next();

				rb.at(sequence) = i;

				pb.produce(sequence);
			}

			disruptor::seq_t sequence = pb.next();

			rb.at(sequence) = -1;

			pb.produce(sequence);
		}));
	}

	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::time_point::max();

	for (size_t i = 0; i < numberOfProducers; ++i)
	{
		producers.front()->join();

		startTime = std::min(startTime, startTimeTable[i]);

		producers.pop_front();
	}

	consumer.join();

	std::chrono::duration<float> elapsed = endTime - startTime;

	size_t mps = (size_t)((double)numberOfMessages / elapsed.count());

	getOutStream() << "Elapsed: " << elapsed.count() << "s, MPS: " << mps << std::endl;
	getOutStream() << std::endl;

	return result == uint64_t(numberOfMessages - 1) * numberOfMessages / 2;
}

bool pipelineTest(size_t bufferSize, size_t numberOfMessages)
{
	getOutStream() << "bufferSize=" << bufferSize
		<< ", numberOfMessages=" << numberOfMessages << std::endl;

	disruptor::RingBuffer<int> rb(bufferSize);

	disruptor::SingleThreadedSequencer sequencer(bufferSize);

	disruptor::BlockingWaitStrategy waitStrategy;

	disruptor::ProducerBarrier pb(sequencer, waitStrategy);

	disruptor::ConsumerBarrier cb1(sequencer, waitStrategy);

	disruptor::ConsumerBarrier cb2(sequencer, waitStrategy, cb1);

	disruptor::ConsumerBarrier cb3(sequencer, waitStrategy, cb2);

	std::thread consumer1([&rb, &cb1]()
	{
		disruptor::seq_t sequence = 0;

		for (;;)
		{
			disruptor::seq_t availableSequence = cb1.waitFor(sequence);

			if (availableSequence >= sequence)
			{
				for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
				{
					int msg = rb.at(i);
					
					if (msg < 0)
					{
						cb1.consume(i);
						return;
					}

					rb.at(i) = msg * 2;
				}

				cb1.consume(availableSequence);

				sequence = availableSequence + 1;
			}
		}
	});

	std::thread consumer2([&rb, &cb2]()
	{
		disruptor::seq_t sequence = 0;

		for (;;)
		{
			disruptor::seq_t availableSequence = cb2.waitFor(sequence);

			if (availableSequence >= sequence)
			{
				for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
				{
					int msg = rb.at(i);
					
					if (msg < 0)
					{
						cb2.consume(i);
						return;
					}

					rb.at(i) = msg + 1;
				}

				cb2.consume(availableSequence);

				sequence = availableSequence + 1;
			}
		}
	});

	uint64_t result;

	std::chrono::steady_clock::time_point endTime;

	std::thread consumer3([&rb, &cb3, &result, &endTime]()
	{
		uint64_t sum = 0;

		disruptor::seq_t sequence = 0;

		for (;;)
		{
			disruptor::seq_t availableSequence = cb3.waitFor(sequence);

			if (availableSequence >= sequence)
			{
				for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
				{
					int msg = rb.at(i);
					
					if (msg < 0)
					{
						result = sum;
						endTime = std::chrono::steady_clock::now();
						return;
					}

					sum += msg;
				}

				cb3.consume(availableSequence);

				sequence = availableSequence + 1;
			}
		}
	});

	std::chrono::steady_clock::time_point startTime;

	std::thread producer([&rb, &pb, numberOfMessages, &startTime]()
	{
		startTime = std::chrono::steady_clock::now();

		int begin = 0;
		int end = (int)numberOfMessages;

		for (int i = begin; i < end; ++i)
		{
			disruptor::seq_t sequence = pb.next();

			rb.at(sequence) = i;

			pb.produce(sequence);
		}

		disruptor::seq_t sequence = pb.next();

		rb.at(sequence) = -1;

		pb.produce(sequence);
	});

	producer.join();

	consumer1.join();
	consumer2.join();
	consumer3.join();

	std::chrono::duration<float> elapsed = endTime - startTime;

	size_t mps = (size_t)((double)numberOfMessages / elapsed.count());

	getOutStream() << "Elapsed: " << elapsed.count() << "s, MPS: " << mps << std::endl;
	getOutStream() << std::endl;

	return result == (uint64_t)numberOfMessages * numberOfMessages;
}

bool multicastTest(size_t bufferSize, size_t numberOfMessages)
{
	getOutStream() << "bufferSize=" << bufferSize
		<< ", numberOfMessages=" << numberOfMessages << std::endl;

	disruptor::RingBuffer<int> rb(bufferSize);

	disruptor::SingleThreadedSequencer sequencer(bufferSize);

	disruptor::BlockingWaitStrategy waitStrategy;

	disruptor::ProducerBarrier pb(sequencer, waitStrategy);

	std::shared_ptr<disruptor::ConsumerBarrier> cbTable[3];

	cbTable[0].reset(new disruptor::ConsumerBarrier(sequencer, waitStrategy));
	cbTable[1].reset(new disruptor::ConsumerBarrier(sequencer, waitStrategy));
	cbTable[2].reset(new disruptor::ConsumerBarrier(sequencer, waitStrategy));

	uint64_t resultTable[3];

	std::chrono::steady_clock::time_point endTimeTable[3];

	std::list<std::shared_ptr<std::thread>> consumers;

	for (size_t consumerIdx = 0; consumerIdx < 3; ++consumerIdx)
	{
		consumers.push_back(std::make_shared<std::thread>([&rb, consumerIdx, &cbTable, &resultTable, &endTimeTable]()
		{
			uint64_t sum = 0;

			disruptor::seq_t sequence = 0;

			for (;;)
			{
				disruptor::seq_t availableSequence = cbTable[consumerIdx]->waitFor(sequence);

				if (availableSequence >= sequence)
				{
					for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
					{
						int msg = rb.at(i);

						if (msg < 0)
						{
							resultTable[consumerIdx] = sum;
							endTimeTable[consumerIdx] = std::chrono::steady_clock::now();
							return;
						}

						sum += msg;
					}

					cbTable[consumerIdx]->consume(availableSequence);

					sequence = availableSequence + 1;
				}
			}
		}));
	}

	std::chrono::steady_clock::time_point startTime;

	std::thread producer([&rb, &pb, numberOfMessages, &startTime]()
	{
		startTime = std::chrono::steady_clock::now();

		int begin = 0;
		int end = (int)numberOfMessages;

		for (int i = begin; i < end; ++i)
		{
			disruptor::seq_t sequence = pb.next();

			rb.at(sequence) = i;

			pb.produce(sequence);
		}

		disruptor::seq_t sequence = pb.next();

		rb.at(sequence) = -1;

		pb.produce(sequence);
	});

	producer.join();

	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::time_point::min();

	for (size_t i = 0; i < 3; ++i)
	{
		consumers.front()->join();

		endTime = std::max(endTime, endTimeTable[i]);

		consumers.pop_front();
	}

	std::chrono::duration<float> elapsed = endTime - startTime;

	size_t mps = (size_t)((double)numberOfMessages / elapsed.count());

	getOutStream() << "Elapsed: " << elapsed.count() << "s, MPS: " << mps << std::endl;
	getOutStream() << std::endl;

	for (size_t i = 0; i < 3; ++i)
	{
		if (resultTable[i] != uint64_t(numberOfMessages - 1) * numberOfMessages / 2) {
			return false;
		}
	}

	return true;
}

TEST_CASE("Sequencer")
{
	for (int producers = 1; producers <= 16; producers *= 2)
	{
		CHECK(sequencerTest(16384, producers, 50000000, wait_strategy_blocking));
	}
}

TEST_CASE("WaitStrategy")
{
	for (int ws = 0; ws < wait_strategy_max; ++ws)
	{
		CHECK(sequencerTest(16384, 4, 50000000, (WaitStrategy)ws));
	}
}

TEST_CASE("Pipeline(1P3C)")
{
	CHECK(pipelineTest(16384, 50000000));
}

TEST_CASE("Multicast(1P3C)")
{
	CHECK(multicastTest(16384, 50000000));
}

