/*
 *  (c) Copyright 2016-2017, 2021 Hewlett Packard Enterprise Development Company LP.
 *
 *  This software is available to you under a choice of one of two
 *  licenses. You may choose to be licensed under the terms of the
 *  GNU Lesser General Public License Version 3, or (at your option)
 *  later with exceptions included below, or under the terms of the
 *  MIT license (Expat) available in COPYING file in the source tree.
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

#ifndef BITS_TASK_H
#define BITS_TASK_H

#include <nvmm/fam.h>

namespace radixtree {
    
struct TaskId {
    TaskId(uint64_t id)
        : u64(id)
    { }

    union {
        uint64_t u64;
        int64_t i64;
        char buf[8];
    };

};

struct JobId {
    JobId(uint64_t id)
        : u64(id)
    { }

    union {
        uint64_t u64;
        int64_t i64;
        char buf[8];
    };
};

inline bool operator==(const JobId& l, const JobId& r)
{
    return l.i64 == r.i64;
}

inline bool operator!=(const JobId& l, const JobId& r)
{
    return !(l == r);
}

inline JobId operator+(JobId& l, int c)
{
    JobId job_id(l.i64 + c);
    return job_id;
}



class AbstractTask {
public:
    static const int64_t READY = 0;
    static const int64_t RUNNING = 1;
    static const int64_t FINISHED = 2;

    struct Descriptor {
        Descriptor(JobId _job_id, TaskId _task_id, AbstractTask* (*_factory)(nvmm::GlobalPtr, void*))
            : job_id(_job_id), task_id(_task_id), factory(_factory)
        { }
        int64_t status;
        JobId job_id;
        TaskId task_id;
        AbstractTask* (*factory)(nvmm::GlobalPtr, void*);
    };

public:
    virtual void call() = 0;

    int64_t GetStatus() 
    {
        int64_t status = fam_atomic_64_read(&descriptor_->status);
        return status;
    }

    void SetStatus(int64_t status) 
    {
        fam_atomic_64_write(&descriptor_->status, status);
    }

    int64_t CasStatus(int64_t oldstatus, int64_t newstatus) 
    {
        int64_t result = fam_atomic_64_compare_and_store((int64_t*)&descriptor_->status, oldstatus, newstatus);
        return result;
    }

public:
    nvmm::GlobalPtr descriptor_ptr_;
    Descriptor* descriptor_;
};

#define TASK_NUM_ARGS 0
#include "task.h.in"
#undef TASK_NUM_ARGS

#define TASK_NUM_ARGS 1
#include "task.h.in"
#undef TASK_NUM_ARGS

#define TASK_NUM_ARGS 2
#include "task.h.in"
#undef TASK_NUM_ARGS

#define TASK_NUM_ARGS 3
#include "task.h.in"
#undef TASK_NUM_ARGS

} // namespace radixtree

#endif // BITS_TASK_H
