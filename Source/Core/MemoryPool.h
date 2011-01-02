#pragma once

// MEMORY POOL ALLOCATOR
class MemoryPool
{
private:
	// size of blocks in the pool
	size_t mSize;

	// number of blocks to add on start
	size_t mStart;

	// number of blocks to add when out of blocks
	size_t mGrow;

	// pointer to list of allocated chunks
	void *mChunk;

	// pointer to list of free blocks
	void *mFree;

private:
	// grow the memory pool
	void Grow(size_t aCount);

public:
	// default constructor
	GAME_API MemoryPool(void);

	// consructor
	GAME_API MemoryPool(size_t aSize, size_t aStart = 256, size_t aGrow = 16);

	// destructor
	GAME_API ~MemoryPool();

	// setup
	GAME_API void Setup(size_t aSize, size_t aStart, size_t aGrow);

	// initialize
	GAME_API void Init(void);

	// cleanup
	GAME_API void Clean(void);

	// allocate a block
	GAME_API void * Alloc(void);

	// free a block
	GAME_API void Free(void * aPtr);

	// get block size
	size_t GetSize(void) const
	{
		return mSize;
	}

	// get start block count
	size_t GetStart(void) const
	{
		return mStart;
	}

	// get grow block count
	size_t GetGrow(void) const
	{
		return mGrow;
	}
};
