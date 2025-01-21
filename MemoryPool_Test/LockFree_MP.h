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

    atomic<Node*> m_Head;               // ������ top ������ (atomic)
    vector<unique_ptr<T>> m_Storage;    // �޸� ��ü �����
    size_t m_Capacity;                  // Ǯ�� ũ��

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

// �ʱ� Ǯ ũ�� ���� �� ��ü ����
template <typename T>
LockFree_MP<T>::LockFree_MP(size_t capacity) : m_Capacity(capacity)
{
    InitializePool();
}

// ��� ��ü ����
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

// Ǯ �ʱ�ȭ (��ü �̸� ����)
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

// ��ü �Ҵ� (Lock-Free)
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
    if (warning_count++ < 10)  // �ִ� 10ȸ ��� ���
    {
        cerr << "Warning: Pool exhausted. Creating new object.\n";
    }

    return new T();
}

// ��ü ��ȯ (Lock-Free)
template <typename T>
void LockFree_MP<T>::Deallocate(T* obj)
{
    if (!obj)
        return;  // null ��ȣ

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

    // �Ϲ� new/delete �Ҵ� �׽�Ʈ
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


    // ������ �޸� Ǯ �Ҵ� �׽�Ʈ
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