#include "Vector_MP.h"
#include "Queue_MP.h"
#include "ThreadSafe_MP.h"
#include "LockFree_MP.h"

#include <iostream>
#include <thread>
#include <functional>

thread_local ThreadSafe_MP<t_TestStruct> tlsPool(100000);

void Func(int iter)
{
	for (int i = 0; i < iter; i++)
	{
		t_TestStruct* obj = tlsPool.Allocate();

		obj->a = i;
		obj->b = i * 2;
		obj->c = i * 3;
		tlsPool.DeAllocate(obj);
	}
}

thread_local LockFree_MP<L_TestStruct> LlsPool(25000);

void L_Func(int iter)
{
	constexpr int BATCH_SIZE = 100;
	vector<L_TestStruct*> batch;
	batch.reserve(BATCH_SIZE);

	for (int i = 0; i < iter / BATCH_SIZE; i++)
	{
		for (int j = 0; j < BATCH_SIZE; j++)
		{
			L_TestStruct* obj = LlsPool.Allocate();
			obj->a = j;
			obj->b = j * 2;
			obj->c = j * 3;
			batch.push_back(obj);
		}

		for (auto obj : batch)
		{
			LlsPool.Deallocate(obj);
		}
		batch.clear();
	}
}

int main()
{
	cout << "-------------------------------------\n";
	cout << "Test Vector Memory Allocation Time\n";
	cout << "-------------------------------------\n";

	// Vector Memory Pool Test, Block Cnt = 100,000
	Vector_MP<v_TestStruct> testVecPool(100000);
	testVecPool.TestMemoryPool();

	cout << '\n' << '\n';

	cout << "------------------------------------\n";
	cout << "Test Queue Memory Allocation Time\n";
	cout << "------------------------------------\n";

	// Queue Memory Pool Test, Block Cnt = 100,000
	Queue_MP<q_TestStruct> testQueuePool(100000);
	testQueuePool.TestMemoryPool();

	cout << '\n' << '\n';

	cout << "------------------------------------------------------\n";
	cout << "Test Thread Safe Memory Allocation Time ( 4 thread )\n";
	cout << "------------------------------------------------------\n";

	// Thread Safe Memory Pool Test, Block Cnt = 100,000
	ThreadSafe_MP<t_TestStruct> testThreadPool(100000);
	testThreadPool.TestMemoryPool();

	// Multi Thread Memory Pool Test, Thread = 4
	const int numThread = 4;
	const int iterPerThread = 25000;

	vector<thread> threads;
	threads.reserve(numThread);

	auto start = high_resolution_clock::now();

	for (int i = 0; i < numThread; i++)
	{
		threads.emplace_back(Func, iterPerThread);
	}

	for (auto& t : threads)
	{
		t.join();
	}

	auto end = high_resolution_clock::now();

	cout << "Multi Threads Memory Pool Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";

	cout << '\n' << '\n';

	cout << "----------------------------------------------------\n";
	cout << "Test Lock Free Memory Allocation Time ( 4 thread )\n";
	cout << "----------------------------------------------------\n";

	// Lock Free Memory Pool Test, Block Cnt = 100,000
	LockFree_MP<L_TestStruct> testLockPool(100000);
	testLockPool.TestMemoryPool();

	// Multi Thread Lock Free Memory Pool Test, Thread = 4
	const int L_numThreads = 4;
	const int L_iterPerThread = 25000;
	
	vector<thread> L_threads;
	L_threads.reserve(L_numThreads);
	
	auto L_start = high_resolution_clock::now();
	
	for (int i = 0; i < L_numThreads; i++)
	{
		L_threads.emplace_back(L_Func, L_iterPerThread);
	}
	
	for (auto& t : L_threads)
	{
		t.join();
	}
	
	auto L_end = high_resolution_clock::now();
	
	cout << "Multi Threads Lock Free Memory Pool Time: " << duration<double, milli>(L_end - L_start).count() << " ms\n";

	return 0;
}