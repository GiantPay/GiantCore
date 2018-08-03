// Copyright (c) 2014 The Bitcoin Core developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers 
// Copyright (c) 2015-2017 The ALQO developers
// Copyright (c) 2018 The GIANT developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"
#include "main.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(main_tests)

CAmount nMoneySupplyPoWEnd = 57879300 * COIN;

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    CAmount nSum = 0;
    for (int nHeight = 0; nHeight < 1; nHeight += 1) {
        /* premine in block 1 */
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 200000 * COIN);
        nSum += nSubsidy;
    }

    for (int nHeight = 1; nHeight < 501; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 0 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 501; nHeight < 20001; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 10 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 20001; nHeight < 40001; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 15 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 40001; nHeight < 262801; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 20 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 262801; nHeight < 525601; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 10 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 525601; nHeight < 788401; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 8 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 788401; nHeight < 1051201; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 6 * COIN);
        nSum += nSubsidy;
    }
    for (int nHeight = 1051201; nHeight < 1314001; nHeight += 1) {
        CAmount nSubsidy = GetBlockValue(nHeight);
        BOOST_CHECK(nSubsidy <= 4 * COIN);
        nSum += nSubsidy;
    }
	BOOST_CHECK(nSum > 0 && nSum <= nMoneySupplyPoWEnd);
}

BOOST_AUTO_TEST_SUITE_END()
