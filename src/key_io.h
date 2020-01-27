// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GIANT_KEY_IO_H
#define GIANT_KEY_IO_H

#include "chainparams.h"
#include "key.h"
#include "keystore.h"
#include "pubkey.h"
#include "script/standard.h"

#include <string>

std::string EncodeDestination(const CTxDestination& dest);
CTxDestination DecodeDestination(const std::string& str);
bool IsValidDestinationString(const std::string& str);
bool IsValidDestinationString(const std::string& str, const CChainParams& params);
bool IsValidContractSenderAddressString(const std::string& str);

#endif // GIANT_KEY_IO_H
