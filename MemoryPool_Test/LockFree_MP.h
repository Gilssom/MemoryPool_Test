#pragma once

#include "global.h"

#include <atomic>
#include <memory>
#include <thread>

struct alignas(64) L_TestStruct
{
    int a, b, c;
};

template <typename T>
class LockFree_MP
{
private:
    struct Node 
    {
        T* data;
        Node* next;
    };

    atomic<Node*> m_Head;               // 스택의 top 포인터 (atomic)
    vector<unique_ptr<T>> m_Storage;    // 메모리 객체 저장소
    size_t m_Capacity;                  // 풀의 크기

private:
    void InitializePool();

public:
    T* Allocate();
    void Deallocate(T* obj);

public:
    void TestMemoryPool();

public:
    LockFree_MP(size_t capacity);
    ~LockFree_MP();
};

// 초기 풀 크기 설정 및 객체 생성
template <typename T>
LockFree_MP<T>::LockFree_MP(size_t capacity) : m_Capacity(capacity)
{
    InitializePool();
}

// 모든 객체 해제
template <typename T>
LockFree_MP<T>::~LockFree_MP()
{
    Node* node = m_Head.exchange(nullptr, memory_order_acquire);

    while (node)
    {
        Node* next = node->next;
        delete node->data;
        delete node;
        node = next;
    }
}

// 풀 초기화 (객체 미리 생성)
template <typename T>
void LockFree_MP<T>::InitializePool()
{
    Node* newHead = nullptr;

    try
    {
        for (size_t i = 0; i < m_Capacity; ++i)
        {
            T* obj = new T();
            Node* node = new Node{ obj, newHead };
            newHead = node;
            m_Storage.emplace_back(unique_ptr<T>(obj));
        }
    }
    catch (const std::bad_alloc& e)
    {
        cerr << "Memory allocation failed: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    m_Head.store(newHead, memory_order_release);
}

// 객체 할당 (Lock-Free)
template <typename T>
T* LockFree_MP<T>::Allocate()
{
    Node* node = nullptr;
    do
    {
        node = m_Head.load(memory_order_acquire);
    } 
    while (node && !m_Head.compare_exchange_weak(node, node->next,
                                                memory_order_release,
                                                memory_order_relaxed));

    if (node)
    {
        return node->data;
    }

    static atomic<int> warning_count = 0;
    if (warning_count++ < 10)  // 최대 10회 경고 출력
    {
        cerr << "Warning: Pool exhausted. Creating new object.\n";
    }

    return new T();
}

// 객체 반환 (Lock-Free)
template <typename T>
void LockFree_MP<T>::Deallocate(T* obj)
{
    if (!obj)
        return;  // null 보호

    Node* newNode = new Node{ obj, nullptr };

    do
    {
        newNode->next = m_Head.load(memory_order_acquire);
    } 
    while (!m_Head.compare_exchange_weak(newNode->next, newNode,
                                            memory_order_release,
                                            memory_order_relaxed));
}

template <typename T>
void LockFree_MP<T>::TestMemoryPool()
{
    const int TEST_SIZE = 1000000;

    // 일반 new/delete 할당 테스트
    auto start = high_resolution_clock::now();
    vector<L_TestStruct*> normalObjects;

    for (int i = 0; i < TEST_SIZE; ++i)
    {
        normalObjects.push_back(new L_TestStruct());
    }

    for (L_TestStruct* obj : normalObjects)
    {
        delete obj;
    }

    auto end = high_resolution_clock::now();

    cout << "Normal Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";


    // 락프리 메모리 풀 할당 테스트
    LockFree_MP<L_TestStruct> pool(TEST_SIZE);
    start = high_resolution_clock::now();
    vector<L_TestStruct*> poolObjects;

    for (int i = 0; i < TEST_SIZE; ++i)
    {
        poolObjects.push_back(pool.Allocate());
    }

    for (L_TestStruct* obj : poolObjects)
    {
        pool.Deallocate(obj);
    }

    end = high_resolution_clock::now();

    cout << "Lock Free Memory Pool Allocation Time : " << duration<double, milli>(end - start).count() << " ms\n";
}