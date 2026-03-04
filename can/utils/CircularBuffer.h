#pragma once
#include <vector>

template<typename T>
class CircularBuffer
{
public:
    CircularBuffer(size_t capacity)
        : buffer(capacity)
        , head(0)
        , tail(0)
        , full(false)
    {}

    void push(const T &item)
    {
        buffer[tail] = item;
        tail = (tail + 1) % buffer.size();

        if (full) {
            head = (head + 1) % buffer.size(); // Overwrite oldest
        } else if (tail == head) {
            full = true;
        }
    }

    bool pop(T &item)
    {
        if (empty())
            return false;

        item = buffer[head];
        head = (head + 1) % buffer.size();
        full = false;
        return true;
    }

    bool empty() const { return (!full && (head == tail)); }

    bool isFull() const { return full; }

    size_t size() const
    {
        if (full)
            return buffer.size();
        if (tail >= head)
            return tail - head;
        return buffer.size() + tail - head;
    }

    size_t capacity() const { return buffer.size(); }

    void clear()
    {
        head = tail = 0;
        full = false;
    }

private:
    std::vector<T> buffer;
    size_t head;
    size_t tail;
    bool full;
};
