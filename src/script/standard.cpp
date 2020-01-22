// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017 The PIVX developers
// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/standard.h"

#include "pubkey.h"
#include "script/script.h"
#include "util.h"
#include "utilstrencodings.h"
#include "vm/vm.h"
#include "streams.h"
#include <boost/variant.hpp>

using namespace std;

typedef vector<unsigned char> valtype;

unsigned nMaxDatacarrierBytes = MAX_OP_RETURN_RELAY;

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {
}

const char* GetTxnOutputType(txnouttype t) {
    switch (t) {
        case TX_NONSTANDARD: return "nonstandard";
        case TX_PUBKEY: return "pubkey";
        case TX_PUBKEYHASH: return "pubkeyhash";
        case TX_SCRIPTHASH: return "scripthash";
        case TX_MULTISIG: return "multisig";
        case TX_NULL_DATA: return "nulldata";
    }
    return NULL;
}

/**
 * Return public keys or hashes from scriptPubKey, for 'standard' transaction types.
 */
txnouttype Solver(const CScript& scriptPubKey, std::vector<std::vector<unsigned char>>& vSolutionsRet, bool contractConsensus) {
    //contractConsesus is true when evaluating if a contract tx is "standard" for consensus purposes
    //It is false in all other cases, so to prevent a particular contract tx from being broadcast on mempool, but allowed in blocks,
    //one should ensure that contractConsensus is false
    
    // Templates
    static multimap<txnouttype, CScript> mTemplates;
    if (mTemplates.empty()) {
        // Standard tx, sender provides pubkey, receiver adds signature
        mTemplates.insert(make_pair(TX_PUBKEY, CScript() << OP_PUBKEY << OP_CHECKSIG));

        // Bitcoin address tx, sender provides hash of pubkey, receiver provides signature and pubkey
        mTemplates.insert(make_pair(TX_PUBKEYHASH, CScript() << OP_DUP << OP_HASH160 << OP_PUBKEYHASH << OP_EQUALVERIFY << OP_CHECKSIG));

        // Sender provides N pubkeys, receivers provides M signatures
        mTemplates.insert(make_pair(TX_MULTISIG, CScript() << OP_SMALLINTEGER << OP_PUBKEYS << OP_SMALLINTEGER << OP_CHECKMULTISIG));

        // Contract creation tx with sender
        mTemplates.insert(make_pair(TX_CREATE_SENDER, CScript() << OP_ADDRESS_TYPE << OP_ADDRESS << OP_SCRIPT_SIG << OP_SENDER << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE << OP_DATA << OP_CREATE));

        // Call contract tx with sender
        mTemplates.insert(make_pair(TX_CALL_SENDER, CScript() << OP_ADDRESS_TYPE << OP_ADDRESS << OP_SCRIPT_SIG << OP_SENDER << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE << OP_DATA << OP_PUBKEYHASH << OP_CALL));

        // Contract creation tx
        mTemplates.insert(make_pair(TX_CREATE, CScript() << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE << OP_DATA << OP_CREATE));

        // Call contract tx
        mTemplates.insert(make_pair(TX_CALL, CScript() << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE << OP_DATA << OP_PUBKEYHASH << OP_CALL));
    }
    vSolutionsRet.clear();

    // Shortcut for pay-to-script-hash, which are more constrained than the other types:
    // it is always OP_HASH160 20 [20 byte hash] OP_EQUAL
    if (scriptPubKey.IsPayToScriptHash()) {
        vector<unsigned char> hashBytes(scriptPubKey.begin() + 2, scriptPubKey.begin() + 22);
        vSolutionsRet.push_back(hashBytes);
        return TX_SCRIPTHASH;
    }

    // Provably prunable, data-carrying output
    //
    // So long as script passes the IsUnspendable() test and all but the first
    // byte passes the IsPushOnly() test we don't care what exactly is in the
    // script.
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin() + 1)) {
        return TX_NULL_DATA;
    }

    // Scan templates
    const CScript& script1 = scriptPubKey;
    for (const std::pair<txnouttype, CScript>& tplate : mTemplates) {
        const CScript& script2 = tplate.second;
        vSolutionsRet.clear();

        opcodetype opcode1, opcode2;
        vector<unsigned char> vch1, vch2;
        
        // TODO
        uint64_t addressType = 0;
        VersionVM version;
        version.rootVM = 20; //set to some invalid value

        // Compare
        CScript::const_iterator pc1 = script1.begin();
        CScript::const_iterator pc2 = script2.begin();
        while (true) {
            if (pc1 == script1.end() && pc2 == script2.end()) {
                // Found a match
                if (tplate.first == TX_MULTISIG) {
                    // Additional checks for TX_MULTISIG:
                    unsigned char m = vSolutionsRet.front()[0];
                    unsigned char n = vSolutionsRet.back()[0];
                    if (m < 1 || n < 1 || m > n || vSolutionsRet.size() - 2 != n)
                        return TX_NONSTANDARD;
                }
                return tplate.first;
            }
            if (!script1.GetOp(pc1, opcode1, vch1))
                break;
            if (!script2.GetOp(pc2, opcode2, vch2))
                break;

            // Template matching opcodes:
            if (opcode2 == OP_PUBKEYS) {
                while (vch1.size() >= 33 && vch1.size() <= 65) {
                    vSolutionsRet.push_back(vch1);
                    if (!script1.GetOp(pc1, opcode1, vch1))
                        break;
                }
                if (!script2.GetOp(pc2, opcode2, vch2))
                    break;
                // Normal situation is to fall through
                // to other if/else statements
            }

            if (opcode2 == OP_PUBKEY) {
                if (vch1.size() < 33 || vch1.size() > 65)
                    break;
                vSolutionsRet.push_back(vch1);
            } else if (opcode2 == OP_PUBKEYHASH) {
                if (vch1.size() != sizeof (uint160))
                    break;
                vSolutionsRet.push_back(vch1);
            } else if (opcode2 == OP_SMALLINTEGER) { // Single-byte small integer pushed onto vSolutions
                if (opcode1 == OP_0 ||
                        (opcode1 >= OP_1 && opcode1 <= OP_16)) {
                    char n = (char) CScript::DecodeOP_N(opcode1);
                    vSolutionsRet.push_back(valtype(1, n));
                } else
                    break;
            } 
            else if (opcode2 == OP_VERSION)
            {
                if(0 <= opcode1 && opcode1 <= OP_PUSHDATA4)
                {
                    if(vch1.empty() || vch1.size() > 4 || (vch1.back() & 0x80))
                        return TX_NONSTANDARD;

                    version = VersionVM::fromRaw(CScriptNum::vch_to_uint64(vch1));
                    if(!(version.toRaw() == VersionVM::GetEVMDefault().toRaw() || version.toRaw() == VersionVM::GetNoExec().toRaw())){
                        // only allow standard EVM and no-exec transactions to live in mempool
                        return TX_NONSTANDARD;
                    }
                }
            }
            else if(opcode2 == OP_GAS_LIMIT) {
                try {
                    uint64_t val = CScriptNum::vch_to_uint64(vch1);
                    if(contractConsensus) {
                        //consensus rules (this is checked more in depth later using DGP)
                        if (version.rootVM != 0 && val < 1) {
                            return TX_NONSTANDARD;
                        }
                        if (val > MAX_BLOCK_GAS_LIMIT_DGP) {
                            //do not allow transactions that could use more gas than is in a block
                            return TX_NONSTANDARD;
                        }
                    }else{
                        //standard mempool rules for contracts
                        //consensus rules for contracts
                        if (version.rootVM != 0 && val < STANDARD_MINIMUM_GAS_LIMIT) {
                            return TX_NONSTANDARD;
                        }
                        if (val > DEFAULT_BLOCK_GAS_LIMIT_DGP / 2) {
                            //don't allow transactions that use more than 1/2 block of gas to be broadcast on the mempool
                            return TX_NONSTANDARD;
                        }

                    }
                }
                catch (const scriptnum_error &err) {
                    return TX_NONSTANDARD;
                }
            }
            else if(opcode2 == OP_GAS_PRICE) {
                try {
                    uint64_t val = CScriptNum::vch_to_uint64(vch1);
                    if(contractConsensus) {
                        //consensus rules (this is checked more in depth later using DGP)
                        if (version.rootVM != 0 && val < 1) {
                            return TX_NONSTANDARD;
                        }
                    }else{
                        //standard mempool rules
                        if (version.rootVM != 0 && val < STANDARD_MINIMUM_GAS_PRICE) {
                            return TX_NONSTANDARD;
                        }
                    }
                }
                catch (const scriptnum_error &err) {
                    return TX_NONSTANDARD;
                }
            }
            else if(opcode2 == OP_DATA)
            {
                if(0 <= opcode1 && opcode1 <= OP_PUSHDATA4)
                {
                    if(vch1.empty())
                        break;
                }
            }
            else if(opcode2 == OP_ADDRESS_TYPE)
            {
                try {
                    uint64_t val = CScriptNum::vch_to_uint64(vch1);
                    if(val < addresstype::PUBKEYHASH || val > addresstype::NONSTANDARD)
                        break;

                    addressType = val;
                }
                catch (const scriptnum_error &err) {
                    break;
                }
            }
            else if(opcode2 == OP_ADDRESS)
            {
                // Get the destination
                CTxDestination dest;
                if(addressType == addresstype::PUBKEYHASH && vch1.size() == sizeof(CKeyID))
                {
                    dest = CKeyID(uint160(vch1));
                }
                else
                    break;

                // Get the public key for the destination
                CScript senderPubKey = GetScriptForDestination(dest);
                CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                ss << senderPubKey;
                vSolutionsRet.push_back(std::vector<unsigned char>(ss.begin(), ss.end()));
            }
            else if(opcode2 == OP_SCRIPT_SIG)
            {
                if(0 <= opcode1 && opcode1 <= OP_PUSHDATA4)
                {
                    if(vch1.empty())
                        break;

                    // Check the max size of the signature script
                    if(vch1.size() > MAX_BASE_SCRIPT_SIZE)
                        return TX_NONSTANDARD;

                    vSolutionsRet.push_back(vch1);
                }
            }
            else if (opcode1 != opcode2 || vch1 != vch2) {
                // Others must match exactly
                break;
            }
        }
    }

    vSolutionsRet.clear();
    return TX_NONSTANDARD;
}

