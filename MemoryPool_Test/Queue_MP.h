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

// �̸� ��ü �����Ͽ� ť�� ����
template<typename T>
Queue_MP<T>::Queue_MP(size_t capacity) : m_Capacity(capacity)
{
    for (size_t i = 0; i < capacity; i++)
    {
        m_Pool.push(new T());
    }
}

// ť�� �ִ� ��ü �޸� ����
template<typename T>
Queue_MP<T>::~Queue_MP()
{
    while (!m_Pool.empty())
    {
        delete m_Pool.front();
        m_Pool.pop();
    }
}

// �޸� Ǯ���� ��ü �Ҵ� (���Լ��� ���)
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

// ����� ��ü�� �ٽ� Ǯ�� ��ȯ
template<typename T>
void Queue_MP<T>::DeAllocate(T* obj)
{
    m_Pool.push(obj);
}

// �޸� Ǯ ���� �׽�Ʈ �Լ�
template<typename T>
void Queue_MP<T>::TestMemoryPool()
{
    const int TEST_SIZE = 1000000;

    // �Ϲ� ���� �Ҵ� ���� �׽�Ʈ
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


    // ť �޸� Ǯ ���� �׽�Ʈ
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