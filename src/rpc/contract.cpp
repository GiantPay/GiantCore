// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "amount.h"
#include "base58.h"
#include "coincontrol.h"
#include "core_io.h"
#include "init.h"
#include "key_io.h"
#include "keystore.h"
#include "main.h"
#include "primitives/transaction.h"
#include "pubkey.h"
#include "rpc/server.h"
#include "script/script.h"
#include "script/sign.h"
#include "script/interpreter.h"
#include "script/script_error.h"
#include "script/standard.h"
#include "uint256.h"
#include "utilmoneystr.h"
#include "vm/vm.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"

#include <stdint.h>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <univalue.h>
#include <functional>

using namespace boost;
using namespace boost::assign;
using namespace std;

static const string PRC_CONTRACT_PARAMS_CODE = "code";
static const string PRC_CONTRACT_PARAMS_GAS_LIMIT = "gasLimit";
static const string PRC_CONTRACT_PARAMS_GAS_PRICE = "gasPrice";
static const string PRC_CONTRACT_PARAMS_SENDER = "sender";
static const string RPC_CONTRACT_PARAMS_BROADCAST = "broadcast";
static const string RPC_CONTRACT_PARAMS_CHANGE_TO_SENDER = "changeToSender";

static const uint64_t DEFAULT_GAS_LIMIT_OP_CREATE = 2500000;
static const uint64_t DEFAULT_GAS_LIMIT_OP_SEND = 250000;
static const uint64_t MINIMUM_GAS_LIMIT = 10000;

static const CAmount DEFAULT_GAS_PRICE = 0.00000040 * COIN;
static const CAmount MAX_RPC_GAS_PRICE = 0.00000100 * COIN;

static const bool DEFAULT_CONTRACT_TX_BROADCAST = true;
static const bool DEFAULT_CONTRACT_CHANGE_TO_SENDER = true;

string GetCode(const UniValue& contractParams) {
    string strCode = contractParams[PRC_CONTRACT_PARAMS_CODE].get_str();
    if (strCode.size() == 0 || !IsHex(strCode)) // TODO validate of javascript code
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid code of contract");
    return strCode;
}

uint64_t GetGasLimit(const UniValue& contractParams) {
    uint64_t blockGasLimit = 0; // TODO
    uint64_t nGasLimit = DEFAULT_GAS_LIMIT_OP_CREATE;

    if (contractParams.exists(PRC_CONTRACT_PARAMS_GAS_LIMIT)) {
        nGasLimit = contractParams[PRC_CONTRACT_PARAMS_GAS_LIMIT].get_int64();
        if (nGasLimit > blockGasLimit)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Maximum is: " + i64tostr(blockGasLimit) + ")");
        if (nGasLimit < MINIMUM_GAS_LIMIT)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit (Minimum is: " + i64tostr(MINIMUM_GAS_LIMIT) + ")");
        if (nGasLimit <= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasLimit");
    }
    return nGasLimit;
}

uint64_t GetGasPrice(const UniValue& contractParams) {
    uint64_t minGasPrice = 0; // TODO
    CAmount nGasPrice = minGasPrice > DEFAULT_GAS_PRICE ? minGasPrice : DEFAULT_GAS_PRICE;

    if (contractParams.exists(PRC_CONTRACT_PARAMS_GAS_PRICE)) {
        nGasPrice = AmountFromValue(contractParams[PRC_CONTRACT_PARAMS_GAS_PRICE]);
        if (nGasPrice <= 0)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice");
        CAmount maxRpcGasPrice = GetArg("-rpcmaxgasprice", MAX_RPC_GAS_PRICE);
        if (nGasPrice > (int64_t) maxRpcGasPrice)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice, Maximum allowed in RPC calls is: " + FormatMoney(maxRpcGasPrice) + " (use -rpcmaxgasprice to change it)");
        if (nGasPrice < (int64_t) minGasPrice)
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid value for gasPrice (Minimum is: " + FormatMoney(minGasPrice) + ")");
    }
    return nGasPrice;
}

CTxDestination GetSender(const UniValue& contractParams, bool& fHasSender) {
    CTxDestination sender;
    if (contractParams.exists(PRC_CONTRACT_PARAMS_SENDER)) {
        sender = DecodeDestination(contractParams[PRC_CONTRACT_PARAMS_SENDER].get_str());
        if (!IsValidDestination(sender))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Giant address to send from");
        if (!IsValidContractSenderAddress(sender))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid contract sender address. Only P2PK and P2PKH allowed");
        else
            fHasSender = true;
    }
    return sender;
}