int ScriptSigArgsExpected(txnouttype t, const std::vector<std::vector<unsigned char> >& vSolutions) {
    switch (t) {
        case TX_NONSTANDARD:
        case TX_NULL_DATA:
            return -1;
        case TX_PUBKEY:
            return 1;
        case TX_PUBKEYHASH:
            return 2;
        case TX_MULTISIG:
            if (vSolutions.size() < 1 || vSolutions[0].size() < 1)
                return -1;
            return vSolutions[0][0] + 1;
        case TX_SCRIPTHASH:
            return 1; // doesn't include args needed by the script
    }
    return -1;
}

bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType) {
    vector<valtype> vSolutions;
    whichType = Solver(scriptPubKey, vSolutions);
    if (whichType == TX_NONSTANDARD)
        return false;

    if (whichType == TX_MULTISIG) {
        unsigned char m = vSolutions.front()[0];
        unsigned char n = vSolutions.back()[0];
        // Support up to x-of-3 multisig txns as standard
        if (n < 1 || n > 3)
            return false;
        if (m < 1 || m > n)
            return false;
    } else if (whichType == TX_NULL_DATA &&
            (!GetBoolArg("-datacarrier", true) || scriptPubKey.size() > nMaxDatacarrierBytes))
        return false;

    return whichType != TX_NONSTANDARD;
}

bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet) {
    vector<valtype> vSolutions;
    txnouttype whichType = Solver(scriptPubKey, vSolutions);
    if (whichType == TX_NONSTANDARD)
        return false;

    if (whichType == TX_PUBKEY) {
        CPubKey pubKey(vSolutions[0]);
        if (!pubKey.IsValid())
            return false;

        addressRet = pubKey.GetID();
        return true;
    } else if (whichType == TX_PUBKEYHASH) {
        addressRet = CKeyID(uint160(vSolutions[0]));
        return true;
    } else if (whichType == TX_SCRIPTHASH) {
        addressRet = CScriptID(uint160(vSolutions[0]));
        return true;
    }
    // Multisig txns have more than one address...
    return false;
}

bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, vector<CTxDestination>& addressRet, int& nRequiredRet) {
    addressRet.clear();
    vector<valtype> vSolutions;
    typeRet = Solver(scriptPubKey, vSolutions);
    if (typeRet == TX_NONSTANDARD) {
        return false;
    } else if (typeRet == TX_NULL_DATA) {
        // This is data, not addresses
        return false;
    }

    if (typeRet == TX_MULTISIG) {
        nRequiredRet = vSolutions.front()[0];
        for (unsigned int i = 1; i < vSolutions.size() - 1; i++) {
            CPubKey pubKey(vSolutions[i]);
            if (!pubKey.IsValid())
                continue;

            CTxDestination address = pubKey.GetID();
            addressRet.push_back(address);
        }

        if (addressRet.empty())
            return false;
    } else {
        nRequiredRet = 1;
        CTxDestination address;
        if (!ExtractDestination(scriptPubKey, address))
            return false;
        addressRet.push_back(address);
    }

    return true;
}

