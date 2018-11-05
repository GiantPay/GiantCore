// Copyright (c) 2018 The GIANT developers

#include "base58.h"
#include "rpcserver.h"
#include "net.h"
#include "util.h"

#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace boost;
using namespace json_spirit;
using namespace std;

Value ContractDeploy(const CBitcoinAddress& oAddressObject, const string &sJsCode, const double dPrioritet)
{
    return 0;
}

Value ContractCall(bool bReadOnly, const double dPrioritet,const CBitcoinAddress& oAddressObject, const string& sMethodName, const Array& aArgs)
{
  return 0;
}

Value ContractAddress(const string& sStrHex)
{
    return 0;
}

Value ContractInfo(const CBitcoinAddress& oAddressObject, bool bCalculateProperties, bool bCallReadonlyMethods)
{
    return 0;
}

Value ContractCode(const CBitcoinAddress& oAddressObject)
{
    return 0;
}

Value ContractBalance(const CBitcoinAddress& oAddressObject)
{
    return 0;
}

Value ContractStatus(const CBitcoinAddress& oAddressObject)
{
    return 0;
}

Value ContractDeployEstimate(const string &sJsCode, const double dPrioritet, uint64_t uiLoopCount)
{
    return 0;
}

Value ContractCallEstimate(const CBitcoinAddress& oAddressObject, const string& sMethodName, const double dPrioritet, uint64_t uiLoopCount)
{
    return 0;
}

Value ContractCount()
{
    return 0;
}

Value contractdeploy(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
                "contractdeploy '{\"address\":\"address\",\"jscode\":\"code\",\"prioritet\":123}'\n"
                "\nCreates transaction which deploit the smart contract and sends it to mempool\n"
                "\nArguments:\n"
                "       {\n"
                "         \"address\":\"address\", - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
                "         \"jscode\":\"jscode\", - (string) smart contract code\n"
                "         \"prioritet\": prioritet - (double) the weight influencing the size of the commission burned for warm the smart contract\n"
                "       }\n"
                "\nResult:\n"
                "\n    if transaction was added to mempool - that the method will return a transaction hash\n"
                "       }\n"
                "    ]\n"
                );

    Object oDeployInfo = params[0].get_obj();
    string sAddress;
    string sJsCode;
    double dPrioritet = 1;

    BOOST_FOREACH(const Pair& s, oDeployInfo) {
        if (s.name_ == "address") {
            sAddress = s.value_.get_str();
        } else if (s.name_ == "jscode") {
            sJsCode = s.value_.get_str();
        } else if (s.name_ == "prioritet") {
            dPrioritet = s.value_.get_real();
        }
    };

    if (sJsCode.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid JScode");
    }

    if (dPrioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");
    }

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractDeploy(oAddressObject, sJsCode, dPrioritet);
}

Value contractcall(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractcall '{\"readonly\":true/false,\"prioritet\":123,\"address\":\"address\",\"methodName\":\"methodName\",\"args\":[\"args\"]}' \n"
            "\nArguments:\n"
            "       {\n"
            "         \"readonly\": true/false,- (bool: true|false) if readonly=true and in a call of a method goes\n"
            "                                    attempt that to change in a blockchain (for example to change a variable\n"
            "                                    in the smart contract) there will be a mistake\n"
            "         \"prioritet\": prioritet, - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "         \"address\": \"address\", - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
            "         \"methodName\": \"methodName\", - (string) name of a method\n"
            "         \"args\": args - (array) method call parameter"
            "       }\n"
            "\nResult:\n"
            "\nif the readonly=true method that returns value which will return a smart contract method\n"
            "\nif the readonly=false method and transaction it is successfully added that the transaction hash will return\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcall", "") + HelpExampleRpc("contractcall", ""));

    Object oCallInfo = params[0].get_obj();
    bool bReadOnly;
    double dPrioritet = 1;
    string sAddress;
    string sMethodName;
    Array aArgs;

    BOOST_FOREACH(const Pair& c, oCallInfo) {
        if (c.name_ == "readonly") {
            bReadOnly = c.value_.get_bool();
        } else if (c.name_ == "prioritet") {
            dPrioritet = c.value_.get_real();
        } else if (c.name_ == "address") {
            sAddress = c.value_.get_str();
        } else if (c.name_ == "methodName") {
            sMethodName = c.value_.get_str();
        } else if (c.name_ == "args") {
            aArgs = c.value_.get_array();
        }
    };

    if (dPrioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");
    }

    if (sMethodName.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong methodName parameter");
    }

    if (aArgs.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong args parameter");
    }

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractCall(bReadOnly, dPrioritet, oAddressObject, sMethodName, aArgs);
}

