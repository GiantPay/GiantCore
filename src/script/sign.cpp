// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2017 The PIVX developers
// Copyright (c) 2018-2020 The GIANT developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/sign.h"

#include "primitives/transaction.h"
#include "key.h"
#include "keystore.h"
#include "script/standard.h"
#include "uint256.h"
#include "streams.h"
#include "util.h"


using namespace std;

typedef vector<unsigned char> valtype;

bool Sign1(const CKeyID& address, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet)
{
    CKey key;
    if (!keystore.GetKey(address, key))
        return false;

    vector<unsigned char> vchSig;
    if (!key.Sign(hash, vchSig))
        return false;
    vchSig.push_back((unsigned char)nHashType);
    scriptSigRet << vchSig;

    return true;
}

bool SignN(const vector<valtype>& multisigdata, const CKeyStore& keystore, uint256 hash, int nHashType, CScript& scriptSigRet)
{
    int nSigned = 0;
    int nRequired = multisigdata.front()[0];
    for (unsigned int i = 1; i < multisigdata.size()-1 && nSigned < nRequired; i++)
    {
        const valtype& pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
            ++nSigned;
    }
    return nSigned==nRequired;
}

/**
 * Sign scriptPubKey with private keys stored in keystore, given transaction hash and hash type.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey can't be signed),
 * unless whichTypeRet is TX_SCRIPTHASH, in which case scriptSigRet is the redemption script.
 * Returns false if scriptPubKey could not be completely satisfied.
 */
bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType,
                  CScript& scriptSigRet, txnouttype& whichTypeRet)
{
    scriptSigRet.clear();

    vector<valtype> vSolutions;
    whichTypeRet = Solver(scriptPubKey, vSolutions);
    if (whichTypeRet == TX_NONSTANDARD)
    {
        LogPrintf("*** solver solver failed \n");
        return false;
    }

    CKeyID keyID;
    switch (whichTypeRet)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
    {
        LogPrintf("*** null data \n");
        return false;
    }
    case TX_PUBKEY:
        keyID = CPubKey(vSolutions[0]).GetID();
        if(!Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
        {
            LogPrintf("*** Sign1 failed \n");
            return false;
        }
        return true;
    case TX_PUBKEYHASH:
        keyID = CKeyID(uint160(vSolutions[0]));
        if (!Sign1(keyID, keystore, hash, nHashType, scriptSigRet))
        {
            LogPrintf("*** solver failed to sign \n");
            return false;
        }
        else
        {
            CPubKey vch;
            keystore.GetPubKey(keyID, vch);
            scriptSigRet << ToByteVector(vch);
        }
        return true;
    case TX_SCRIPTHASH:
        return keystore.GetCScript(uint160(vSolutions[0]), scriptSigRet);

    case TX_MULTISIG:
        scriptSigRet << OP_0; // workaround CHECKMULTISIG bug
        return (SignN(vSolutions, keystore, hash, nHashType, scriptSigRet));
    }
    LogPrintf("*** solver no case met \n");
    return false;
}

bool SignSignature(const CKeyStore &keystore, const CScript& fromPubKey, CMutableTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];

    // Leave out the signature from the hash, since a signature can't sign itself.
    // The checksig op will also drop the signatures from its hash.
    uint256 hash = SignatureHash(fromPubKey, txTo, nIn, nHashType);

    txnouttype whichType;
    if (!Solver(keystore, fromPubKey, hash, nHashType, txin.scriptSig, whichType))
        return false;

    if (whichType == TX_SCRIPTHASH)
    {
        // Solver returns the subscript that need to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        CScript subscript = txin.scriptSig;

        // Recompute txn hash using subscript in place of scriptPubKey:
        uint256 hash2 = SignatureHash(subscript, txTo, nIn, nHashType);

        txnouttype subType;
        bool fSolved =
            Solver(keystore, subscript, hash2, nHashType, txin.scriptSig, subType) && subType != TX_SCRIPTHASH;
        // Append serialized subscript whether or not it is completely signed:
        txin.scriptSig << static_cast<valtype>(subscript);
        if (!fSolved) return false;
    }

    // Test solution
    const CAmount& amount = txTo.vout[txin.prevout.n].nValue;
    return VerifyScript(txin.scriptSig, fromPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, MutableTransactionSignatureChecker(&txTo, nIn, amount));
}

