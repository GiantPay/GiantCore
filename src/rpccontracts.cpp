// Copyright (c) 2018 The GIANT developers

#include "base58.h"
#include "rpcserver.h"

#include <boost/assign/list_of.hpp>

using namespace boost;
using namespace boost::assign;
using namespace json_spirit;
using namespace std;

Value ContractDeploy(const CBitcoinAddress& address, string code, double prioritet)
{
  return 0;
}

Value ContractCall(bool readonly, double prioritet,const CBitcoinAddress& address, string methodName, Array args)
{
  return 0;
}

Value ContractAddress(string strHex)
{
    return 0;
}

Value ContractInfo(const CBitcoinAddress& address, bool calculateProperties, bool callReadonlyMethods)
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

Value ContractDeployEstimate(string code, double prioritet, uint64_t loopCount)
{
    return 0;
}

Value ContractCallEstimate(const CBitcoinAddress& address, const string& methodName, double prioritet, uint64_t loopCount)
{
    return 0;
}

Value ContractCallEstimate()
{
    return 0;
}

Value contractdeploy(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "contractdeploy \"address\" \"code\" \"prioritet:\"\n"
            "\nArguments:\n"
            "\naddress - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
            "\ncode - (string) smart contract code\n"
            "\nprioritet - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "\nCreates transaction which deploit the smart contract and sends it to mempool\n"
            "\nResult:\n"
            "\n    if transaction was added to mempool - that the method will return a transaction hash\n"
            "\nExamples:\n" +
            HelpExampleCli("contractdeploy", "") + HelpExampleRpc("contractdeploy", ""));

    RPCTypeCheck(params, list_of(str_type)(str_type)(real_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    std::string code = params[1].get_str();

    double prioritet = params[2].get_real();
    if (prioritet < 1.0)
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");

    return ContractDeploy(address, code, prioritet);
}

Value contractcall(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 5)
        throw runtime_error(
            "contractcall \"readonly\" \"prioritet:\" \"address\" \"methodName\" \"args\" \n"
            "\nArguments:\n"
            "\nreadonly - (bool: true|false) if readonly=true and in a call of a method goes\n"
            "             attempt that to change in a blockchain (for example to change a variable\n"
            "             in the smart contract) there will be a mistake\n"
            "\nprioritet - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "\naddress - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
            "\nmethodName - (string) name of a method\n"
            "\nargs - (array) method call parameter"
            "\nResult:\n"
            "\nif the readonly=true method that returns value which will return a smart contract method\n"
            "\nif the readonly=false method and transaction it is successfully added that the transaction hash will return\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcall", "") + HelpExampleRpc("contractcall", ""));

    RPCTypeCheck(params, list_of(bool_type)(real_type)(str_type)(str_type)(array_type));

    bool readonly = params[0].get_bool();

    double prioritet = params[1].get_real();
    if (prioritet < 1.0)
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");

    CBitcoinAddress address(params[2].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    string methodName = params[3].get_str();

    Array args = params[4].get_array();

    return ContractCall(readonly, prioritet, address, methodName, args);
}

Value contractaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractaddress \"txid\"\n"
            "\nArguments:\n"
            "\ntxid - (string) transaction hash, line 64 symbols long\n"
            "\nReturns the smart contract address on a transaction hash\n"
            "\nResult:\n"
            "\n    the transaction address if the contract is found\n"
            "\nExamples:\n" +
            HelpExampleCli("contractaddress", "") + HelpExampleRpc("contractaddress", ""));

    RPCTypeCheck(params, list_of(str_type));

    string strHex = params[0].get_str();

    return ContractAddress(strHex);
}

