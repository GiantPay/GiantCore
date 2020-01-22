// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GIANT_VM_H
#define GIANT_VM_H

#include<stdint.h>

static const uint32_t MIN_BLOCK_SIZE_DGP = 500000;
static const uint32_t MAX_BLOCK_SIZE_DGP = 32000000;
static const uint32_t DEFAULT_BLOCK_SIZE_DGP = 2000000;

static const uint64_t MIN_MIN_GAS_PRICE_DGP = 1;
static const uint64_t MAX_MIN_GAS_PRICE_DGP = 10000;
static const uint64_t DEFAULT_MIN_GAS_PRICE_DGP = 40;

static const uint64_t MIN_BLOCK_GAS_LIMIT_DGP = 1000000;
static const uint64_t MAX_BLOCK_GAS_LIMIT_DGP = 1000000000;
static const uint64_t DEFAULT_BLOCK_GAS_LIMIT_DGP = 40000000;

struct VersionVM {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t format : 2;
    uint8_t rootVM : 6;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t rootVM : 6;
    uint8_t format : 2;
#endif
    uint8_t vmVersion;
    uint16_t flagOptions;
    // CONSENSUS CRITICAL!
    // Do not add any other fields to this struct

    uint32_t toRaw() {
        return *(uint32_t*)this;
    }

    static VersionVM fromRaw(uint32_t val) {
        VersionVM x = *(VersionVM*) & val;
        return x;
    }

    static VersionVM GetNoExec() {
        VersionVM x;
        x.flagOptions = 0;
        x.rootVM = 0;
        x.format = 0;
        x.vmVersion = 0;
        return x;
    }

    static VersionVM GetEVMDefault() {
        VersionVM x;
        x.flagOptions = 0;
        x.rootVM = 1;
        x.format = 0;
        x.vmVersion = 0;
        return x;
    }
} __attribute__ ((__packed__));

#endif // GIANT_VM_H
