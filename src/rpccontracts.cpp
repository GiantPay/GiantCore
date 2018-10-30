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

Value ContractDeploy(const CBitcoinAddress& addressObject, string jscode, const double prioritet)
{
    return 0;
}

Value ContractCall(bool readonly, const double& prioritet,const CBitcoinAddress& addressObject, const string& methodName, const Array& args)
{
  return 0;
}

Value ContractAddress(const string& strHex)
{
    return 0;
}

Value ContractInfo(const CBitcoinAddress& addressObject, bool calculateProperties, bool callReadonlyMethods)
{
    return 0;
}

Value ContractCode(const CBitcoinAddress& address)
{
    return 0;
}

Value ContractBalance(const CBitcoinAddress& address)
{
    return 0;
}

Value ContractStatus(const CBitcoinAddress& address)
{
    return 0;
}

Value ContractDeployEstimate(string jscode, const double& prioritet, uint64_t loopCount)
{
    return 0;
}

Value ContractCallEstimate(const CBitcoinAddress& addressObject, const string& methodName, const double& prioritet, uint64_t loopCount)
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

    Object deployInfo = params[0].get_obj();
    string address;
    string jscode;
    double prioritet = 1;

    BOOST_FOREACH(const Pair& s, deployInfo) {
        if (s.name_ == "address") {
            address = s.value_.get_str();
        } else if (s.name_ == "jscode") {
            jscode = s.value_.get_str();
        } else if (s.name_ == "prioritet") {
            prioritet = s.value_.get_real();
        }
    };

    if (jscode.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid JScode");
    }

    if (prioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr prioritet");
    }

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractDeploy(addressObject, jscode, prioritet);
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

    Object callInfo = params[0].get_obj();
    bool readonly;
    double prioritet = 1;
    string address;
    string methodName;
    Array args;

    BOOST_FOREACH(const Pair& c, callInfo) {
        if (c.name_ == "readonly") {
            readonly = c.value_.get_bool();
        } else if (c.name_ == "prioritet") {
            prioritet = c.value_.get_real();
        } else if (c.name_ == "address") {
            address = c.value_.get_str();
        } else if (c.name_ == "methodName") {
            methodName = c.value_.get_str();
        } else if (c.name_ == "args") {
            args = c.value_.get_array();
        }
    };

    if (prioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr prioritet");
    }

    if (methodName.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong methodName parametr");
    }

    if (args.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong args parametr");
    }

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractCall(readonly, prioritet, addressObject, methodName, args);
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

    Object addressInfo = params[0].get_obj();
    string strHex;

    BOOST_FOREACH(const Pair& d, addressInfo) {
        if (d.name_ == "txid") {
            strHex = d.value_.get_str();
        }
    };

    if (strHex.size() != 64) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid txid transaction");
    }

    return ContractAddress(strHex);
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

    Object contInfo = params[0].get_obj();
    string address;
    bool calculateProperties = false;
    bool callReadonlyMethods = false;

    BOOST_FOREACH(const Pair& a, contInfo) {
        if (a.name_ == "address") {
            address = a.value_.get_str();
        } else if (a.name_ == "calculateProperties") {
            calculateProperties = a.value_.get_bool();
        } else if (a.name_ == "callReadonlyMethods") {
            callReadonlyMethods = a.value_.get_bool();
        }
    };

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractInfo( addressObject, calculateProperties, callReadonlyMethods);
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

    Object codeInfo = params[0].get_obj();
    string address;

    BOOST_FOREACH(const Pair& cc, codeInfo) {
        if (cc.name_ == "address") {
            address = cc.value_.get_str();
        }
    };

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractCode(address);
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

    Object balanceInfo = params[0].get_obj();
    string address;

    BOOST_FOREACH(const Pair& cb, balanceInfo) {
        if (cb.name_ == "address") {
            address = cb.value_.get_str();
        }
    };

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractBalance(address);
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

    Object statusInfo = params[0].get_obj();
    string address;

    BOOST_FOREACH(const Pair& g, statusInfo) {
        if (g.name_ == "address") {
            address = g.value_.get_str();
        }
    };

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractStatus(address);
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

    Object deployestimateInfo = params[0].get_obj();
    string jscode;
    double prioritet = 1;
    uint64_t loopCount =1;

    BOOST_FOREACH(const Pair& b, deployestimateInfo) {
        if (b.name_ == "jscode") {
            jscode = b.value_.get_str();
        } else if (b.name_ == "prioritet") {
            prioritet = b.value_.get_real();
        } else if (b.name_ == "loopCount") {
            loopCount = b.value_.get_uint64();
        }
    };

    if (jscode.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid JScode");
    }

    if (prioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr prioritet");
    }

    if (loopCount < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr loopCount");
    }

    return ContractDeployEstimate(jscode, prioritet, loopCount);
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

    Object callestimateInfo = params[0].get_obj();
    string address;
    string methodName;
    double prioritet = 1;
    uint64_t loopCount =1;

    BOOST_FOREACH(const Pair& d, callestimateInfo) {
        if (d.name_ == "address") {
            address = d.value_.get_str();
        } else if (d.name_ == "methodName") {
            methodName = d.value_.get_str();
        } else if (d.name_ == "prioritet") {
            prioritet = d.value_.get_real();
        } else if (d.name_ == "loopCount") {
            loopCount = d.value_.get_uint64();
        }
    };

    if (methodName.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong methodName parametr");
    }

    if (prioritet < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr prioritet");
    }

    if (loopCount < 1) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong parametr loopCount");
    }

    if (address.empty()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }
    CBitcoinAddress addressObject(address);
    if (!addressObject.IsValid()) {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");
    }

    return ContractCallEstimate(addressObject, methodName, prioritet, loopCount);
}

Value contractcount(const Array& params, bool fHelp)
{
    return ContractCount();
}