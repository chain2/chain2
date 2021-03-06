// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uritests.h"

#include "chainparams.h"
#include "guiutil.h"
#include "options.h"
#include "walletmodel.h"

#include <QUrl>

void URITests::uriTestsBase58() {
    SelectParams(CBaseChainParams::MAIN);

    SendCoinsRecipient rv;
    QString scheme =
        QString::fromStdString(Params(CBaseChainParams::MAIN).CashAddrPrefix());
    QUrl uri;
    uri.setUrl(QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?label=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=100&label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI(scheme, "ctwo://175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?message=Wikipedia Example Address", &rv));
    QVERIFY(rv.address == QString("175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?req-message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1,000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString("ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?amount=1,000.0&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));
}

void URITests::uriTestsCashAddr() {
    SendCoinsRecipient rv;
    QUrl uri;
    QString scheme =
        QString::fromStdString(Params(CBaseChainParams::MAIN).CashAddrPrefix());
    uri.setUrl(QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?"
                       "req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(
        QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?label="
                "Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?amount=100&"
        "label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?message="
        "Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI(
        scheme, "ctwo://"
                "qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?"
                "message=Wikipedia Example Address",
        &rv));
    QVERIFY(rv.address ==
            QString("ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?req-message="
        "Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?amount=1,"
        "000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));

    uri.setUrl(QString(
        "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?amount=1,"
        "000.0&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(scheme, uri, &rv));
}

void URITests::uriTestFormatURI() {

    auto arg = new DummyArgGetter;
    auto raii = SetDummyArgGetter(std::unique_ptr<ArgGetter>(arg));
    {
        arg->Set("-usecashaddr", 1);
        SendCoinsRecipient r;
        r.address = "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw";
        r.message = "test";
        QString uri = GUIUtil::formatBitcoinURI(r);
        QVERIFY(uri == "ctwo:qpm2qsznhks23z7629mms6s4cwef74vcwv3262c7tw?"
                       "message=test");
    }

    {
        arg->Set("-usecashaddr", 0);
        SendCoinsRecipient r;
        r.address = "175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W";
        r.message = "test";
        QString uri = GUIUtil::formatBitcoinURI(r);
        QVERIFY(uri ==
                "ctwo:175tWpb8K1S7NmH4Zx6rewF9WQrcZv245W?message=test");
    }
}

void URITests::uriTestScheme() {
    auto arg = new DummyArgGetter;
    auto raii = SetDummyArgGetter(std::unique_ptr<ArgGetter>(arg));
    {
        // cashaddr - scheme depends on selected chain params
        arg->Set("-usecashaddr", 1);
        QVERIFY("ctwo" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::MAIN)));
        QVERIFY("ctwotest" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::TESTNET)));
        QVERIFY("ctworeg" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::REGTEST)));
    }
    {
        // legacy - scheme is "bitcoin" regardless of chain params
        arg->Set("-usecashaddr", 0);
        QVERIFY("ctwo" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::MAIN)));
        QVERIFY("ctwo" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::TESTNET)));
        QVERIFY("ctwo" == GUIUtil::bitcoinURIScheme(Params(CBaseChainParams::REGTEST)));
    }
}