bool GetBroadcast(const UniValue& contractParams) {
    bool fBroadcast = DEFAULT_CONTRACT_TX_BROADCAST;
    if (contractParams.exists(RPC_CONTRACT_PARAMS_BROADCAST)) {
        fBroadcast = contractParams[RPC_CONTRACT_PARAMS_BROADCAST].get_bool();
    }
    return fBroadcast;
}

bool GetChangeToSender(const UniValue& contractParams) {
    bool fChangeToSender = DEFAULT_CONTRACT_CHANGE_TO_SENDER;
    if (contractParams.exists(RPC_CONTRACT_PARAMS_CHANGE_TO_SENDER)) {
        fChangeToSender = contractParams[RPC_CONTRACT_PARAMS_CHANGE_TO_SENDER].get_bool();
    }
    return fChangeToSender;
}

bool SetDefaultSignSenderAddress(CTxDestination& destAdress) {
    // Set default sender address if none provided
    // Select any valid unspent output that can be used for contract sender address
    std::vector<COutput> vecOutputs;
    pwalletMain->AvailableCoins(vecOutputs, false);

    for (const COutput& out : vecOutputs) {
        const CScript& scriptPubKey = out.tx->vout[out.i].scriptPubKey;
        bool fValidAddress = ExtractDestination(scriptPubKey, destAdress)
                && IsValidContractSenderAddress(destAdress);

        if (!fValidAddress)
            continue;
        break;
    }
    return !boost::get<CNoDestination>(&destAdress);
}

bool SetDefaultPayForContractAddress(CCoinControl & coinControl) {
    // Set default coin to pay for the contract
    // Select any valid unspent output that can be used to pay for the contract
    std::vector<COutput> vecOutputs;
    coinControl.fAllowOtherInputs = true;
    pwalletMain->AvailableCoins(vecOutputs, false);

    for (const COutput& out : vecOutputs) {
        CTxDestination destAdress;
        const CScript& scriptPubKey = out.tx->vout[out.i].scriptPubKey;
        bool fValidAddress = ExtractDestination(scriptPubKey, destAdress)
                && IsValidContractSenderAddress(destAdress);

        if (!fValidAddress)
            continue;

        coinControl.Select(COutPoint(out.tx->GetHash(), out.i));
        break;
    }

    return coinControl.HasSelected();
}

bool GetSenderDest(const CWalletTx& tx, CTxDestination& txSenderDest) {
    // Initialize variables
    CScript senderPubKey;

    // Get sender destination
    if (tx.HasOpSender()) {
        // Get destination from the outputs
        for (CTxOut out : tx.vout) {
            if (out.scriptPubKey.HasOpSender()) {
                ExtractSenderData(out.scriptPubKey, &senderPubKey, 0);
                break;
            }
        }
    } else {
        // Get destination from the inputs
        senderPubKey = pwalletMain->mapWallet.at(tx.vin[0].prevout.hash).vout[tx.vin[0].prevout.n].scriptPubKey;
    }

    // Extract destination from script
    return ExtractDestination(senderPubKey, txSenderDest);
}

