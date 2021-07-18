# Disruptor-CPP

Disruptor-CPP is a basic implementation of [LMAX Disruptor](https://lmax-exchange.github.io/disruptor/disruptor.html) in C++11.



## Compatibility

The following platform and compiler combinations have been tested:

- Linux using g++ 7.5
- Linux using Clang 11.1
- Windows using Visual C++ 14/16
- macOS using Xcode 12.5.1
- iOS (macOS using Xcode 12.5.1)
- Android (Windows using Android Studio 4.1.2)



## Example

```c++
#include <Disruptor/RingBuffer.h>
#include <Disruptor/ProducerBarrier.h>
#include <Disruptor/ConsumerBarrier.h>
#include <Disruptor/BlockingWaitStrategy.h>
#include <Disruptor/MultiThreadedSequencer.h>

#include <thread>
#include <list>
#include <iostream>
#include <stdint.h>
#include <stddef.h>

using namespace vallest;

void send(disruptor::RingBuffer<int>& rb, disruptor::ProducerBarrier& pb, int msg)
{
    disruptor::seq_t sequence = pb.next();

    rb[sequence] = msg;

    pb.produce(sequence);
}

void producerRoutine(disruptor::RingBuffer<int>& rb, disruptor::ProducerBarrier& pb)
{
    for (int i = 0; i < 1000000; ++i)
    {
        send(rb, pb, i);
    }
}

void consumerRoutine(disruptor::RingBuffer<int>& rb, disruptor::ConsumerBarrier& cb)
{
    int64_t sum = 0;

    disruptor::seq_t sequence = 0;

    for (;;)
    {
        disruptor::seq_t availableSequence = cb.waitFor(sequence);

        if (availableSequence >= sequence)
        {
            for (disruptor::seq_t i = sequence; i <= availableSequence; ++i)
            {
                int msg = rb[i];

                if (msg < 0)
                {
                    std::cout << sum << std::endl;
                    return;
                }

                sum += msg;
            }

            cb.consume(availableSequence);

            sequence = availableSequence + 1;
        }
    }
}

int main()
{
    const size_t bufferSize = 65536;

    disruptor::RingBuffer<int> rb(bufferSize);

    disruptor::MultiThreadedSequencer sequencer(bufferSize);

    disruptor::BlockingWaitStrategy waitStrategy;

    disruptor::ProducerBarrier pb(sequencer, waitStrategy);

    disruptor::ConsumerBarrier cb(sequencer, waitStrategy);

    std::thread consumer(consumerRoutine, std::ref(rb), std::ref(cb));

    std::list<std::thread> producers;

    for (int i = 0; i < 4; ++i)
    {
        producers.push_back(std::thread(producerRoutine, std::ref(rb), std::ref(pb)));
    }

    while (!producers.empty())
    {
        producers.back().join();
        producers.pop_back();
    }

    send(rb, pb, -1);

    consumer.join();
}
```