bool SignSignature(const CKeyStore &keystore, const CTransaction& txFrom, CMutableTransaction& txTo, unsigned int nIn, int nHashType)
{
    assert(nIn < txTo.vin.size());
    CTxIn& txin = txTo.vin[nIn];
    assert(txin.prevout.n < txFrom.vout.size());
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    return SignSignature(keystore, txout.scriptPubKey, txTo, nIn, nHashType);
}

static CScript PushAll(const vector<valtype>& values)
{
    CScript result;
    for (const valtype& v : values)
        result << v;
    return result;
}

static CScript CombineMultisig(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                               const vector<valtype>& vSolutions,
                               const vector<valtype>& sigs1, const vector<valtype>& sigs2)
{
    // Combine all the signatures we've got:
    set<valtype> allsigs;
    for (const valtype& v : sigs1)
    {
        if (!v.empty())
            allsigs.insert(v);
    }
    for (const valtype& v : sigs2)
    {
        if (!v.empty())
            allsigs.insert(v);
    }

    // Build a map of pubkey -> signature by matching sigs to pubkeys:
    assert(vSolutions.size() > 1);
    unsigned int nSigsRequired = vSolutions.front()[0];
    unsigned int nPubKeys = vSolutions.size()-2;
    map<valtype, valtype> sigs;
    for (const valtype& sig : allsigs)
    {
        for (unsigned int i = 0; i < nPubKeys; i++)
        {
            const valtype& pubkey = vSolutions[i+1];
            if (sigs.count(pubkey))
                continue; // Already got a sig for this pubkey

            // TODO
            if (TransactionSignatureChecker(&txTo, nIn, 0).CheckSig(sig, pubkey, scriptPubKey))
            {
                sigs[pubkey] = sig;
                break;
            }
        }
    }
    // Now build a merged CScript:
    unsigned int nSigsHave = 0;
    CScript result; result << OP_0; // pop-one-too-many workaround
    for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++)
    {
        if (sigs.count(vSolutions[i+1]))
        {
            result << sigs[vSolutions[i+1]];
            ++nSigsHave;
        }
    }
    // Fill any missing with OP_0:
    for (unsigned int i = nSigsHave; i < nSigsRequired; i++)
        result << OP_0;

    return result;
}

static CScript CombineSignatures(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                                 const txnouttype txType, const vector<valtype>& vSolutions,
                                 vector<valtype>& sigs1, vector<valtype>& sigs2)
{
    switch (txType)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
        // Don't know anything about this, assume bigger one is correct:
        if (sigs1.size() >= sigs2.size())
            return PushAll(sigs1);
        return PushAll(sigs2);
    case TX_PUBKEY:
    case TX_PUBKEYHASH:
        // Signatures are bigger than placeholders or empty scripts:
        if (sigs1.empty() || sigs1[0].empty())
            return PushAll(sigs2);
        return PushAll(sigs1);
    case TX_SCRIPTHASH:
        if (sigs1.empty() || sigs1.back().empty())
            return PushAll(sigs2);
        else if (sigs2.empty() || sigs2.back().empty())
            return PushAll(sigs1);
        else
        {
            // Recur to combine:
            valtype spk = sigs1.back();
            CScript pubKey2(spk.begin(), spk.end());

            vector<vector<unsigned char> > vSolutions2;
            txnouttype txType2 = Solver(pubKey2, vSolutions2);
            sigs1.pop_back();
            sigs2.pop_back();
            CScript result = CombineSignatures(pubKey2, txTo, nIn, txType2, vSolutions2, sigs1, sigs2);
            result << spk;
            return result;
        }
    case TX_MULTISIG:
        return CombineMultisig(scriptPubKey, txTo, nIn, vSolutions, sigs1, sigs2);
    }

    return CScript();
}

