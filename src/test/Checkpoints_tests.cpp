// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "uint256.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(Checkpoints_tests)

BOOST_AUTO_TEST_CASE(sanity)
{
    uint256 p0 = uint256("0x000007b556429edd30fc5a0736451513896ac7b5df3570f1b903d812b8d1f01f");
    uint256 p40300 = uint256("0x0000000000003b6e75d462fba032e235c76b20b4fe3854294ace55df08f2c7e3");
    BOOST_CHECK(Checkpoints::CheckBlock(0, p0));
    BOOST_CHECK(Checkpoints::CheckBlock(40300, p40300));


    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(0, p40300));
    BOOST_CHECK(!Checkpoints::CheckBlock(40300, p0));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(0+1, p40300));
    BOOST_CHECK(Checkpoints::CheckBlock(40300+1, p0));

    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= 40300);
}

BOOST_AUTO_TEST_SUITE_END()
