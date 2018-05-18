/*
 *  (c) Copyright 2016-2017 Hewlett Packard Enterprise Development Company LP.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the
 *  Application containing code generated by the Library and added to the
 *  Application during this compilation process under terms of your choice,
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <gtest/gtest.h>

#include "radixtree/split_ordered.h"

#include "nvmm/memory_manager.h"
#include "nvmm/epoch_manager.h"
#include "nvmm/log.h"
#include "nvmm/heap.h"
#include "nvmm/fam.h"

using namespace radixtree;
using namespace nvmm;

void print_kv(SplitOrderedList* so, SplitOrderedList::ByteKey key, size_t key_size, SplitOrderedList::Value val)
{
    Gptr val_ptr = val;
    std::cout << key << ": " << val << std::endl;
}

void print_kv2(SplitOrderedList* so, SplitOrderedList::ByteKey key, size_t key_size, SplitOrderedList::Value val)
{
    Gptr val_ptr = val;
    if (val != 0) {
        std::cout << key << ": " << *so->toLocal<uint64_t>(val) << std::endl;
    } else {
        //std::cout << key << ": " << (void*) so->toLocal<uint64_t>(val) << std::endl;
    }
}

TEST(SplitOrderedList, SingleProcess) {
    PoolId const heap_id = 1; // assuming we only use heap id 1
    size_t const heap_size = 1024*1024*1024; // 1024MB

    // init memory manager and heap
    MemoryManager *mm = MemoryManager::GetInstance();
    Heap *heap = nullptr;
    EXPECT_EQ(NO_ERROR, mm->CreateHeap(heap_id, heap_size));
    EXPECT_EQ(NO_ERROR, mm->FindHeap(heap_id, &heap));
    EXPECT_NE(nullptr, heap);

    EpochManager *em = EpochManager::GetInstance();

    // open the heap
    EXPECT_EQ(NO_ERROR, heap->Open());

    // Global pointer to the split ordered list to be used across instantiations
    GlobalPtr sod;

    // begin a new lexical scope so that epoch terminates when we exit scope
    { 
        // create a new list
        SplitOrderedList *so = new SplitOrderedList(mm, heap, NULL);
        EXPECT_NE(nullptr, so);
        sod = so->get_descriptor();

        delete so;
    }

    // begin a new lexical scope so that epoch terminates when we exit scope
    { 
        // open an existing list
        SplitOrderedList *so = new SplitOrderedList(mm, heap, NULL, sod);
        EXPECT_NE(nullptr, so);

        EpochOp op(em);

        SplitOrderedList::ByteKey key1 = "9";
        SplitOrderedList::ByteKey key2 = "10";
        SplitOrderedList::ByteKey key3 = "11";
        so->Insert(op, key1, strlen(key1)+1, 9);
        so->Insert(op, key2, strlen(key2)+1, 10);
        so->Insert(op, key3, strlen(key3)+1, 11);

        so->foreach(print_kv);

        delete so;
    }

    // begin a new lexical scope so that epoch terminates when we exit scope
    { 
        // open an existing list
        SplitOrderedList* so = new SplitOrderedList(mm, heap, NULL, sod);
        EXPECT_NE(nullptr, so);

        EpochOp op(em);

        SplitOrderedList::ByteKey key1 = "9";
        SplitOrderedList::ByteKey key2 = "10";
        SplitOrderedList::ByteKey key3 = "11";
        EXPECT_EQ(9LLU, so->Find(op, key1, strlen(key1)+1));
        EXPECT_EQ(10LLU, so->Find(op, key2, strlen(key2)+1));
        EXPECT_EQ(11LLU, so->Find(op, key3, strlen(key3)+1));
 
        delete so;
    }


    // begin a new lexical scope so that epoch terminates when we exit scope
    { 
        // open an existing list
        SplitOrderedList* so = new SplitOrderedList(mm, heap, NULL, sod);
        EXPECT_NE(nullptr, so);

        EpochOp op(em);

        SplitOrderedList::ByteKey key1 = "9";
        EXPECT_EQ(1, so->Delete(op, key1, strlen(key1)+1, NULL));
        EXPECT_EQ(0LLU, so->Find(op, key1, strlen(key1)+1));
 
        delete so;
    }


#if 0
    // destroy 1
    key = 1;
    key_size = sizeof(key);
    memcpy((char*)&key_buf, (char*)&key, key_size);
    result = tree->destroy(key_buf, key_size);
    EXPECT_EQ(0UL, result);

    // put 1:1
    key = 1;
    key_size = sizeof(key);
    memcpy((char*)&key_buf, (char*)&key, key_size);

    value = 1;
    value_gptr = heap->Alloc(sizeof(value));
    value_ptr = (uint64_t*)mm->GlobalToLocal(value_gptr);
    *value_ptr = value;
    fam_persist(value_ptr, sizeof(value));

    success = tree->put(key_buf, key_size, value_gptr);
    EXPECT_EQ(true, success);

    // get 1
    key = 1;
    key_size = sizeof(key);
    memcpy((char*)&key_buf, (char*)&key, key_size);
    result = tree->get(key_buf, key_size);
    EXPECT_EQ(value_gptr, result);

    // destroy 1
    key = 1;
    key_size = sizeof(key);
    memcpy((char*)&key_buf, (char*)&key, key_size);
    result = tree->destroy(key_buf, key_size);
    EXPECT_EQ(value_gptr, result);
    heap->Free(result);

    // get 1
    key = 1;
    key_size = sizeof(key);
    memcpy((char*)&key_buf, (char*)&key, key_size);
    result = tree->get(key_buf, key_size);
    EXPECT_EQ(0UL, result);

#endif

    EXPECT_EQ(NO_ERROR, heap->Close());
    EXPECT_EQ(NO_ERROR, mm->DestroyHeap(heap_id));
}

// multi-process
void DoWork(GlobalPtr descriptor, PoolId heap_id)
{
    // =======================================================================
    // reset epoch manager after fork()
    EpochManager *em = EpochManager::GetInstance();
    em->Start();

    // =======================================================================
    // init memory manager and heap
    MemoryManager *mm = MemoryManager::GetInstance();
    Heap *heap = nullptr;
    EXPECT_EQ(NO_ERROR, mm->FindHeap(heap_id, &heap));
    EXPECT_NE(nullptr, heap);

    // open the heap
    EXPECT_EQ(NO_ERROR, heap->Open());

    // init the split ordered list
    SplitOrderedList *so = nullptr;
    so = new SplitOrderedList(mm, heap, NULL, descriptor);
    EXPECT_NE(nullptr, so);

    // =======================================================================
    // stress test

#if 0
    pid_t pid = getpid();

    SplitOrderedList::ByteKey key_buf;
    int key_size;
    memset(&key_buf, 0, sizeof(key_buf));
    uint64_t key, value;
    uint64_t *value_ptr;
    GlobalPtr value_gptr, result;

    GlobalPtr ptr;
    int count = 500;
    for (int i=0; i<count; i++)
    {
        key = i;
        key_size = sizeof(key);
        memcpy((char*)&key_buf, (char*)&key, key_size);

        value = i;
        value_gptr = heap->Alloc(sizeof(value));
        value_ptr = (uint64_t*)mm->GlobalToLocal(value_gptr);
        *value_ptr = value;
        fam_persist(value_ptr, sizeof(value));

        int op=(i+pid)%3;
        if (op==0) {
            EpochOp op(em);
            so->Insert(key_buf, value_gptr);
        }
        else if (op==1) {
            EpochOp op(em);
            so->Find(key_buf);
        }
        else {
            EpochOp op(em);
            Gptr cur_ptr;
            so->Delete(key_buf, &cur_ptr);
            heap->Free(op, cur_ptr);
        }
    }
    std::cout << pid << " DONE" << std::endl;
#endif

    // =======================================================================
    // close the heap
    EXPECT_EQ(NO_ERROR, heap->Close());
    delete heap;
    delete so;
}

TEST(SplitOrderedList, MultiProcess) {
    int const process_count = 16;
    PoolId const heap_id = 1; // assuming we only use heap id 1
    size_t const heap_size = 1024*1024*1024; // 1024MB

    // =======================================================================
    // init memory manager and heap
    MemoryManager *mm = MemoryManager::GetInstance();
    Heap *heap = nullptr;
    EXPECT_EQ(NO_ERROR, mm->CreateHeap(heap_id, heap_size));
    EXPECT_EQ(NO_ERROR, mm->FindHeap(heap_id, &heap));
    EXPECT_NE(nullptr, heap);

    // open the heap
    EXPECT_EQ(NO_ERROR, heap->Open());

    // init the radix tree
    SplitOrderedList *so = nullptr;
    GlobalPtr sod;
    // create a new radix tree
    so = new SplitOrderedList(mm, heap, NULL);
    sod = so->get_descriptor();
    EXPECT_NE(nullptr, so);
    delete so;

    // close the heap before reset epoch manager
    EXPECT_EQ(NO_ERROR, heap->Close());

    // =======================================================================
    // reset epoch manager before fork()
    EpochManager *em = EpochManager::GetInstance();
    em->Stop();

    // =======================================================================
    // do work
    pid_t pid[process_count];

    for (int i=0; i< process_count; i++)
    {
        pid[i] = fork();
        ASSERT_LE(0, pid[i]);
        if (pid[i]==0)
        {
            // child
            DoWork(sod, heap_id);
            exit(0); // this will leak memory (see valgrind output)
        }
        else
        {
            // parent
            continue;
        }
    }

    for (int i=0; i< process_count; i++)
    {
        int status;
        waitpid(pid[i], &status, 0);
    }

    so = new SplitOrderedList(mm, heap, NULL, sod);
    so->foreach(print_kv2);
    delete so;

    // =======================================================================
    // destroy the heap
    EXPECT_EQ(NO_ERROR, mm->DestroyHeap(heap_id));

    // =======================================================================
    // reset epoch manager after fork() for the main process
    em->Start();
}





void InitTest(boost::log::trivial::severity_level level, bool to_console)
{
    // init boost::log
    if (to_console == true)
    {
        nvmm::init_log(level, "");
    }
    else
    {
        nvmm::init_log(level, "mm.log");
    }

    nvmm::ResetNVMM();
    nvmm::StartNVMM();
}

int main (int argc, char** argv) {
    InitTest(boost::log::trivial::severity_level::debug, true);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