Value contractinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "contractinfo \"address\" \"calculateProperties\" \"callReadonlyMethods\" \n"
            "\nArguments:\n"
            "\naddress - (string) public key through which indication the private key for the signature of transaction will be chosen\n"
            "\ncalculateProperties - (bool) if true that are calculated values of all fields. If that false isn't specified\n"
            "\ncallReadonlyMethods - (bool) if true that are calculated values of all readonly of methods. If that false isn't specified\n"
            "\nReturns information on the smart contract on his address\n"
            "\nResult:\n"
            "\ndata on the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractinfo", "") + HelpExampleRpc("contractinfo", ""));

    RPCTypeCheck(params, list_of(str_type)(bool_type)(bool_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    bool calculateProperties = params[1].get_bool();

    bool callReadonlyMethods = params[2].get_bool();

    return ContractInfo( address, calculateProperties, callReadonlyMethods);
}

Value contractcode(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractcode \"address\"\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn a smart contract code to his address\n"
            "\nResult:\n"
            "\n smart contract code as he is kept in transaction\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcode", "") + HelpExampleRpc("contractcode", ""));

    RPCTypeCheck(params, list_of(str_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    return ContractCode(address);
}

Value contractbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractbalance \"address\"\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn balance of the smart contract to his address\n"
            "\nResult:\n"
            "\nbalance of the contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractbalance", "") + HelpExampleRpc("contractbalance", ""));

    RPCTypeCheck(params, list_of(str_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    return ContractBalance(address);
}

Value contractstatus(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "contractstatus \"address\"\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nReturn a state it (is active, removed, suspended) the smart contract to his address\n"
            "\nResult:\n"
            "\ncondition of the contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractstatus", "") + HelpExampleRpc("contractstatus", ""));

    RPCTypeCheck(params, list_of(str_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    return ContractStatus(address);
}

Value contractdeployestimate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
        throw runtime_error(
            "contractdeployestimate \"code\" \"prioritet:\" \"loopCount\"\n"
            "\nArguments:\n"
            "\ncode - (string) smart contract code\n"
            "\nprioritet - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "\nloopCount (uint64_t) quantity of cycles for calculation of the commission, by default value 1"
            "\nEstimates smart contract deploy cost\n"
            "\nResult:\n"
            "\nvalue of the commission and plan of implementation of the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractdeployestimate", "") + HelpExampleRpc("contractdeployestimate", ""));

    RPCTypeCheck(params, list_of(str_type)(real_type)(int_type));

    string code = params[0].get_str();

    double prioritet = params[1].get_real();
    if (prioritet < 1.0)
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");

    uint64_t loopCount = params[2].get_uint64();

    return ContractDeployEstimate(code, prioritet, loopCount);
}

Value contractcallestimate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 4)
        throw runtime_error(
            "contractcallestimate \"address\" \"methodName\" \"prioritet:\" \"loopCount\"\n"
            "\nArguments:\n"
            "\naddress - (string) the contract address, the line containing the valid address of the smart contract\n"
            "\nmethodName - (string) smart contract method"
            "\nprioritet - (double) the weight influencing the size of the commission burned for warm the smart contract"
            "\nloopCount (uint64_t) quantity of cycles for calculation of the commission, by default value 1"
            "\nEstimates the smart contract method call cost (not readonly)\n"
            "\nResult:\n"
            "\nvalue of the commission and plan of performance of a method of the smart contract\n"
            "\nExamples:\n" +
            HelpExampleCli("contractcallestimate", "") + HelpExampleRpc("contractcallestimate", ""));

    RPCTypeCheck(params, list_of(str_type)(str_type)(real_type)(int_type));

    CBitcoinAddress address(params[0].get_str());
    if (bool isValid = address.IsValid())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Invalid address transaction");

    string methodName = params[1].get_str();

    double prioritet = params[2].get_real();
    if (prioritet < 1.0)
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Wrong prioritet");

    uint64_t loopCount = params[3].get_uint64();

    return ContractCallEstimate(address, methodName, prioritet, loopCount);
}

Value contractcount(const Array& params, bool fHelp)
{
    return ContractCallEstimate();
}