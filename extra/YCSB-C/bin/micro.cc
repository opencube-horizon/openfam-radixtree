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
#include <random>
#include <limits>
#include <functional>
#include <fstream>
#include <unordered_map>

#include <cereal/archives/binary.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <cache_api.h> // kvs cache API
#include "radixtree/kvs.h"
#include "radixtree/hrtime.h"

using namespace radixtree;
using namespace nvmm;


std::vector<std::string> keys; // pre-generated keys

struct argument {
    uint64_t process_id;
    uint64_t process_cnt;
    std::string cmd;
    std::string kvs_path;
    std::string kvs_type;
    std::string kvs_loc;
    size_t kvs_size;
    size_t cache_size;
    std::string key_type;
    std::string val_type;
    uint64_t key_size; // for regular string: max key len; for decimal and binary string, max key value
    uint64_t val_size; // val_size is always the size of the value
    uint64_t thread_cnt;
    uint64_t record_cnt;
    std::string key_file; // pre-generated keys
};


using GenFunc = std::string (*) (uint64_t, size_t, size_t);


struct thread_argument{
    uint64_t thread_id;
    uint64_t start_record;
    uint64_t record_cnt;
    size_t min_key_len;
    size_t max_key_len;
    size_t min_val_len;
    size_t max_val_len;
    GenFunc key_gen;
    GenFunc val_gen;
    double latency;
};


nvmm::GlobalPtr str2gptr(std::string root_str) {
    std::string delimiter = ":";
    size_t loc = root_str.find(delimiter);
    if (loc==std::string::npos)
        return 0;
    std::string shelf_id = root_str.substr(1, loc-1);
    std::string offset = root_str.substr(loc+1, root_str.size()-3-shelf_id.size());
    return nvmm::GlobalPtr((unsigned char)std::stoul(shelf_id), std::stoul(offset));
}

// key/val generators
std::random_device r;
std::default_random_engine e1(r());
inline uint64_t rand_uint64(uint64_t min, uint64_t max)
{
    std::uniform_int_distribution<uint64_t> uniform_dist(min, max);
    return uniform_dist(e1);
}

inline std::string rand_string(size_t min_len, size_t max_len)
{
    // // random char string
    // static char const dict[] =
    //     "0123456789"
    //     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    //     "abcdefghijklmnopqrstuvwxyz";

    // size_t len = (size_t)rand_uint64(min_len, max_len);
    // std::string ret(len, 0);
    // for (size_t i = 0; i < len; i++)
    // {
    //     ret[i]=dict[rand_uint64(0,sizeof(dict)-2)];
    // }

    // return ret;

    // random binary string
    size_t len = (size_t)rand_uint64(min_len, max_len);
    std::string ret(len, 0);
    for (size_t i = 0; i < len; i++)
    {
        ret[i]=(unsigned char)rand_uint64(0,255);
    }

    return ret;
}

