// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key_io.h"
#include "base58.h"
#include "script/script.h"

#include <algorithm>
#include <assert.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <string.h>

namespace {

    CTxDestination DecodeDestination(const std::string& str, const CChainParams& params) {
        std::vector<unsigned char> data;
        uint160 hash;
        if (DecodeBase58Check(str, data)) {
            // base58-encoded Bitcoin addresses.
            // Public-key-hash-addresses have version 0 (or 111 testnet).
            // The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
            const std::vector<unsigned char>& pubkey_prefix = params.Base58Prefix(CChainParams::PUBKEY_ADDRESS);
            if (data.size() == hash.size() + pubkey_prefix.size() && std::equal(pubkey_prefix.begin(), pubkey_prefix.end(), data.begin())) {
                std::copy(data.begin() + pubkey_prefix.size(), data.end(), hash.begin());
                return CKeyID(hash);
            }
            // Script-hash-addresses have version 5 (or 196 testnet).
            // The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
            const std::vector<unsigned char>& script_prefix = params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
            if (data.size() == hash.size() + script_prefix.size() && std::equal(script_prefix.begin(), script_prefix.end(), data.begin())) {
                std::copy(data.begin() + script_prefix.size(), data.end(), hash.begin());
                return CScriptID(hash);
            }
        }
        data.clear();
        return CNoDestination();
    }
} // namespace

CTxDestination DecodeDestination(const std::string& str) {
    return DecodeDestination(str, Params());
}

bool IsValidDestinationString(const std::string& str, const CChainParams& params)
{
    return IsValidDestination(DecodeDestination(str, params));
}

bool IsValidDestinationString(const std::string& str)
{
    return IsValidDestinationString(str, Params());
}

bool IsValidContractSenderAddressString(const std::string& str)
{
    return IsValidContractSenderAddress(DecodeDestination(str));
}