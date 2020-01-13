// Copyright (c) 2018 The PIVX developers
// Copyright (c) 2018-2019 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GIANT_INVALID_H
#define GIANT_INVALID_H

#endif //GIANT_INVALID_H

#include <univalue/include/univalue.h>
#include <primitives/transaction.h>

namespace invalid_out
{
    extern std::set<COutPoint> setInvalidOutPoints;
    UniValue read_json(const std::string& jsondata);
    bool ContainsOutPoint(const COutPoint& out);
    bool LoadOutpoints();
}