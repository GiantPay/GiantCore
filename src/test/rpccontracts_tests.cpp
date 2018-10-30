// Copyright (c) 2018 The GIANT developers

#include "rpcserver.h"
#include "rpcclient.h"

#include "base58.h"
#include "netbase.h"

#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>

#define BOOST_TEST_MODULE rpccontracts_test

using namespace std;
using namespace json_spirit;

Value CallContRPC(string args)
{
    vector<string> vArgs;
    boost::split(vArgs, args, boost::is_any_of(" \t"));
    string strMethod = vArgs[0];
    vArgs.erase(vArgs.begin());
    Array params = RPCConvertValues(strMethod, vArgs);

    rpcfn_type method = tableRPC[strMethod]->actor;
    try {
        Value result = (*method)(params, false);
        return result;
    }
    catch (Object& objError)
    {
        throw runtime_error(find_value(objError, "message").get_str());
    }
}

string sAddressValid = "\"address\":\"GeKm65nBdbZs9MRKd858u3SrFR3yNGNV5q\"";
string sAddressInvalid = "\"address\":\"1eKm65nBdbZs9MRKd858u3SrFR3yNGNV5q\"";
string sPrioritetValid = "\"prioritet\":1";
string sPrioritetInvalid = "\"prioritet\":0.1";
string sMethodNameValid = "\"methodName\":\"methodName1\"";
string sMethodNameInvalid = "";
string sArgsValid = "\"args\":[\"args\"]";
string sArgsInvalid = "";
string sReadOnlyValid = "\"readonly\":true";
string sCalculateProperties = "\"calculateProperties\":true";
string sCallReadOnlyMethodsValid = "\"callReadonlyMethods\":true";
string sLoopCountValid = "\"loopCount\":1";
string sLoopCountInvalid = "\"loopCount\":0.1";
string sJsCode = "\"jscode\":\"Hello+,World!\"";
string sTxidInvalid = "\"txid\":\"a3b807410df0b60fcb9736768df5823938b2f838694939ba45f3c0a1bff150ed1\"";

BOOST_AUTO_TEST_SUITE(rpccontracts_tests)

BOOST_AUTO_TEST_CASE(rpc_contractdeploy)
{
    BOOST_CHECK_THROW(CallContRPC("contractdeploy"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeploy ") + "{" + sAddressInvalid + "," + sJsCode + "," + sPrioritetValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeploy ") + "{" + sAddressValid + "," + sJsCode + "," + sPrioritetInvalid + "}"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractdeploy ") + "{" + sAddressValid + "," + sJsCode + "," + sPrioritetValid + "}") == 0);
}

BOOST_AUTO_TEST_CASE(rpc_contractcall)
{
    BOOST_CHECK_THROW(CallContRPC("contractcall"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcall ") + "{" + sReadOnlyValid + "," + sPrioritetValid + "," + sAddressValid + "," + sMethodNameValid + "," + sArgsValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + sPrioritetInvalid + "," + sAddressValid + "," + sMethodNameValid + "," + sArgsValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + sReadOnlyValid + "," + sPrioritetValid + "," + sAddressInvalid + "," + sMethodNameValid + "," + sArgsValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + sReadOnlyValid + "," + sPrioritetValid + "," + sAddressValid + "," + sMethodNameInvalid + "," + sArgsValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + sReadOnlyValid + "," + sPrioritetValid + "," + sAddressValid + "," + sMethodNameValid + "," + sArgsInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractaddress)
{
    BOOST_CHECK_THROW(CallContRPC("contractaddress"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractaddress ") + "{" + sTxidInvalid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractaddress ") + "{" + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractinfo)
{
    BOOST_CHECK_THROW(CallContRPC("contractinfo"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractinfo ") + "{" + sAddressValid + "," + sCalculateProperties + "," + sCallReadOnlyMethodsValid + "}") == 0);
    BOOST_CHECK(CallContRPC(string("contractinfo ") + "{" + sAddressValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractinfo ") + "{" + sAddressInvalid + "," + sCalculateProperties + "," + sCallReadOnlyMethodsValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractinfo ") + "{" + sAddressInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractcode)
{
    BOOST_CHECK_THROW(CallContRPC("contractcode"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcode ") + "{" + sAddressValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcode ") + "{" + sAddressInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractbalance)
{
    BOOST_CHECK_THROW(CallContRPC("contractbalance"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractbalance ") + "{" + sAddressValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractbalance ") + "{" + sAddressInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractstatus)
{
    BOOST_CHECK_THROW(CallContRPC("contractstatus"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractstatus ") + "{" + sAddressValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractstatus ") + "{" + sAddressInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractdeployestimate)
{
    BOOST_CHECK_THROW(CallContRPC("contractdeployestimate"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractdeployestimate ") + "{" + sJsCode + "," + sPrioritetValid + "," + sLoopCountValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeployestimate ") + "{" + sJsCode + "," + sPrioritetInvalid + "," + sLoopCountValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeployestimate ") + "{" + sJsCode + "," + sPrioritetValid + "," + sLoopCountInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractcallestimate)
{
    BOOST_CHECK_THROW(CallContRPC("contractcallestimate"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcallestimate ") + "{" + sAddressValid + "," + sMethodNameValid + "," + sPrioritetValid + "," + sLoopCountValid + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + sAddressInvalid + "," + sMethodNameValid + "," + sPrioritetValid + "," + sLoopCountValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + sAddressValid + "," + sMethodNameInvalid + "," + sPrioritetValid + "," + sLoopCountValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + sAddressValid + "," + sMethodNameValid + "," + sPrioritetInvalid + "," + sLoopCountValid + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + sAddressValid + "," + sMethodNameValid + "," + sPrioritetValid + "," + sLoopCountInvalid + "}"), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()