/*
 *  (c) Copyright 2016-2021 Hewlett Packard Enterprise Development Company LP.
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


#ifndef KVS_SPLIT_ORDERED_H
#define KVS_SPLIT_ORDERED_H

#include <stddef.h>
#include <stdint.h>
#include <limits>

#include "nvmm/global_ptr.h"
#include "nvmm/shelf_id.h"
#include "nvmm/memory_manager.h"
#include "nvmm/epoch_manager.h"
#include "nvmm/heap.h"

#include "radixtree/kvs.h"
#include "kvs_metrics.h"
#include "split_ordered.h"


namespace radixtree {
// TODO: error codes!

using Emgr = nvmm::EpochManager;
using Eop = nvmm::EpochOp;

class KVSSplitOrdered : public KeyValueStore {
public:
    static size_t const kMaxKeyLen = sizeof(SplitOrderedList::ByteKey); // 40 bytes
    static size_t const kMaxValLen = std::numeric_limits<size_t>::max();

    KVSSplitOrdered(Gptr descriptor, std::string base, std::string user, size_t heap_size, nvmm::PoolId heap_id, SplitOrderedMetrics* kvs_metrics);
    ~KVSSplitOrdered();

    void Maintenance();

    int Put (char const *key, size_t const key_len,
	     char const *val, size_t const val_len);

    int Get (char const *key, size_t const key_len,
	     char *val, size_t &val_len);

    int FindOrCreate (char const *key, size_t const key_len,
         char const *val, size_t const val_len,
        char *ret_val, size_t &ret_len);

    int Del (char const *key, size_t const key_len);

    Gptr Location () {return descriptor_;}

    size_t MaxKeyLen() {return kMaxKeyLen;}
    size_t MaxValLen() {return kMaxValLen;}

    void ReportMetrics();

private:
    struct ValBuf {
        size_t size;
        char val[0];
    };

    nvmm::PoolId heap_id_;
    size_t heap_size_;

    Mmgr *mmgr_;
    Emgr *emgr_;
    Heap *heap_;

    SplitOrderedList *sol_;
    Gptr descriptor_;
    SplitOrderedMetrics *metrics_;

    int Open();
    int Close();
};

} // namespace radixtree

#endif
