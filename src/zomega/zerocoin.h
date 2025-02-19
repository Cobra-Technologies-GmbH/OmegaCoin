// Copyright (c) 2017-2020 The PIVX developers
// Copyright (c) 2021 The OMEGACOIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OMEGACOIN_ZEROCOIN_H
#define OMEGACOIN_ZEROCOIN_H

#include "uint256.h"
#include "libzerocoin/bignum.h"

uint256 GetPubCoinHash(const CBigNum& bnValue);

#endif //OMEGACOIN_ZEROCOIN_H
