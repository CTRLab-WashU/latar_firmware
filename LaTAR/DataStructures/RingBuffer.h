#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <array>

template <typename T, uint32_t capacity> 
class RingBuffer
{
	std::array<T, capacity> buffer;
	T default_value;
	uint32_t head;
	uint32_t tail;
	bool full;
	
public:
	RingBuffer(){}
	~RingBuffer(){}
	
	bool enqueue(T item)
	{
		// if buffer is full, throw an error
		if(isFull())
			return false;
		// insert item at back of buffer
		buffer[tail] = item;
		// increment tail
		tail = (tail + 1) % capacity;
		return true;
	}
	
	T dequeue()
	{
		// if buffer is empty, throw an error
		if(isEmpty())
			return default_value;
		// get item at head
		T item = buffer[head];
		// set item at head to be empty
		buffer[head] = default_value;
		// move head foward
		head = (head + 1) % capacity;
		// return item
		return item;
	}
	
	T peek()
	{
		// if buffer is empty, throw an error
		if(isEmpty())
			return default_value;
		// get item at head
		T item = buffer[head];
		
		return item;
	}
	
		
	std::array<T, capacity>& getRawBuffer()
	{
		return buffer;
	}
	
	bool isFull()
	{ 
		return tail == (head - 1) % capacity; 
	}
	
	bool isEmpty()
	{ 
		return head == tail; 
	}
	
	T front()
	{
		return buffer[head]; 
	}
	
	uint32_t size()
	{
		if (tail >= head)
			return tail - head;
		return capacity - head - tail;
	}
	
	uint32_t getCapacity()
	{ 
		return capacity; 
	}
		
	uint32_t getHeadIndex()
	{ 
		return head; 
	}
	
		
	uint32_t getTailIndex()
	{ 
		return tail; 
	}
	
	void clear()
	{
		while (!isEmpty())
		{
			dequeue();
		}
		head = 0;
		tail = 0;
	}
		
};

#endif