// Copyright (c) 2021 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php.

#include "test/test_omegacoin.h"
#include "primitives/transaction.h"
#include "evo/providertx.h"
#include "evo/specialtx.h"
#include "messagesigner.h"
#include "netbase.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(evo_specialtx_tests, TestingSetup)

static CKey GetRandomKey()
{
    CKey key;
    key.MakeNewKey(true);
    return key;
}

static CKeyID GetRandomKeyID()
{
    return GetRandomKey().GetPubKey().GetID();
}

static CScript GetRandomScript()
{
    return GetScriptForDestination(GetRandomKeyID());
}

static ProRegPL GetRandomProRegPayload()
{
    ProRegPL pl;
    pl.collateralOutpoint.hash = GetRandHash();
    pl.collateralOutpoint.n = InsecureRandBits(2);
    BOOST_CHECK(Lookup("57.12.210.11:51472", pl.addr, Params().GetDefaultPort(), false));
    pl.keyIDOwner = GetRandomKeyID();
    pl.keyIDOperator = GetRandomKeyID();
    pl.keyIDVoting = GetRandomKeyID();
    pl.scriptPayout = GetRandomScript();
    pl.nOperatorReward = InsecureRandRange(10000);
    pl.scriptOperatorPayout = GetRandomScript();
    pl.inputsHash = GetRandHash();
    pl.vchSig = InsecureRandBytes(63);
    return pl;
}

BOOST_AUTO_TEST_CASE(special_tx_validation_test)
{
    CMutableTransaction mtx;
    CValidationState state;

    // v1 can only be Type=0
    mtx.nType = CTransaction::TxType::PROREG;
    mtx.nVersion = CTransaction::TxVersion::LEGACY;
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-type-version");

    // version >= Sapling, type = 0, payload != null.
    mtx.nType = CTransaction::TxType::NORMAL;
    mtx.extraPayload = std::vector<uint8_t>(10, 1);
    mtx.nVersion = CTransaction::TxVersion::SAPLING;
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-type-payload");

    // version >= Sapling, type = 0, payload == null --> pass
    mtx.extraPayload = nullopt;
    BOOST_CHECK(CheckSpecialTxNoContext(CTransaction(mtx), state));

    // nVersion>=2 and nType!=0 without extrapayload
    mtx.nType = CTransaction::TxType::PROREG;
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK(state.GetRejectReason().find("without extra payload"));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-payload-empty");

    // Size limits
    mtx.extraPayload = std::vector<uint8_t>(MAX_SPECIALTX_EXTRAPAYLOAD + 1, 1);
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-payload-oversize");

    // Remove one element, so now it passes the size check
    mtx.extraPayload->pop_back();
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-protx-payload");

    // valid payload but invalid inputs hash
    mtx.extraPayload->clear();
    ProRegPL pl = GetRandomProRegPayload();
    SetTxPayload(mtx, pl);
    BOOST_CHECK(!CheckSpecialTxNoContext(CTransaction(mtx), state));
    BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-protx-inputs-hash");

    // all good.
    mtx.vin.emplace_back(GetRandHash(), 0);
    mtx.extraPayload->clear();
    pl.inputsHash = CalcTxInputsHash(CTransaction(mtx));
    SetTxPayload(mtx, pl);
    BOOST_CHECK(CheckSpecialTxNoContext(CTransaction(mtx), state));
}

BOOST_AUTO_TEST_CASE(providertx_setpayload_test)
{
    const ProRegPL& pl = GetRandomProRegPayload();

    CMutableTransaction mtx;
    SetTxPayload(mtx, pl);
    ProRegPL pl2;
    BOOST_CHECK(GetTxPayload(mtx, pl2));
    BOOST_CHECK(pl.collateralOutpoint == pl2.collateralOutpoint);
    BOOST_CHECK(pl.addr  == pl2.addr);
    BOOST_CHECK(pl.keyIDOwner == pl2.keyIDOwner);
    BOOST_CHECK(pl.keyIDOperator == pl2.keyIDOperator);
    BOOST_CHECK(pl.keyIDVoting == pl2.keyIDVoting);
    BOOST_CHECK(pl.scriptPayout == pl2.scriptPayout);
    BOOST_CHECK(pl.nOperatorReward  == pl2.nOperatorReward);
    BOOST_CHECK(pl.scriptOperatorPayout == pl2.scriptOperatorPayout);
    BOOST_CHECK(pl.inputsHash == pl2.inputsHash);
    BOOST_CHECK(pl.vchSig == pl2.vchSig);
}

BOOST_AUTO_TEST_CASE(providertx_checkstringsig_test)
{
    ProRegPL pl = GetRandomProRegPayload();
    pl.vchSig.clear();
    const CKey& key = GetRandomKey();
    BOOST_CHECK(CMessageSigner::SignMessage(pl.MakeSignString(), pl.vchSig, key));

    std::string strError;
    const CKeyID& keyID = key.GetPubKey().GetID();
    BOOST_CHECK(CMessageSigner::VerifyMessage(keyID, pl.vchSig, pl.MakeSignString(), strError));
    // Change owner address or script payout
    pl.keyIDOwner = GetRandomKeyID();
    BOOST_CHECK(!CMessageSigner::VerifyMessage(keyID, pl.vchSig, pl.MakeSignString(), strError));
    pl.scriptPayout = GetRandomScript();
    BOOST_CHECK(!CMessageSigner::VerifyMessage(keyID, pl.vchSig, pl.MakeSignString(), strError));
}


BOOST_AUTO_TEST_SUITE_END()