Value contractaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractaddress '{\"txid\":\"txid\"}'\n"
            "\nArguments:\n"
            "\ntxid - (string) transaction hash, line 64 symbols long\n"
            "\nReturns the smart contract address on a transaction hash\n"
            "\nResult:\n"
            "\n    the transaction address if the contract is found\n"
            "\nExamples:\n" +
            HelpExampleCli("contractaddress", "") + HelpExampleRpc("contractaddress", ""));

    Object oAddressInfo = params[0].get_obj();
    string sStrHex;

    BOOST_FOREACH(const Pair& d, oAddressInfo) {
        if (d.name_ == "txid") {
            sStrHex = d.value_.get_str();
        }
    };

    if (sStrHex.size() != 64) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid txid");
    }

    return ContractAddress(sStrHex);
}

Value contractinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractinfo '{\"address\":\"address\",\"calculateProperties\":true/false,\"callReadonlyMethods\":true/false}' \n"
            "\nArguments:\n"
            "       {\n"
            "         \"address\":\"address\", - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
            "         \"calculateProperties\":true, - (bool) if true that are calculated values of all fields. If that false isn't specified\n"
            "         \"callReadonlyMethods\":true - (bool) if true that are calculated values of all readonly of methods. If that false isn't specified\n"
            "       }\n"
            "\nReturns information on the smart contract on his address\n"
            "\nResult:\n"
            "\ndata on the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractinfo", "") + HelpExampleRpc("contractinfo", ""));

    Object oContInfo = params[0].get_obj();
    string sAddress;
    bool bCalculateProperties = false;
    bool bCallReadonlyMethods = false;

    BOOST_FOREACH(const Pair& a, oContInfo) {
        if (a.name_ == "address") {
            sAddress = a.value_.get_str();
        } else if (a.name_ == "calculateProperties") {
            bCalculateProperties = a.value_.get_bool();
        } else if (a.name_ == "callReadonlyMethods") {
            bCallReadonlyMethods = a.value_.get_bool();
        }
    };

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractInfo( oAddressObject, bCalculateProperties, bCallReadonlyMethods);
}

Value contractcode(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractcode '{\"address\":\"address\"}'\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn a smart contract code to his address\n"
            "\nResult:\n"
            "\n smart contract code as he is kept in transaction\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcode", "") + HelpExampleRpc("contractcode", ""));

    Object oCodeInfo = params[0].get_obj();
    string sAddress;

    BOOST_FOREACH(const Pair& cc, oCodeInfo) {
        if (cc.name_ == "address") {
            sAddress = cc.value_.get_str();
        }
    };

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractCode(oAddressObject);
}

Value contractbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractbalance '{\"address\":\"address\"}'\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn balance of the smart contract to his address\n"
            "\nResult:\n"
            "\nbalance of the contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractbalance", "") + HelpExampleRpc("contractbalance", ""));

    Object oBalanceInfo = params[0].get_obj();
    string sAddress;

    BOOST_FOREACH(const Pair& cb, oBalanceInfo) {
        if (cb.name_ == "address") {
            sAddress = cb.value_.get_str();
        }
    };

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractBalance(oAddressObject);
}