UniValue createcontract(const UniValue& params, bool fHelp) {
    if (fHelp || params.size() != 1) // TODO
        throw runtime_error("todo createcontract help message");

    assert(pwalletMain != NULL);
#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif
    RPCTypeCheck(params, boost::assign::list_of(UniValue::VOBJ), false);

    UniValue contractParams = params[0].get_obj();

    auto strCode = GetCode(contractParams);
    auto nGasLimit = GetGasLimit(contractParams);
    auto nGasPrice = GetGasPrice(contractParams);
    bool fHasSender = false;
    auto sender = GetSender(contractParams, fHasSender);
    auto fBroadcast = GetBroadcast(contractParams);
    auto fChangeToSender = GetChangeToSender(contractParams);

    CCoinControl coinControl = CCoinControl();
    CTxDestination signer = CNoDestination();

    if (fHasSender) {// Find a UTXO with sender address
        std::vector<COutput> vecOutputs;
        coinControl.fAllowOtherInputs = true;

        pwalletMain->AvailableCoins(vecOutputs, false);
        for (const COutput& out : vecOutputs) {
            CTxDestination destAdress;
            const CScript& scriptPubKey = out.tx->vout[out.i].scriptPubKey;
            bool fValidAddress = ExtractDestination(scriptPubKey, destAdress);

            if (!fValidAddress || sender != destAdress)
                continue;

            coinControl.Select(COutPoint(out.tx->GetHash(), out.i));

            break;
        }

        if (coinControl.HasSelected()) {
            // Change to the sender
            if (fChangeToSender) {
                coinControl.destChange = sender;
            }
        }
        signer = sender;
    } else {
        // If no sender address provided set to the default sender address
        SetDefaultSignSenderAddress(signer);
    }

    EnsureWalletIsUnlocked();
    CAmount nGasFee = nGasPrice * nGasLimit;
    CAmount nBalance = pwalletMain->GetBalance();

    // Check amount
    if (nGasFee <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");

    if (nGasFee > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    // Select default coin that will pay for the contract if none selected
    if (!coinControl.HasSelected() && !SetDefaultPayForContractAddress(coinControl))
        throw JSONRPCError(RPC_TYPE_ERROR, "Does not have any P2PK or P2PKH unspent outputs to pay for the contract.");

    // Build OP_EXEC script
    CScript scriptPubKey = CScript() << CScriptNum(VersionVM::GetEVMDefault().toRaw()) << CScriptNum(nGasLimit) << CScriptNum(nGasPrice) << ParseHex(strCode) << OP_CREATE;
    if (IsValidDestination(signer)) {
        CKeyID key_id = GetKeyForDestination(*pwalletMain, signer);
        CKey key;
        if (!pwalletMain->GetKey(key_id, key)) {
            throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
        }
        std::vector<unsigned char> scriptSig;
        scriptPubKey = (CScript() << CScriptNum(addresstype::PUBKEYHASH) << ToByteVector(key_id) << ToByteVector(scriptSig) << OP_SENDER) + scriptPubKey;
    } else {
        // OP_SENDER will always be used when QIP5Height is active
        throw JSONRPCError(RPC_TYPE_ERROR, "Sender address fail to set for OP_SENDER.");
    }

    // Create and send the transaction
    CReserveKey reservekey(pwalletMain);
    CAmount nFeeRequired;
    std::string strError;
    vector<pair<CScript, CAmount> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, 0));
    CWalletTx wtx;

    if (!pwalletMain->CreateTransaction2(vecSend, wtx, reservekey, nFeeRequired, strError, &coinControl, ALL_COINS, true, nGasFee, true, &signer)) {
        if (nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    CTxDestination txSenderDest;
    GetSenderDest(wtx, txSenderDest);

    if (fHasSender && !(sender == txSenderDest)) {
        throw JSONRPCError(RPC_TYPE_ERROR, "Sender could not be set, transaction was not committed!");
    }

    UniValue result(UniValue::VOBJ);
    if (fBroadcast) {
        if (!pwalletMain->CommitTransaction(wtx, reservekey)) {
            throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of the wallet and coins were spent in the copy but not marked as spent here.");
        }
        std::string txId = wtx.GetHash().GetHex();
        result.pushKV("txid", txId);

        CTxDestination txSenderAdress(txSenderDest);
        CKeyID keyid = GetKeyForDestination(*pwalletMain, txSenderAdress);

        result.pushKV("sender", EncodeDestination(txSenderAdress));
        result.pushKV("hash160", HexStr(valtype(keyid.begin(), keyid.end())));

        std::vector<unsigned char> SHA256TxVout(32);
        std::vector<unsigned char> contractAddress(20);
        std::vector<unsigned char> txIdAndVout(wtx.GetHash().begin(), wtx.GetHash().end());
        uint32_t voutNumber = 0;
        for (const CTxOut& txout : wtx.vout) {
            if (txout.scriptPubKey.HasOpCreate()) {
                std::vector<unsigned char> voutNumberChrs;
                if (voutNumberChrs.size() < sizeof (voutNumber)) {
                    voutNumberChrs.resize(sizeof (voutNumber));
                }
                std::memcpy(voutNumberChrs.data(), &voutNumber, sizeof (voutNumber));
                txIdAndVout.insert(txIdAndVout.end(), voutNumberChrs.begin(), voutNumberChrs.end());
                break;
            }
            voutNumber++;
        }
        CSHA256().Write(txIdAndVout.data(), txIdAndVout.size()).Finalize(SHA256TxVout.data());
        CRIPEMD160().Write(SHA256TxVout.data(), SHA256TxVout.size()).Finalize(contractAddress.data());
        result.pushKV("address", HexStr(contractAddress));
    } else {
        std::string strHex = EncodeHexTx(wtx);
        result.pushKV("raw transaction", strHex);
    }
    return result;
}

UniValue sendtocontract(const UniValue& params, bool fHelp) {
    if (fHelp || params.size() != 1)
        throw runtime_error("todo sendtocontract help message");

    return NullUniValue;
}

UniValue callcontract(const UniValue& params, bool fHelp) {
    if (fHelp || params.size() != 1)
        throw runtime_error("todo callcontract help message");

    return NullUniValue;
}



