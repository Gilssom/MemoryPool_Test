#pragma once

#include "global.h"

struct v_TestStruct
{
	int a, b, c;
};

template <typename T>
class Vector_MP
{
private:
	vector<T*> m_Pool;		// 객체 저장 vector
	size_t m_Capacity;		// pool 용량

public:
	T* Allocate();
	void DeAllocate(T* obj);

public:
	void TestMemoryPool();

public:
	Vector_MP(size_t capacity);
	~Vector_MP();
};

// 초기 용량 설정 및 미리 메모리 할당
template<typename T>
Vector_MP<T>::Vector_MP(size_t capacity) : m_Capacity(capacity)
{
	m_Pool.reserve(capacity);

	for (size_t i = 0; i < capacity; i++)
	{
		m_Pool.push_back(new T());
	}
}

// 할당된 모든 메모리 해체
template<typename T>
Vector_MP<T>::~Vector_MP()
{
	for (T* ptr : m_Pool)
	{
		delete ptr;
	}
}

// 객체 할당
template<typename T>
T* Vector_MP<T>::Allocate()
{
	if (m_Pool.empty())
	{
		return new T();
	}

	T* obj = m_Pool.back();
	m_Pool.pop_back();

	return obj;
}

// 객체 반환
template<typename T>
void Vector_MP<T>::DeAllocate(T* obj)
{
	m_Pool.push_back(obj);
}

template<typename T>
void Vector_MP<T>::TestMemoryPool()
{
	const int TEST_SIZE = 1000000;

	// 일반 동적 할당 테스트
	auto start = high_resolution_clock::now();
	vector<v_TestStruct*> normalObjects;

	for (int i = 0; i < TEST_SIZE; i++)
	{
		normalObjects.push_back(new v_TestStruct());
	}

	for (v_TestStruct* obj : normalObjects)
	{
		delete obj;
	}

	auto end = high_resolution_clock::now();

	cout << "Normal Allocation Time : " << duration<double, milli>(end - start).count() << "ms\n";


	// Vector Memory Pool 사용 테스트
	Vector_MP<v_TestStruct> pool(TEST_SIZE);
	start = high_resolution_clock::now();
	vector<v_TestStruct*> poolObjects;

	for (int i = 0; i < TEST_SIZE; i++)
	{
		poolObjects.push_back(pool.Allocate());
	}

	for (v_TestStruct* obj : poolObjects)
	{
		pool.DeAllocate(obj);
	}

	end = high_resolution_clock::now();

	cout << "Vector Memory Pool Allocation Time : " << duration<double, milli>(end - start).count() << "ms\n";
}