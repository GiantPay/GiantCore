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

string adv = "\"address\":\"GeKm65nBdbZs9MRKd858u3SrFR3yNGNV5q\"";      // valid giant address
string adiv = "\"address\":\"1eKm65nBdbZs9MRKd858u3SrFR3yNGNV5q\"";     // invalid giant address
string jscv = "\"jscode\":\"codejs1\"";                                 // valid js code
string jsciv = "";                                                      // invalid parametr js code
string prv = "\"prioritet\":1";                                         // valid parametr prioritet 1<
string priv = "\"prioritet\":0.1";                                      // invalid parametr prioritet <1
string mNv = "\"methodName\":\"methodName1\"";
string mNiv = "";
string argsv = "\"args\":[\"args\"]";
string argsiv = "";
string rov = "\"readonly\":true";
string calProv = "\"calculateProperties\":true";
string calRoMetv = "\"callReadonlyMethods\":true";
string lCv = "\"loopCount\":1";                                         // valid parametr loopCount 1<
string lCiv = "\"loopCount\":0.1";                                      // invalid parametr loopCount <1

BOOST_AUTO_TEST_SUITE(rpccontracts_tests)

BOOST_AUTO_TEST_CASE(rpc_contractdeploy)
{
    BOOST_CHECK_THROW(CallContRPC("contractdeploy"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeploy ") + "{" + adiv + "," + jscv + "," + prv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeploy ") + "{" + adv + "," + jsciv + "," + prv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeploy ") + "{" + adv + "," + jscv + "," + priv + "}"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractdeploy ") + "{" + adv + "," + jscv + "," + prv + "}") == 0);
}

BOOST_AUTO_TEST_CASE(rpc_contractcall)
{
    BOOST_CHECK_THROW(CallContRPC("contractcall"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcall ") + "{" + rov + "," + prv + "," + adv + "," + mNv + "," + argsv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + priv + "," + adv + "," + mNv + "," + argsv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + rov + "," + prv + "," + adiv + "," + mNv + "," + argsv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + rov + "," + prv + "," + adv + "," + mNiv + "," + argsv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcall ") + "{" + rov + "," + prv + "," + adv + "," + mNv + "," + argsiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractaddress)
{
    BOOST_CHECK_THROW(CallContRPC("contractaddress"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractinfo)
{
    BOOST_CHECK_THROW(CallContRPC("contractinfo"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractinfo ") + "{" + adv + "," + calProv + "," + calRoMetv + "}") == 0);
    BOOST_CHECK(CallContRPC(string("contractinfo ") + "{" + adv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractinfo ") + "{" + adiv + "," + calProv + "," + calRoMetv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractinfo ") + "{" + adiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractcode)
{
    BOOST_CHECK_THROW(CallContRPC("contractcode"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcode ") + "{" + adv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcode ") + "{" + adiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractbalance)
{
    BOOST_CHECK_THROW(CallContRPC("contractbalance"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractbalance ") + "{" + adv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractbalance ") + "{" + adiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractstatus)
{
    BOOST_CHECK_THROW(CallContRPC("contractstatus"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractstatus ") + "{" + adv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractstatus ") + "{" + adiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractdeployestimate)
{
    BOOST_CHECK_THROW(CallContRPC("contractdeployestimate"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractdeployestimate ") + "{" + jscv + "," + prv + "," + lCv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeployestimate ") + "{" + jsciv + "," + prv + "," + lCv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeployestimate ") + "{" + jscv + "," + priv + "," + lCv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractdeployestimate ") + "{" + jscv + "," + prv + "," + lCiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_contractcallestimate)
{
    BOOST_CHECK_THROW(CallContRPC("contractcallestimate"), runtime_error);
    BOOST_CHECK(CallContRPC(string("contractcallestimate ") + "{" + adv + "," + mNv + "," + prv + "," + lCv + "}") == 0);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + adiv + "," + mNv + "," + prv + "," + lCv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + adv + "," + mNiv + "," + prv + "," + lCv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + adv + "," + mNv + "," + priv + "," + lCv + "}"), runtime_error);
    BOOST_CHECK_THROW(CallContRPC(string("contractcallestimate ") + "{" + adv + "," + mNv + "," + prv + "," + lCiv + "}"), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()