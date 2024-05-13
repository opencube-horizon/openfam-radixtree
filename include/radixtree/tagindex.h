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

#ifndef TAGINDEX_H
#define TAGINDEX_H

#include <nvmm/fam.h>

namespace radixtree {

struct TagIndex {
    typedef int32_t Index;

    TagIndex()
    {
        tag_ = 0;
        index_ = 0;
    }

    TagIndex(Index index)
    {
        tag_ = 0;
        index_ = index;
    }

    Index index()
    {
        return index_;
    }

    void incr(int32_t i)
    {
        index_+=i;
    }

    TagIndex operator+(const int32_t incr)
    {
        TagIndex res = *this;
        res.index_ = res.index_ + incr;
        return res;
    }

    TagIndex operator-(const TagIndex& other)
    {
        TagIndex res = *this;
        res.index_ = res.index_ - other.index_;
        return res;
    }

    operator int32_t() const
    {
        return index_;
    }

    union {
        struct {
            int32_t tag_;
            Index index_;
        };
        int32_t i32[2];
        int64_t i64;
    };
};

static inline
TagIndex atomic_load(TagIndex* addr) 
{
    TagIndex result;
    result.i64 = fam_atomic_64_read((int64_t*)addr);
    return result;
}

static inline
void atomic_store(TagIndex* addr, TagIndex val) 
{
    fam_atomic_64_write((int64_t*)addr, val.i64);
}

static inline
TagIndex atomic_compare_and_swap(void* target, TagIndex old_value, TagIndex new_value) {
    TagIndex result;
    result.i64 = fam_atomic_64_compare_and_store((int64_t*)target, old_value.i64, new_value.i64);
    return result;
}

static inline
int32_t atomic_fetch_and_add(TagIndex* addr, int32_t incr) 
{
    TagIndex* taddr = (TagIndex*) addr;
    void* iaddr = &taddr->index_;
    return fam_atomic_32_fetch_and_add((int32_t*) iaddr, incr);
}

} // end radixtree


#endif // TAGINDEX_H