CScript CombineSignatures(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn,
                          const CScript& scriptSig1, const CScript& scriptSig2)
{
    vector<vector<unsigned char> > vSolutions;
    txnouttype txType = Solver(scriptPubKey, vSolutions);

    vector<valtype> stack1;
    EvalScript(stack1, scriptSig1, SCRIPT_VERIFY_STRICTENC, BaseSignatureChecker());
    vector<valtype> stack2;
    EvalScript(stack2, scriptSig2, SCRIPT_VERIFY_STRICTENC, BaseSignatureChecker());

    return CombineSignatures(scriptPubKey, txTo, nIn, txType, vSolutions, stack1, stack2);
}

MutableTransactionSignatureOutputCreator::MutableTransactionSignatureOutputCreator(const CMutableTransaction* txToIn, unsigned int nOutIn, const CAmount& amountIn, int nHashTypeIn) : txTo(txToIn), nOut(nOutIn), nHashType(nHashTypeIn), amount(amountIn), checker(txTo, nOut, amountIn) {
}

bool MutableTransactionSignatureOutputCreator::CreateSig(const SigningProvider& provider, std::vector<unsigned char>& vchSig, const CKeyID& address, const CScript& scriptCode) const {
    CKey key;
    if (!provider.GetKey(address, key))
        return false;

    uint256 hash = SignatureHashOutput(scriptCode, *txTo, nOut, nHashType, amount);
    if (!key.Sign(hash, vchSig))
        return false;
    vchSig.push_back((unsigned char) nHashType);
    return true;
}

static bool GetCScript(const SigningProvider& provider,
        const SignatureData& sigdata,
        const CScriptID& scriptid,
        CScript& script) {
    if (provider.GetCScript(scriptid, script)) {
        return true;
    }
    // Look for scripts in SignatureData
    if (CScriptID(sigdata.redeem_script) == scriptid) {
        script = sigdata.redeem_script;
        return true;
    }
    return false;
}

static bool GetPubKey(const SigningProvider& provider,
        const SignatureData& sigdata,
        const CKeyID& address,
        CPubKey& pubkey) {
    // TODO
//    // Look for pubkey in all partial sigs
//    const auto it = sigdata.signatures.find(address);
//    if (it != sigdata.signatures.end()) {
//        pubkey = it->second.first;
//        return true;
//    }
//    // Look for pubkey in pubkey list
//    const auto& pk_it = sigdata.misc_pubkeys.find(address);
//    if (pk_it != sigdata.misc_pubkeys.end()) {
//        pubkey = pk_it->second.first;
//        return true;
//    }
    // Query the underlying provider
    return provider.GetPubKey(address, pubkey);
}

static bool CreateSig(const BaseSignatureCreator& creator,
        SignatureData& sigdata,
        const SigningProvider& provider,
        std::vector<unsigned char>& sig_out,
        const CPubKey& pubkey,
        const CScript& scriptcode) {
    // TODO
//    CKeyID keyid = pubkey.GetID();
//    const auto it = sigdata.signatures.find(keyid);
//    if (it != sigdata.signatures.end()) {
//        sig_out = it->second.second;
//        return true;
//    }
//    KeyOriginInfo info;
//    if (provider.GetKeyOrigin(keyid, info)) {
//        sigdata.misc_pubkeys.emplace(keyid, std::make_pair(pubkey, std::move(info)));
//    }
//    if (creator.CreateSig(provider, sig_out, keyid, scriptcode)) {
//        auto i = sigdata.signatures.emplace(keyid, SigPair(pubkey, sig_out));
//        assert(i.second);
//        return true;
//    }
//    // Could not make signature or signature not found, add keyid to missing
//    sigdata.missing_sigs.push_back(keyid);
    return false;
}

/**
 * Sign scriptPubKey using signature made with creator.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey can't be signed),
 * unless whichTypeRet is TX_SCRIPTHASH, in which case scriptSigRet is the redemption script.
 * Returns false if scriptPubKey could not be completely satisfied.
 */
