// Copyright (c) 2018 The GIANT developers

#include "rpcserver.h"
#include "rpcclient.h"

#include "base58.h"
#include "netbase.h"

#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>

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


BOOST_AUTO_TEST_SUITE(rpccontracts_test)

BOOST_AUTO_TEST_CASE(rpc_contractsparams)
{
    BOOST_CHECK_THROW(CallContRPC("contractdeploy"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractcall"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractaddress"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractinfo"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractcode"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractbalance"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractstatus"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractdeployestimate"), runtime_error);

    BOOST_CHECK_THROW(CallContRPC("contractcallestimate"), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()