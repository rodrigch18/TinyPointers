extern "C" {
    #include "tiny_ptr_unified.h"
}
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

// Test 1: Operations on a NULL table.
TEST(TinyPtrSimple, NullTableOperations) {
    EXPECT_EQ(tiny_ptr_allocate(nullptr, 123, 456), -1);
    EXPECT_EQ(tiny_ptr_dereference(nullptr, 123, 0), -1);
    // tiny_ptr_free is void; calling it on a null table should not crash.
    tiny_ptr_free(nullptr, 123, 0);
}

// Test 2: Basic allocation, dereference and free.
TEST(TinyPtrSimple, BasicAllocation) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    for (int i = 0; i < 100; i++) {
        int key = i + 1000;
        int value = key * 10;
        int tp = tiny_ptr_allocate(table, key, value);
        EXPECT_NE(tp, -1) << "Allocation failed for key " << key;
        int ret = tiny_ptr_dereference(table, key, tp);
        EXPECT_EQ(ret, value) << "Dereference mismatch for key " << key;
        tiny_ptr_free(table, key, tp);
        ret = tiny_ptr_dereference(table, key, tp);
        EXPECT_EQ(ret, 0) << "Slot not reset after free for key " << key;
    }
    tiny_ptr_destroy(table);
}

// Test 3: Multiple allocations with the same key.
TEST(TinyPtrSimple, MultipleAllocationsSameKey) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 5000;
    int value1 = 123, value2 = 456;
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

// Test 4: Allocate until full and then free.
TEST(TinyPtrSimple, AllocateUntilFull) {
    size_t capacity = 64; // small capacity to force failure quickly
    tiny_ptr_table_t* table = tiny_ptr_create(capacity, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    std::vector<int> allocated;
    int key = 1000, value = 10;
    while (true) {
        int tp = tiny_ptr_allocate(table, key, value);
        if (tp == -1) break;
        allocated.push_back(tp);
        key++;
        value += 10;
    }
    EXPECT_GT(allocated.size(), 0u);
    key = 1000;
    for (int tp : allocated) {
        tiny_ptr_free(table, key, tp);
        key++;
    }
    int new_tp = tiny_ptr_allocate(table, 9999, 99990);
    EXPECT_NE(new_tp, -1);
    tiny_ptr_destroy(table);
}

// Test 5: Resize test (SIMPLE variant only).
TEST(TinyPtrSimple, ResizeTest) {
    size_t capacity = 128;
    tiny_ptr_table_t* table = tiny_ptr_create(capacity, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    int half = capacity / 2;
    for (int i = 0; i < half; i++) {
        int key = i + 2000;
        int value = key * 10;
        int tp = tiny_ptr_allocate(table, key, value);
        EXPECT_NE(tp, -1);
    }
    EXPECT_EQ(tiny_ptr_resize(&table, capacity * 2), 0);
    for (int i = half; i < (int)capacity; i++) {
        int key = i + 2000;
        int value = key * 10;
        int tp = tiny_ptr_allocate(table, key, value);
        EXPECT_NE(tp, -1);
        EXPECT_EQ(tiny_ptr_dereference(table, key, tp), value);
        tiny_ptr_free(table, key, tp);
    }
    tiny_ptr_destroy(table);
}

// Test 6: Multi-threaded operations.
TEST(TinyPtrSimple, MultiThreaded) {
    size_t capacity = 10000;
    tiny_ptr_table_t* table = tiny_ptr_create(capacity, TINY_PTR_SIMPLE, 0.9);
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

// Test 7: Reallocation after free.
TEST(TinyPtrSimple, ReallocateAfterFree) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 3000, value1 = 111, value2 = 222;
    int tp = tiny_ptr_allocate(table, key, value1);
    ASSERT_NE(tp, -1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp), value1);
    tiny_ptr_free(table, key, tp);
    int tp_new = tiny_ptr_allocate(table, key, value2);
    EXPECT_NE(tp_new, -1);
    EXPECT_EQ(tiny_ptr_dereference(table, key, tp_new), value2);
    tiny_ptr_destroy(table);
}

// Test 8: Double free should not crash.
TEST(TinyPtrSimple, DoubleFree) {
    tiny_ptr_table_t* table = tiny_ptr_create(1024, TINY_PTR_SIMPLE, 0.9);
    ASSERT_NE(table, nullptr);
    int key = 4000, value = 999;
    int tp = tiny_ptr_allocate(table, key, value);
    ASSERT_NE(tp, -1);
    tiny_ptr_free(table, key, tp);
    // Second free; while undefined behavior, ensure no crash.
    tiny_ptr_free(table, key, tp);
    tiny_ptr_destroy(table);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