static bool SignStep(const SigningProvider& provider,
        const BaseSignatureCreator& creator,
        const CScript& scriptPubKey,
        std::vector<valtype>& ret,
        txnouttype& whichTypeRet,
        SignatureData& sigdata) {
    CScript scriptRet;
    uint160 h160;
    ret.clear();
    std::vector<unsigned char> sig;

    std::vector<valtype> vSolutions;
    whichTypeRet = Solver(scriptPubKey, vSolutions);

    switch (whichTypeRet) {
        case TX_NONSTANDARD:
        case TX_NULL_DATA:
            return false;
        case TX_PUBKEY:
            if (!CreateSig(creator, sigdata, provider, sig, CPubKey(vSolutions[0]), scriptPubKey)) return false;
            ret.push_back(std::move(sig));
            return true;
        case TX_PUBKEYHASH:
        {
            CKeyID keyID = CKeyID(uint160(vSolutions[0]));
            CPubKey pubkey;
            if (!GetPubKey(provider, sigdata, keyID, pubkey)) {
                // Pubkey could not be found, add to missing
                sigdata.missing_pubkeys.push_back(keyID);
                return false;
            }
            if (!CreateSig(creator, sigdata, provider, sig, pubkey, scriptPubKey)) return false;
            ret.push_back(std::move(sig));
            ret.push_back(ToByteVector(pubkey));
            return true;
        }
        case TX_SCRIPTHASH:
            h160 = uint160(vSolutions[0]);
            if (GetCScript(provider, sigdata, h160, scriptRet)) {
                ret.push_back(std::vector<unsigned char>(scriptRet.begin(), scriptRet.end()));
                return true;
            }
            // Could not find redeemScript, add to missing
            sigdata.missing_redeem_script = h160;
            return false;

        case TX_MULTISIG:
        {
            size_t required = vSolutions.front()[0];
            ret.push_back(valtype()); // workaround CHECKMULTISIG bug
            for (size_t i = 1; i < vSolutions.size() - 1; ++i) {
                CPubKey pubkey = CPubKey(vSolutions[i]);
                if (ret.size() < required + 1 && CreateSig(creator, sigdata, provider, sig, pubkey, scriptPubKey)) {
                    ret.push_back(std::move(sig));
                }
            }
            bool ok = ret.size() == required + 1;
            for (size_t i = 0; i + ret.size() < required + 1; ++i) {
                ret.push_back(valtype());
            }
            return ok;
        }

        default:
            return false;
    }
}

bool ProduceSignature(const SigningProvider& provider,
        const BaseSignatureCreator& creator,
        const CScript& fromPubKey,
        SignatureData& sigdata) {
    if (sigdata.complete) return true;

    std::vector<valtype> result;
    txnouttype whichType;
    bool solved = SignStep(provider, creator, fromPubKey, result, whichType, sigdata);
    bool P2SH = false;
    CScript subscript;

    if (solved && whichType == TX_SCRIPTHASH) {
        // Solver returns the subscript that needs to be evaluated;
        // the final scriptSig is the signatures from that
        // and then the serialized subscript:
        subscript = CScript(result[0].begin(), result[0].end());
        sigdata.redeem_script = subscript;
        solved = solved && SignStep(provider, creator, subscript, result, whichType, sigdata) && whichType != TX_SCRIPTHASH;
        P2SH = true;
    }

    if (P2SH) {
        result.push_back(std::vector<unsigned char>(subscript.begin(), subscript.end()));
    }
    sigdata.scriptSig = PushAll(result);

    // Test solution
    sigdata.complete = solved && VerifyScript(sigdata.scriptSig, fromPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, creator.Checker());
    return sigdata.complete;
}

bool UpdateOutput(CTxOut &output, const SignatureData &data) {
    bool ret = false;
    CDataStream streamSig(SER_NETWORK, PROTOCOL_VERSION);
    streamSig << data.scriptSig;
    CScript scriptPubKey;
    if (output.scriptPubKey.UpdateSenderSig(ToByteVector(streamSig), scriptPubKey)) {
        output.scriptPubKey = scriptPubKey;
        ret = true;
    }
    return ret;
}
