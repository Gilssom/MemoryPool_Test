#pragma once

#include "global.h"

#include <mutex>

struct t_TestStruct
{
    int a, b, c;
};

template <typename T>
class ThreadSafe_MP
{
private:
    vector<T*> m_Pool;      // 메모리 풀을 벡터로 관리 (LIFO 방식)
    size_t m_Capacity;      // 풀 크기
    mutable mutex m_Mutex;  // 동기화를 위한 뮤텍스

public:
    T* Allocate();
    void DeAllocate(T* obj);

public:
    void TestMemoryPool();

public:
    ThreadSafe_MP(size_t capacity);
    ~ThreadSafe_MP();
};

// 초기 용량만큼 객체를 미리 할당
template<typename T>
ThreadSafe_MP<T>::ThreadSafe_MP(size_t capacity) : m_Capacity(capacity)
{
    lock_guard<mutex> lock(m_Mutex);  // 초기화 시 동기화
    m_Pool.reserve(capacity);

    for (size_t i = 0; i < capacity; i++)
    {
        m_Pool.push_back(new T());
    }
}

// 모든 객체 해제
template<typename T>
ThreadSafe_MP<T>::~ThreadSafe_MP()
{
    lock_guard<mutex> lock(m_Mutex);

    for (T* ptr : m_Pool)
    {
        delete ptr;
    }
}

// 객체 할당 (스레드 안전)
template<typename T>
T* ThreadSafe_MP<T>::Allocate()
{
    lock_guard<mutex> lock(m_Mutex);

    if (m_Pool.empty())
    {
        return new T();  // 풀에 객체가 없으면 새로 할당
    }

    T* obj = m_Pool.back();
    m_Pool.pop_back();
    return obj;
}

// 객체 반환 (스레드 안전)
template<typename T>
void ThreadSafe_MP<T>::DeAllocate(T* obj)
{
    lock_guard<mutex> lock(m_Mutex);
    m_Pool.push_back(obj);
}

// 멀티스레드 메모리 풀 성능 테스트
template<typename T>
void ThreadSafe_MP<T>::TestMemoryPool()
{
    const int TEST_SIZE = 1000000;

    // 일반 동적 할당 테스트
    auto start = high_resolution_clock::now();
    vector<t_TestStruct*> normalObjects;

    for (int i = 0; i < TEST_SIZE; i++)
    {
        normalObjects.push_back(new t_TestStruct());
    }

    for (t_TestStruct* obj : normalObjects)
    {
        delete obj;
    }

    auto end = high_resolution_clock::now();
    cout << "Normal Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";
    

    // 멀티스레드 메모리 풀 테스트
    ThreadSafe_MP<t_TestStruct> pool(TEST_SIZE);
    start = high_resolution_clock::now();
    vector<t_TestStruct*> poolObjects;

    for (int i = 0; i < TEST_SIZE; i++)
    {
        poolObjects.push_back(pool.Allocate());
    }

    for (t_TestStruct* obj : poolObjects)
    {
        pool.DeAllocate(obj);
    }

    end = high_resolution_clock::now();
    cout << "Thread safe Memory Pool Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";
}