Value contractstatus(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractstatus '{\"address\":\"address\"}'\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn a state it (is active, removed, suspended) the smart contract to his address\n"
            "\nResult:\n"
            "\ncondition of the contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractstatus", "") + HelpExampleRpc("contractstatus", ""));

    Object oStatusInfo = params[0].get_obj();
    string sAddress;

    BOOST_FOREACH(const Pair& g, oStatusInfo) {
        if (g.name_ == "address") {
            sAddress = g.value_.get_str();
        }
    };

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractStatus(oAddressObject);
}

Value contractdeployestimate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractdeployestimate '{\"jscode\":\"code\",\"prioritet\":123,\"loopCount\":1}'\n"
            "\nArguments:\n"
            "       {\n"
            "         \"jscode\": \"jscode\", - (string) smart contract code\n"
            "         \"prioritet\": prioritet, - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "         \"loopCount\": loopCount (uint64_t) quantity of cycles for calculation of the commission, by default value 1"
            "       }\n"
            "\nEstimates smart contract deploy cost\n"
            "\nResult:\n"
            "\nvalue of the commission and plan of implementation of the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractdeployestimate", "") + HelpExampleRpc("contractdeployestimate", ""));

    Object oDeployEstimateInfo = params[0].get_obj();
    string sJsCode;
    double dPrioritet = 1;
    uint64_t uiLoopCount =1;

    BOOST_FOREACH(const Pair& b, oDeployEstimateInfo) {
        if (b.name_ == "jscode") {
            sJsCode = b.value_.get_str();
        } else if (b.name_ == "prioritet") {
            dPrioritet = b.value_.get_real();
        } else if (b.name_ == "loopCount") {
            uiLoopCount = b.value_.get_uint64();
        }
    };

    if (sJsCode.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid JScode");
    }

    if (dPrioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");
    }

    if (uiLoopCount < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parameter LoopCount");
    }

    return ContractDeployEstimate(sJsCode, dPrioritet, uiLoopCount);
}

Value contractcallestimate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractcallestimate '{\"address\":\"address\",\"methodName\":\"methodName\",\"prioritet\":123,\"loopCount\":1}'\n"
            "\nArguments:\n"
            "       {\n"
            "         \"address\": \"address\", - (string) the contract address, the line containing the valid address of the smart contract\n"
            "         \"methodName\": \"methodName\", - (string) smart contract method"
            "         \"prioritet\": prioritet, - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "         \"loopCount\": loopCount (uint64_t) quantity of cycles for calculation of the commission, by default value 1"
            "       }\n"
            "\nEstimates the smart contract method call cost (not readonly)\n"
            "\nResult:\n"
            "\nvalue of the commission and plan of performance of a method of the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcallestimate", "") + HelpExampleRpc("contractcallestimate", ""));

    Object oCallEstimateInfo = params[0].get_obj();
    string sAddress;
    string sMethodName;
    double dPrioritet = 1;
    uint64_t uiLoopCount =1;

    BOOST_FOREACH(const Pair& d, oCallEstimateInfo) {
        if (d.name_ == "address") {
            sAddress = d.value_.get_str();
        } else if (d.name_ == "methodName") {
            sMethodName = d.value_.get_str();
        } else if (d.name_ == "prioritet") {
            dPrioritet = d.value_.get_real();
        } else if (d.name_ == "loopCount") {
            uiLoopCount = d.value_.get_uint64();
        }
    };

    if (sMethodName.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong MethodName parameter");
    }

    if (dPrioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");
    }

    if (uiLoopCount < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parameter LoopCount");
    }

    if (sAddress.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }
    CBitcoinAddress oAddressObject(sAddress);
    if (!oAddressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address");
    }

    return ContractCallEstimate(oAddressObject, sMethodName, dPrioritet, uiLoopCount);
}

Value contractcount(const Array& params, bool fHelp)
{
    return ContractCount();
}