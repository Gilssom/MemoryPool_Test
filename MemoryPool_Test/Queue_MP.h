#pragma once

#include "global.h"

struct q_TestStruct
{
    int a, b, c;
};

template <typename T>
class Queue_MP
{
private:
    queue<T*> m_Pool;  
    size_t m_Capacity; 

public:
    T* Allocate();
    void DeAllocate(T* obj);

public:
    void TestMemoryPool();

public:
    Queue_MP(size_t capacity);
    ~Queue_MP();
};

// 미리 객체 생성하여 큐에 저장
template<typename T>
Queue_MP<T>::Queue_MP(size_t capacity) : m_Capacity(capacity)
{
    for (size_t i = 0; i < capacity; i++)
    {
        m_Pool.push(new T());
    }
}

// 큐에 있는 객체 메모리 해제
template<typename T>
Queue_MP<T>::~Queue_MP()
{
    while (!m_Pool.empty())
    {
        delete m_Pool.front();
        m_Pool.pop();
    }
}

// 메모리 풀에서 객체 할당 (선입선출 방식)
template<typename T>
T* Queue_MP<T>::Allocate()
{
    if (m_Pool.empty())
    {
        return new T(); 
    }

    T* obj = m_Pool.front();
    m_Pool.pop();
    return obj;
}

// 사용한 객체를 다시 풀에 반환
template<typename T>
void Queue_MP<T>::DeAllocate(T* obj)
{
    m_Pool.push(obj);
}

// 메모리 풀 성능 테스트 함수
template<typename T>
void Queue_MP<T>::TestMemoryPool()
{
    const int TEST_SIZE = 1000000;

    // 일반 동적 할당 성능 테스트
    auto start = high_resolution_clock::now();
    vector<q_TestStruct*> normalObjects;

    for (int i = 0; i < TEST_SIZE; i++)
    {
        normalObjects.push_back(new q_TestStruct());
    }

    for (q_TestStruct* obj : normalObjects)
    {
        delete obj;
    }

    auto end = high_resolution_clock::now();
    cout << "Normal Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";


    // 큐 메모리 풀 성능 테스트
    Queue_MP<q_TestStruct> pool(TEST_SIZE);
    start = high_resolution_clock::now();
    vector<q_TestStruct*> poolObjects;

    for (int i = 0; i < TEST_SIZE; i++)
    {
        poolObjects.push_back(pool.Allocate());
    }

    for (q_TestStruct* obj : poolObjects)
    {
        pool.DeAllocate(obj);
    }

    end = high_resolution_clock::now();

    cout << "Queue Memory Pool Allocation Time : "  << duration<double, milli>(end - start).count() << " ms\n";
}