inline std::string rand_char_string(size_t min_len, size_t max_len)
{
    // random char string
    static char const dict[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    size_t len = (size_t)rand_uint64(min_len, max_len);
    std::string ret(len, 0);
    for (size_t i = 0; i < len; i++)
    {
        ret[i]=dict[rand_uint64(0,sizeof(dict)-2)];
    }

    return ret;

    // // random binary string
    // size_t len = (size_t)rand_uint64(min_len, max_len);
    // std::string ret(len, 0);
    // for (size_t i = 0; i < len; i++)
    // {
    //     ret[i]=(unsigned char)rand_uint64(0,255);
    // }

    // return ret;
}

inline std::string const_string(size_t min_len, size_t max_len)
{
    size_t len = (size_t)rand_uint64(min_len, max_len);
    return std::string(len, '0');
}

inline uint64_t str2num(std::string str) {
    uint64_t num;
    memcpy((char*)&num, str.c_str(), str.size());
#ifdef SYS_LITTLE_ENDIAN
    num = __builtin_bswap64(num);
#endif
    return num;
}


// =============================================
// string generators

inline std::string load_string(uint64_t i, size_t min, size_t max) {
    return keys[i-1];
}

/* 
   regular strings (variable size)
*/
// random regular string, variable length
// [min, max]
inline std::string random_string(uint64_t i, size_t min, size_t max) {
    return rand_string(min, max);    
}

// const regular string, variable length
// [min, max]
inline std::string const_string(uint64_t i, size_t min, size_t max) {
    return const_string(min, max);
}

/* 
   decimal strings (variable size, based on integer value)
*/
// uint64 -> decimal string, variable length
// i
inline std::string uint64(uint64_t i, size_t min, size_t max) {
    return std::to_string(i);    
}

// random uint64
inline std::string random_uint64(uint64_t i, size_t min, size_t max) {
    return std::to_string(rand_uint64(0, std::numeric_limits<uint64_t>::max()));    
}

// random uint64 in a range
// [min, max]
inline std::string random_range_uint64(uint64_t i, size_t min, size_t max) {
    return std::to_string(rand_uint64(min, max));    
}


/* 
   binary strings (fixed size, 8B)
*/
// uint64 -> binary string, fixed length (8B)
// i
inline std::string uint64_be(uint64_t i, size_t min, size_t max) {
#ifdef SYS_LITTLE_ENDIAN
    i = __builtin_bswap64(i);
#endif
    return std::string((char*)&i, sizeof(i));
}

// random uint64
// NOT safe for tiny value, because tiny does not allow 0 value
inline std::string random_uint64_be(uint64_t i, size_t min, size_t max) {
    i = rand_uint64(0, std::numeric_limits<uint64_t>::max());
    return uint64_be(i, min, max);
}

// random uint64 in a range
// [min, max]
inline std::string random_range_uint64_be(uint64_t i, size_t min, size_t max) {
    i = rand_uint64(min, max);
    return uint64_be(i, min, max);
}

// little endian
inline std::string uint64_le(uint64_t i, size_t min, size_t max) {
    return std::string((char*)&i, sizeof(i));
}
inline std::string random_uint64_le(uint64_t i, size_t min, size_t max) {
    i = rand_uint64(0, std::numeric_limits<uint64_t>::max());
    return uint64_le(i, min, max);
}
inline std::string random_range_uint64_le(uint64_t i, size_t min, size_t max) {
    i = rand_uint64(min, max);
    return uint64_le(i, min, max);
}


GenFunc getGen(std::string type) {
    // load pre-generated key
    if(type == "load_string") {
        return load_string;
    }
    // random string (variable or fixed length)
    else if(type == "random_string") {
        return random_string;
    }
    // const string (all 0s)
    else if(type == "const_string") {
        return const_string;
    }
    // uint64_t => variable length decimal string
    else if(type == "uint64") {
        return uint64;
    }
    else if(type == "random_uint64") {
        return random_uint64;
    }
    else if(type == "random_range_uint64") {
        return random_range_uint64;
    }
    // uint64_t => fixed length binary string (8B)
    else if(type == "uint64_be") {
        return uint64_be;
    }
    else if(type == "random_uint64_be") {
        return random_uint64_be;
    }
    else if(type == "random_range_uint64_be") {
        return random_range_uint64_be;
    }
    else if(type == "uint64_le") {
        return uint64_le;
    }
    else if(type == "random_uint64_le") {
        return random_uint64_le;
    }
    else if(type == "random_range_uint64_be") {
        return random_range_uint64_be;
    }
    else 
        return NULL;
}


// =============================================

void generate_keys(std::string path, size_t min_key_len, size_t max_key_len, size_t count)
{
    std::unordered_set<std::string> hs;
    int i=0;
    while(i<count) {
        std::string key = rand_string(min_key_len, max_key_len);
        if(hs.count(key)==0) {
            hs.insert(key);            
            i++;
            //std::cout << "key " << key << std::endl;
        }
    }
    assert(hs.size()==count);

    keys = std::vector<std::string>(hs.begin(), hs.end());
    
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    cereal::BinaryOutputArchive oarchive(ofs);
    oarchive(keys);
    ofs.close();
}

void load_keys(std::string path)
{
    std::ifstream ifs;
    ifs.open(path, std::ofstream::in | std::ofstream::binary);
    cereal::BinaryInputArchive iarchive(ifs);
    iarchive(keys);
    ifs.close();
}


void *put_worker(void *thread_arg)
{
    thread_argument *arg = (thread_argument*)thread_arg;
    uint64_t start = arg->start_record;
    uint64_t end = arg->start_record+arg->record_cnt;

    size_t min_key_len = arg->min_key_len;
    size_t max_key_len = arg->max_key_len;
    size_t min_val_len = arg->min_val_len;
    size_t max_val_len = arg->max_val_len;

    //std::cout << "put " << start << " " << end << std::endl;

    HRTime tstart, tend;
    tstart = get_hrtime();
    int ret;
    for (uint64_t i=start; i<end; i++)
    {
        std::string key = arg->key_gen(i, min_key_len, max_key_len);
        std::string val = arg->val_gen(i, min_val_len, max_val_len);
        //std::cout << "Put " << key << " " << val << std::endl;
        //std::cout << "Put " << val.size() << std::endl;
        ret = Cache_Put(key.c_str(), key.size(), val.c_str(), val.size());
        assert(ret==0);
    }
    tend = get_hrtime();
    // latency
    arg->latency = ((double)(diff_hrtime_us(tstart, tend)))/((double)(arg->record_cnt));

    return NULL;
}

void *get_worker(void *thread_arg)
{
    thread_argument *arg = (thread_argument*)thread_arg;
    uint64_t start = arg->start_record;
    uint64_t end = arg->start_record+arg->record_cnt;

    size_t min_key_len = arg->min_key_len;
    size_t max_key_len = arg->max_key_len;
    size_t min_val_len = arg->min_val_len;
    size_t max_val_len = arg->max_val_len;

    //std::cout << "get " << start << " " << end << std::endl;

    size_t val_len=arg->max_val_len;
    char *val_buf = new char[arg->max_val_len];

    HRTime tstart, tend;
    tstart = get_hrtime();
    int ret;
    for (uint64_t i=start; i<end; i++)
    {
        std::string key = arg->key_gen(i, min_key_len, max_key_len);
        val_len = arg->max_val_len;
        ret = Cache_Get(key.c_str(), key.size(), val_buf, val_len);
        // while(ret!=0) {
        //     ret = Cache_Get(key.c_str(), key.size(), val_buf, val_len);
        //     std::cout << "retry ..." << std::endl;
        // }
        //std::cout << "Get " << key <<  " " << ret << std::endl;
        assert(ret==0);
    }
    tend = get_hrtime();
    // latency
    arg->latency = ((double)(diff_hrtime_us(tstart, tend)))/((double)(arg->record_cnt));

    delete val_buf;
    return NULL;
}

void do_work(argument args, thread_argument *thread_args) {
    void *(*worker)(void *);
    if(args.cmd == "put") {
        worker = put_worker;
    }
    else {
        worker = get_worker;
    }

    pthread_t *threads = new pthread_t[args.thread_cnt];
    //thread_argument *thread_args = new thread_argument[args.thread_cnt];

    size_t min_key_len;
    size_t max_key_len;
    size_t min_val_len;
    size_t max_val_len;
    
    if(args.kvs_type.find("tiny") != std::string::npos) {
        // radixtree_tiny
        // val_len == 8
        min_key_len=1;
        max_key_len=args.key_size;

        min_val_len=8;
        max_val_len=8;
    }
    else {
        // radixtree
        // val_len == args.val_size
        min_key_len=1;
        max_key_len=args.key_size;

        min_val_len=args.val_size; // TODO: looks like memcached'd item_alloc requires min_val_len of 2
        max_val_len=args.val_size;
    }

    GenFunc key_gen = getGen(args.key_type);
    GenFunc val_gen = getGen(args.val_type);
        
    int ret=0;
    void *status;

    uint64_t start_record = args.process_id * args.record_cnt+1; // to avoid 0
    uint64_t record_cnt = args.record_cnt/args.thread_cnt;
    uint64_t remainder_records = args.record_cnt - record_cnt*args.thread_cnt;

    for(uint64_t i=0; i<args.thread_cnt; i++) {
        
        thread_args[i].thread_id = i;
        thread_args[i].start_record = start_record;
        thread_args[i].record_cnt = i==args.thread_cnt-1?record_cnt+remainder_records:record_cnt;
        thread_args[i].min_key_len = min_key_len;
        thread_args[i].max_key_len = max_key_len;
        thread_args[i].min_val_len = min_val_len;
        thread_args[i].max_val_len = max_val_len;        
        thread_args[i].key_gen = key_gen;
        thread_args[i].val_gen = val_gen;
        ret = pthread_create(&threads[i], NULL, worker, (void*)&thread_args[i]);
        assert(ret==0);

        start_record+=record_cnt;
    }

    for(uint64_t i=0; i<args.thread_cnt; i++)
    {
        ret = pthread_join(threads[i], &status);
        assert(ret==0);
    }
    delete threads;
    //delete thread_args;
}

void micro(argument args) {
    KVS_Init(args.kvs_type, args.kvs_loc, args.kvs_path, "", args.kvs_size, args.cache_size);

    HRTime start, end;

    thread_argument *thread_args = new thread_argument[args.thread_cnt];

    // now args.record_cnt is the total number of records all processes have to cover
    if(args.key_type == "load_string") {
        load_keys(args.key_file);
        assert(keys.size() >= args.record_cnt);
    }

    // set args.record_cnt to the number of records this process is going to cover
    if (args.process_cnt>1) {
        if(args.process_id==args.process_cnt-1) {
            uint64_t record_cnt_per_process = args.record_cnt/args.process_cnt;
            uint64_t remainder = args.record_cnt - record_cnt_per_process*args.process_cnt;
            args.record_cnt=record_cnt_per_process+remainder;
        }
        else {
            args.record_cnt=args.record_cnt/args.process_cnt;
        }
    }
    
    if(args.cmd=="putget") {
        // for cache only
        // do put first
        args.cmd="put";
        do_work(args, thread_args);
        sleep(2);
        // then measure get
        args.cmd="get";
        start = get_hrtime();
        do_work(args, thread_args);
        end = get_hrtime();
    }
    else if(args.cmd=="getget") {
        // for shortcut (get)
        // assuming data is preloaded
        // warm up cache first
        args.cmd="get";
        do_work(args, thread_args);
        sleep(2);
        // then measure get
        args.cmd="get";
        start = get_hrtime();
        do_work(args, thread_args);
        end = get_hrtime();        
    }
    else if(args.cmd=="getput") {
        // for shortcut (put/update)
        // assuming data is preloaded
        // warm up cache first
        args.cmd="get";
        do_work(args, thread_args);
        sleep(2);
        // then measure put
        args.cmd="put";
        start = get_hrtime();
        do_work(args, thread_args);
        end = get_hrtime();        
    }
    else if(args.cmd=="putput") {
        // for cache-only (put/update)
        // assuming data is preloaded
        // warm up cache first
        args.cmd="put";
        do_work(args, thread_args);
        sleep(2);
        // then measure put
        args.cmd="put";
        start = get_hrtime();
        do_work(args, thread_args);
        end = get_hrtime();        
    }
    else {
        // regular get only and put only
        start = get_hrtime();
        do_work(args, thread_args);
        end = get_hrtime();        
    }

    std::cout << "# " << args.cmd << std::endl;
    std::cout << "# process: " << args.process_id << "/" << args.process_cnt << std::endl;
    std::cout << "# thread_cnt: " << args.thread_cnt << std::endl;
    std::cout << "# record_cnt: " << args.record_cnt << std::endl;

    // latency
    // us
    double sum_latency=0;
    for(int i=0; i<args.thread_cnt; i++) {
        std::cout << "# " << args.cmd << " thread " << i << " latency (us): " << thread_args[i].latency << std::endl;
        sum_latency+=thread_args[i].latency;
    }
    std::cout << "# " << args.cmd << " avg latency (us): " << sum_latency/(double)args.thread_cnt << std::endl;

    // throughput
    // K transactions per second
    double tp = (double)(args.record_cnt*1000LLU)/(double)(diff_hrtime_us(start, end));
    double tp_thread = tp/(double)args.thread_cnt;

    std::cout << "# " << args.cmd << " thread throughput (KTPS): " << tp_thread << std::endl;
    std::cout << "# " << args.cmd << " total throughput (KTPS): " << tp << std::endl;

    delete thread_args;   
    KVS_Final();
}

void start(argument args) {
    KeyValueStore::Start(args.kvs_path);
}

void reset(argument args) {
    KeyValueStore::Reset(args.kvs_path);
}

void free_memory(argument args) {
    nvmm::GlobalPtr root = str2gptr(args.kvs_loc);
    assert(root!=0);
    KeyValueStore *kvs = KeyValueStore::MakeKVS(args.kvs_type, root, args.kvs_path, "", args.kvs_size);
    kvs->Maintenance();
    sleep(10);
    delete kvs;
}

int main (int argc, char** argv) {
    if(argc<2) {
        std::cout << "usage: /micro op(get|put) kvs_path kvs_type(radixtree, hashtable) kvs_loc kvs_size cache_size key_type val_type key_size val_size thread_cnt record_cnt process_id" << std::endl;
        exit(1);
    }

    std::string command = argv[1];
    argument args;
    args.cmd = command;
    
    if (command == "start") {
        if(argc==3) {
            args.kvs_path = argv[2];
        }
        else {
            std::cout << "Invalid usage: TODO" << std::endl;
            exit(1);
        }
        start(args);
        return 0;
    }
    else if (command == "reset") {
        if(argc==3) {
            args.kvs_path = argv[2];
        }
        else {
            std::cout << "Invalid usage: TODO" << std::endl;
            exit(1);
        }
        reset(args);
        return 0;
    }
    else if (command == "free_memory") {
        if(argc==6) {
            args.kvs_path = argv[2];
            args.kvs_type = argv[3];
            args.kvs_loc = argv[4];            
            args.kvs_size = std::stoul(argv[5]);
        }
        else {
            std::cout << "Invalid usage: TODO" << std::endl;
            exit(1);
        }
        free_memory(args);
        return 0;
    }
    if(command == "get" || command == "put" || command == "putget" || command == "getget" || command == "getput" || command == "putput") {
        if(argc==16) {
            args.kvs_path = argv[2];
            args.kvs_type = argv[3];
            args.kvs_loc = argv[4];
            args.kvs_size = std::stoul(argv[5]);
            args.cache_size = std::stoul(argv[6]);
            args.key_type = argv[7];
            args.val_type = argv[8];
            args.key_size = std::stoul(argv[9]);
            args.val_size = std::stoul(argv[10]);
            args.thread_cnt = std::stoul(argv[11]);
            args.record_cnt = std::stoul(argv[12]);
            args.key_file=argv[13];
            args.process_id = std::stoul(argv[14]);
            args.process_cnt = std::stoul(argv[15]);
            if(command.substr(0,3) == "get" && args.kvs_loc == "0") {
                std::cout << "Missing kvs location/root pointer?" << std::endl;
                exit(1);
            }
        }
        else {
            std::cout << "Invalid command: " << command << std::endl;
            exit(1);
        }
        
        micro(args);
        return 0;
    }
    else if (command == "generate_keys") {
        std::string output_path;
        size_t min_key_len;
        size_t max_key_len;
        size_t count;
        if(argc==6) {
            output_path = argv[2];
            min_key_len = std::stoul(argv[3]);
            max_key_len = std::stoul(argv[4]);
            count = std::stoul(argv[5]);
        }
        else {
            std::cout << "Invalid usage: TODO" << std::endl;
            exit(1);
        }
        generate_keys(output_path, min_key_len, max_key_len, count);
        return 0;
    }
    else {
        std::cout << "Invalid usage: TODO" << std::endl;
        exit(1);
    }

}