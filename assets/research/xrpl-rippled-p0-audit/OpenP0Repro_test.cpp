#include <test/jtx.h>
#include <test/jtx/Env.h>
#include <test/jtx/trust.h>
#include <test/jtx/testline.h>

#include <xrpl/beast/unit_test/suite.h>
#include <xrpl/protocol/Feature.h>

namespace xrpl {
namespace test {

/** Definitive open-P0 repro (AGTI audit).
    Pass = proves unfixed behavior on this binary.
    After fixes: flip expectations to tecFROZEN and tests become regression guards.
*/
class OpenP0Repro_test : public beast::unit_test::suite
{
    void
    testF33_coverWithdrawRegularFreezeOnly()
    {
        testcase("F3.3 LoanBrokerCoverWithdraw — regular-freeze-only destination (P0)");
        using namespace jtx;
        using namespace loanBroker;

        Account const issuer{"issuer"};
        Account const alice{"alice"};
        Account const dest{"dest"};
        Env env(*this);
        Vault vault{env};

        env.fund(XRP(100'000), issuer, alice, dest);
        env(trust(alice, issuer["IOU"](1'000'000)), THISLINE);
        env(trust(dest, issuer["IOU"](1'000'000)), THISLINE);
        env.close();
        PrettyAsset const asset = issuer["IOU"];
        env(pay(issuer, alice, asset(100'000)), THISLINE);
        env.close();

        auto [vtx, vaultKeylet] = vault.create({.owner = alice, .asset = asset});
        env(vtx, THISLINE);
        env.close();
        auto const le = env.le(vaultKeylet);
        if (!BEAST_EXPECT(le))
            return;

        env(vault.deposit({.depositor = alice, .id = vaultKeylet.key, .amount = asset(50)}),
            THISLINE);
        env.close();

        auto const brokerKeylet = keylet::loanbroker(alice.id(), env.seq(alice));
        env(set(alice, vaultKeylet.key), THISLINE);
        env.close();

        env(coverDeposit(alice, brokerKeylet.key, asset(10)), THISLINE);
        env.close();

        env(fclear(dest, asfDepositAuth), THISLINE);
        env.close();

        // Regular freeze ONLY (no tfSetDeepFreeze) — compliance case the code misses
        env(trust(issuer, asset(1'000), dest, tfSetFreeze), THISLINE);
        env.close();

        auto const destBalBefore = env.balance(dest, asset);
        env(coverWithdraw(alice, brokerKeylet.key, asset(10)),
            destination(dest),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const destBalAfter = env.balance(dest, asset);

        BEAST_EXPECT(destBalAfter.number() > destBalBefore.number());
        BEAST_EXPECT(
            destBalAfter.number() - destBalBefore.number() == asset(10).number());

        // Control: deep freeze on same dest IS blocked (upstream LoanBroker_test ~951)
        env(trust(issuer, asset(1'000), dest, tfSetDeepFreeze), THISLINE);
        env.close();
        env(coverWithdraw(alice, brokerKeylet.key, asset(1)),
            destination(dest),
            ter(tecFROZEN),
            THISLINE);
    }

    void
    testF33_controlDeepFreezeBlocks()
    {
        testcase("F3.3 control — deep-freeze blocks cover withdraw");
        using namespace jtx;
        using namespace loanBroker;

        Account const issuer{"issuer"};
        Account const alice{"alice"};
        Account const dest{"dest"};
        Env env(*this);
        Vault vault{env};

        env.fund(XRP(100'000), issuer, alice, dest);
        env(trust(alice, issuer["IOU"](1'000'000)), THISLINE);
        env(trust(dest, issuer["IOU"](1'000'000)), THISLINE);
        env.close();
        PrettyAsset const asset = issuer["IOU"];
        env(pay(issuer, alice, asset(100'000)), THISLINE);
        env.close();

        auto [vtx, vaultKeylet] = vault.create({.owner = alice, .asset = asset});
        env(vtx, THISLINE);
        env.close();

        env(vault.deposit({.depositor = alice, .id = vaultKeylet.key, .amount = asset(50)}),
            THISLINE);
        env.close();

        auto const brokerKeylet = keylet::loanbroker(alice.id(), env.seq(alice));
        env(set(alice, vaultKeylet.key), THISLINE);
        env.close();
        env(coverDeposit(alice, brokerKeylet.key, asset(10)), THISLINE);
        env.close();

        env(fclear(dest, asfDepositAuth), THISLINE);
        env(trust(issuer, asset(1'000), dest, tfSetFreeze | tfSetDeepFreeze), THISLINE);
        env.close();

        env(coverWithdraw(alice, brokerKeylet.key, asset(1)),
            destination(dest),
            ter(tecFROZEN),
            THISLINE);
    }

    void
    testF61_setTrustMissingIssuerSavOnControl()
    {
        testcase("F6.1 SetTrust missing issuer — tecNO_DST when SAV enabled (control)");
        using namespace jtx;

        FeatureBitset const all{
            jtx::testable_amendments() | featureMPTokensV1 | featureSingleAssetVault |
            featureLendingProtocol};

        Account const ghostIssuer{"ghostIssuer"};
        Account const alice{"alice"};
        Env env(*this, all);

        env.fund(XRP(10'000), alice);
        env.close();

        env(trust(alice, ghostIssuer["USD"](1'000)), ter(tecNO_DST));
    }

public:
    void
    run() override
    {
        testF33_coverWithdrawRegularFreezeOnly();
        testF33_controlDeepFreezeBlocks();
        testF61_setTrustMissingIssuerSavOnControl();
    }
};

BEAST_DEFINE_TESTSUITE(OpenP0Repro, tx, xrpl);

class OpenP0ReproCrash_test : public beast::unit_test::suite
{
    void
    testF61_setTrustMissingIssuerNoSavCrash()
    {
        testcase("F6.1 SetTrust missing issuer — preclaim null deref (expect crash if unfixed)");
        using namespace jtx;

        FeatureBitset const noAmmSav{
            (jtx::testable_amendments() | featureMPTokensV1 | featureLendingProtocol) -
            featureAMM - featureSingleAssetVault};

        Account const ghostIssuer{"ghostIssuer"};
        Account const alice{"alice"};
        Env env(*this, noAmmSav);

        env.fund(XRP(10'000), alice);
        env.close();

        env(trust(alice, ghostIssuer["USD"](1'000)), ter(tecNO_DST));
    }

public:
    void
    run() override
    {
        testF61_setTrustMissingIssuerNoSavCrash();
    }
};

BEAST_DEFINE_TESTSUITE(OpenP0ReproCrash, tx, xrpl);

}  // namespace test
}  // namespace xrpl