namespace {

    class CScriptVisitor : public boost::static_visitor<bool> {
    private:
        CScript *script;
    public:

        CScriptVisitor(CScript *scriptin) {
            script = scriptin;
        }

        bool operator()(const CNoDestination &dest) const {
            script->clear();
            return false;
        }

        bool operator()(const CKeyID &keyID) const {
            script->clear();
            *script << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
            return true;
        }

        bool operator()(const CScriptID &scriptID) const {
            script->clear();
            *script << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
            return true;
        }
    };
}

CScript GetScriptForDestination(const CTxDestination& dest) {
    CScript script;

    boost::apply_visitor(CScriptVisitor(&script), dest);
    return script;
}

CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys) {
    CScript script;

    script << CScript::EncodeOP_N(nRequired);
    for (const CPubKey& key : keys)
        script << ToByteVector(key);
    script << CScript::EncodeOP_N(keys.size()) << OP_CHECKMULTISIG;
    return script;
}

bool IsValidDestination(const CTxDestination& dest) {
    return dest.which() != 0;
}

bool IsValidContractSenderAddress(const CTxDestination &dest) {
    const CKeyID *keyID = boost::get<CKeyID>(&dest);
    return keyID != 0;
}

bool ExtractSenderData(const CScript &outputPubKey, CScript *senderPubKey, CScript *senderSig) {
    if (outputPubKey.HasOpSender()) {
        try {
            // Solve the contract with or without contract consensus
            std::vector<valtype> vSolutions;
            if (TX_NONSTANDARD == Solver(outputPubKey, vSolutions, true) &&
                    TX_NONSTANDARD == Solver(outputPubKey, vSolutions, false))
                return false;

            // Check the size of the returned data
            if (vSolutions.size() < 2)
                return false;

            // Get the sender public key
            if (senderPubKey) {
                CDataStream ss(vSolutions[0], SER_NETWORK, PROTOCOL_VERSION);
                ss >> *senderPubKey;
            }

            // Get the sender signature
            if (senderSig) {
                CDataStream ss(vSolutions[1], SER_NETWORK, PROTOCOL_VERSION);
                ss >> *senderSig;
            }
        }        catch (...) {
            return false;
        }

        return true;
    }
    return false;
}

