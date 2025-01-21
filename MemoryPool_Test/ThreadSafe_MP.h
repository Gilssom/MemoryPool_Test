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
    vector<T*> m_Pool;      // �޸� Ǯ�� ���ͷ� ���� (LIFO ���)
    size_t m_Capacity;      // Ǯ ũ��
    mutable mutex m_Mutex;  // ����ȭ�� ���� ���ؽ�

public:
    T* Allocate();
    void DeAllocate(T* obj);

public:
    void TestMemoryPool();

public:
    ThreadSafe_MP(size_t capacity);
    ~ThreadSafe_MP();
};

// �ʱ� �뷮��ŭ ��ü�� �̸� �Ҵ�
template<typename T>
ThreadSafe_MP<T>::ThreadSafe_MP(size_t capacity) : m_Capacity(capacity)
{
    lock_guard<mutex> lock(m_Mutex);  // �ʱ�ȭ �� ����ȭ
    m_Pool.reserve(capacity);

    for (size_t i = 0; i < capacity; i++)
    {
        m_Pool.push_back(new T());
    }
}

// ��� ��ü ����
template<typename T>
ThreadSafe_MP<T>::~ThreadSafe_MP()
{
    lock_guard<mutex> lock(m_Mutex);

    for (T* ptr : m_Pool)
    {
        delete ptr;
    }
}

// ��ü �Ҵ� (������ ����)
template<typename T>
T* ThreadSafe_MP<T>::Allocate()
{
    lock_guard<mutex> lock(m_Mutex);

    if (m_Pool.empty())
    {
        return new T();  // Ǯ�� ��ü�� ������ ���� �Ҵ�
    }

    T* obj = m_Pool.back();
    m_Pool.pop_back();
    return obj;
}

// ��ü ��ȯ (������ ����)
template<typename T>
void ThreadSafe_MP<T>::DeAllocate(T* obj)
{
    lock_guard<mutex> lock(m_Mutex);
    m_Pool.push_back(obj);
}

// ��Ƽ������ �޸� Ǯ ���� �׽�Ʈ
template<typename T>
void ThreadSafe_MP<T>::TestMemoryPool()
{
    const int TEST_SIZE = 1000000;

    // �Ϲ� ���� �Ҵ� �׽�Ʈ
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
    

    // ��Ƽ������ �޸� Ǯ �׽�Ʈ
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