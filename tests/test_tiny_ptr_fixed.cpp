extern "C" {
    #include "tiny_ptr_unified.h"
}
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

// Test 1: Operations on a NULL table.
TEST(TinyPtrFixed, NullTableOperations) {
    EXPECT_EQ(tiny_ptr_allocate(nullptr, 321, 654), -1);
    EXPECT_EQ(tiny_ptr_dereference(nullptr, 321, 0), -1);
    tiny_ptr_free(nullptr, 321, 0);
}

// Test 2: Basic allocation, dereference and free.
TEST(TinyPtrFixed, BasicAllocation) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    for (int i = 0; i < 100; i++) {
        int key = i + 1100;
        int value = key * 10;
        int tp = tiny_ptr_allocate(table, key, value);
        EXPECT_NE(tp, -1) << "Allocation failed for key " << key;
        EXPECT_EQ(tiny_ptr_dereference(table, key, tp), value)
            << "Dereference mismatch for key " << key;
        tiny_ptr_free(table, key, tp);
        EXPECT_EQ(tiny_ptr_dereference(table, key, tp), 0)
            << "Slot not reset after free for key " << key;
    }
    tiny_ptr_destroy(table);
}

// Test 3: Multiple allocations with the same key.
TEST(TinyPtrFixed, MultipleAllocationsSameKey) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 5500;
    int value1 = 321, value2 = 654;
    int tp1 = tiny_ptr_allocate(table, key, value1);
    EXPECT_NE(tp1, -1);
    int tp2 = tiny_ptr_allocate(table, key, value2);
    EXPECT_NE(tp2, -1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp1), value1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp2), value2);
    tiny_ptr_free(table, key, tp1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp1), 0);
    tiny_ptr_free(table, key, tp2);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp2), 0);
    tiny_ptr_destroy(table);
}

// Test 4: Allocate until full.
TEST(TinyPtrFixed, AllocateUntilFull) {
    size_t capacity = 64;
    tiny_ptr_table_t* table = tiny_ptr_create(capacity, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    std::vector<int> allocated;
    int key = 2000, value = 20;
    while (true) {
        int tp = tiny_ptr_allocate(table, key, value);
        if (tp == -1) break;
        allocated.push_back(tp);
        key++;
        value += 20;
    }
    EXPECT_GT(allocated.size(), 0u);
    key = 2000;
    for (int tp : allocated) {
        tiny_ptr_free(table, key, tp);
        key++;
    }
    int new_tp = tiny_ptr_allocate(table, 8888, 88880);
    EXPECT_NE(new_tp, -1);
    tiny_ptr_destroy(table);
}

// Test 5: Multi-threaded operations.
TEST(TinyPtrFixed, MultiThreaded) {
    size_t capacity = 10000;
    tiny_ptr_table_t* table = tiny_ptr_create(capacity, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    const int num_threads = 4, allocs_per_thread = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> failures(0);
    auto threadFunc = [table, allocs_per_thread, &failures](int start_key) {
        for (int i = 0; i < allocs_per_thread; i++) {
            int key = start_key + i;
            int value = key * 10;
            int tp = tiny_ptr_allocate(table, key, value);
            if (tp == -1) { failures++; continue; }
            if (tiny_ptr_dereference(table, key, tp) != value) { failures++; }
            tiny_ptr_free(table, key, tp);
        }
    };
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(threadFunc, i * allocs_per_thread);
    }
    for (auto& t : threads) { t.join(); }
    EXPECT_EQ(failures.load(), 0);
    tiny_ptr_destroy(table);
}

// Test 6: Reallocation after free.
TEST(TinyPtrFixed, ReallocateAfterFree) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 3500, value1 = 777, value2 = 888;
    int tp = tiny_ptr_allocate(table, key, value1);
    ASSERT_NE(tp, -1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp), value1);
    tiny_ptr_free(table, key, tp);
    int tp_new = tiny_ptr_allocate(table, key, value2);
    EXPECT_NE(tp_new, -1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp_new), value2);
    tiny_ptr_destroy(table);
}

// Test 7: Double free should not crash.
TEST(TinyPtrFixed, DoubleFree) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_FIXED, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 4500, value = 999;
    int tp = tiny_ptr_allocate(table, key, value);
    ASSERT_NE(tp, -1);
    tiny_ptr_free(table, key, tp);
    tiny_ptr_free(table, key, tp);
    tiny_ptr_destroy(table);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
