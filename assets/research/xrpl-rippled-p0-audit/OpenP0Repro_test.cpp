#include <test/jtx.h>
#include <test/jtx/AMMTest.h>
#include <test/jtx/batch.h>
#include <test/jtx/check.h>
#include <test/jtx/credentials.h>
#include <test/jtx/did.h>
#include <test/jtx/directory.h>
#include <test/jtx/Env.h>
#include <test/jtx/escrow.h>
#include <test/jtx/domain.h>
#include <test/jtx/offer.h>
#include <test/jtx/mpt.h>
#include <test/jtx/permissioned_dex.h>
#include <test/jtx/permissioned_domains.h>
#include <test/jtx/ticket.h>
#include <test/jtx/trust.h>
#include <test/jtx/testline.h>
#include <test/jtx/vault.h>

#include <xrpld/app/misc/LendingHelpers.h>
#include <xrpld/app/tx/detail/InvariantCheck.h>

#include <xrpl/basics/Number.h>
#include <xrpl/ledger/ApplyViewImpl.h>
#include <xrpl/ledger/CredentialHelpers.h>
#include <xrpl/ledger/View.h>
#include <xrpl/beast/unit_test/suite.h>
#include <xrpl/protocol/Feature.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/MPTIssue.h>
#include <xrpl/protocol/Rate.h>
#include <xrpl/protocol/Rules.h>
#include <xrpl/protocol/STIssue.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/Serializer.h>
#include <xrpl/protocol/TxFlags.h>
#include <xrpl/protocol/jss.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <limits>

namespace ripple {
namespace test {

/** Definitive open-P0 repro (AGTI audit).
    Pass = proves unfixed behavior on this binary.
    After fixes: flip expectations to tecFROZEN and tests become regression guards.
*/
class OpenP0Repro_test : public beast::unit_test::suite
{
    [[nodiscard]] bool
    offerExists(
        jtx::Env const& env,
        jtx::Account const& account,
        std::uint32_t offerSeq)
    {
        return static_cast<bool>(env.le(keylet::offer(account.id(), offerSeq)));
    }

    struct BrokerFixture
    {
        jtx::PrettyAsset asset;
        uint256 vaultID;
        uint256 brokerID;
        jtx::Account vaultPseudo;
        jtx::Account brokerPseudo;
    };

    BrokerFixture
    createBrokerWithCover(
        jtx::Env& env,
        jtx::Account const& issuer,
        jtx::Account const& owner,
        std::int64_t cover = 10)
    {
        using namespace jtx;

        Vault vault{env};
        PrettyAsset const asset = issuer["IOU"];

        env(trust(owner, asset(1'000'000)), THISLINE);
        env.close();
        env(pay(issuer, owner, asset(100'000)), THISLINE);
        env.close();

        auto [vtx, vaultKeylet] = vault.create({.owner = owner, .asset = asset});
        env(vtx, THISLINE);
        env.close();

        env(vault.deposit({.depositor = owner, .id = vaultKeylet.key, .amount = asset(50'000)}),
            THISLINE);
        env.close();

        auto const brokerKeylet = keylet::loanbroker(owner.id(), env.seq(owner));
        env(loanBroker::set(owner, vaultKeylet.key), THISLINE);
        env.close();

        if (cover > 0)
        {
            env(loanBroker::coverDeposit(owner, brokerKeylet.key, asset(cover)),
                THISLINE);
            env.close();
        }

        auto const vaultSle = env.le(vaultKeylet);
        auto const brokerSle = env.le(brokerKeylet);
        if (!BEAST_EXPECT(vaultSle && brokerSle))
            return {
                asset,
                vaultKeylet.key,
                brokerKeylet.key,
                Account{"missingVaultPseudo"},
                Account{"missingBrokerPseudo"}};

        return {
            asset,
            vaultKeylet.key,
            brokerKeylet.key,
            Account{"vaultPseudo", vaultSle->at(sfAccount)},
            Account{"brokerPseudo", brokerSle->at(sfAccount)}};
    }

    std::pair<BrokerFixture, uint256>
    createLoanWithServiceFee(
        jtx::Env& env,
        jtx::Account const& issuer,
        jtx::Account const& broker,
        jtx::Account const& borrower,
        std::int64_t serviceFee = 100,
        std::int64_t originationFee = 0)
    {
        using namespace jtx;

        auto fixture = createBrokerWithCover(env, issuer, broker, 50'000);

        env(trust(borrower, fixture.asset(1'000'000)), THISLINE);
        env(pay(issuer, borrower, fixture.asset(50'000)), THISLINE);
        env.close();

        auto const brokerSle = env.le(keylet::loanbroker(fixture.brokerID));
        if (!BEAST_EXPECT(brokerSle))
            return {fixture, uint256{}};
        auto const loanSequence = brokerSle->at(sfLoanSequence);
        auto const loanKeylet = keylet::loan(fixture.brokerID, loanSequence);

        env(loan::set(borrower, fixture.brokerID, 10'000),
            sig(sfCounterpartySignature, broker),
            loan::loanOriginationFee(fixture.asset(originationFee).value()),
            loan::loanServiceFee(fixture.asset(serviceFee).value()),
            loan::paymentInterval(100),
            fee(XRP(100)),
            THISLINE);
        env.close();

        return {fixture, loanKeylet.key};
    }

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
    testF35_brokerDeleteRegularFrozenOwner()
    {
        testcase("F3.5 LoanBrokerDelete — regular-freeze-only owner receives cover");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker);
        env.close();

        auto const fixture = createBrokerWithCover(env, issuer, broker, 10);

        env(trust(issuer, fixture.asset(1'000), broker, tfSetFreeze), THISLINE);
        env.close();

        auto const before = env.balance(broker, fixture.asset);
        env(loanBroker::del(broker, fixture.brokerID), ter(tesSUCCESS), THISLINE);
        env.close();
        auto const after = env.balance(broker, fixture.asset);

        BEAST_EXPECT(after.number() - before.number() == fixture.asset(10).number());
    }

    void
    testF39_coverDepositRegularFrozenBrokerPseudo()
    {
        testcase("F3.9 LoanBrokerCoverDeposit — regular-freeze-only broker pseudo receives cover");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker);
        env.close();

        auto const fixture = createBrokerWithCover(env, issuer, broker, 5);

        env(trust(issuer, fixture.asset(1'000), fixture.brokerPseudo, tfSetFreeze),
            THISLINE);
        env.close();

        auto const before = env.balance(fixture.brokerPseudo, fixture.asset);
        env(loanBroker::coverDeposit(broker, fixture.brokerID, fixture.asset(10)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const after = env.balance(fixture.brokerPseudo, fixture.asset);

        BEAST_EXPECT(after.number() - before.number() == fixture.asset(10).number());
    }

    void
    testF37_loanSetRegularFrozenBrokerOwnerOriginationFee()
    {
        testcase("F3.7 LoanSet — regular-freeze-only broker owner receives origination fee");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Account const borrower{"borrower"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker, borrower);
        env.close();

        auto const fixture = createBrokerWithCover(env, issuer, broker, 50'000);
        env(trust(borrower, fixture.asset(1'000'000)), THISLINE);
        env.close();

        env(trust(issuer, fixture.asset(1'000), broker, tfSetFreeze), THISLINE);
        env.close();

        auto const before = env.balance(broker, fixture.asset);
        env(loan::set(borrower, fixture.brokerID, 10'000),
            sig(sfCounterpartySignature, broker),
            loan::loanOriginationFee(fixture.asset(100).value()),
            loan::paymentInterval(100),
            fee(XRP(100)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const after = env.balance(broker, fixture.asset);

        BEAST_EXPECT(after.number() - before.number() == fixture.asset(100).number());
    }

    void
    testF36_loanPayRegularFrozenBrokerOwnerServiceFee()
    {
        testcase("F3.6 LoanPay — regular-freeze-only broker owner receives service fee");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Account const borrower{"borrower"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker, borrower);
        env.close();

        auto const [fixture, loanID] =
            createLoanWithServiceFee(env, issuer, broker, borrower, 100, 0);

        env(trust(issuer, fixture.asset(1'000), broker, tfSetFreeze), THISLINE);
        env.close();

        auto const before = env.balance(broker, fixture.asset);
        env(loan::pay(borrower, loanID, fixture.asset(10'100)),
            fee(XRP(100)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const after = env.balance(broker, fixture.asset);

        BEAST_EXPECT(after.number() - before.number() == fixture.asset(100).number());
    }

    void
    testF38_loanPayRegularFrozenVaultPseudo()
    {
        testcase("F3.8 LoanPay — regular-freeze-only vault pseudo receives repayment");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Account const borrower{"borrower"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker, borrower);
        env.close();

        auto const [fixture, loanID] =
            createLoanWithServiceFee(env, issuer, broker, borrower, 100, 0);

        env(trust(issuer, fixture.asset(1'000), fixture.vaultPseudo, tfSetFreeze),
            THISLINE);
        env.close();

        auto const before = env.balance(fixture.vaultPseudo, fixture.asset);
        env(loan::pay(borrower, loanID, fixture.asset(10'100)),
            fee(XRP(100)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const after = env.balance(fixture.vaultPseudo, fixture.asset);

        BEAST_EXPECT(after.number() > before.number());
    }

    void
    testF310_loanPayRegularFrozenBrokerPseudoFallbackFee()
    {
        testcase("F3.10 LoanPay — regular-freeze-only broker pseudo receives fallback fee");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const broker{"broker"};
        Account const borrower{"borrower"};
        Env env(*this);
        env.fund(XRP(100'000), issuer, broker, borrower);
        env.close();

        auto const [fixture, loanID] =
            createLoanWithServiceFee(env, issuer, broker, borrower, 100, 0);

        auto const brokerTrustline =
            keylet::line(broker.id(), fixture.asset.raw().get<Issue>());
        BEAST_EXPECT(env.le(brokerTrustline) != nullptr);
        auto const brokerBalance = env.balance(broker, fixture.asset);
        if (brokerBalance.number() > beast::zero)
        {
            env(pay(broker, issuer, brokerBalance), THISLINE);
            env.close();
        }
        env(trust(broker, fixture.asset(0)), THISLINE);
        env.close();
        BEAST_EXPECT(env.le(brokerTrustline) == nullptr);

        env(trust(issuer, fixture.asset(1'000), fixture.brokerPseudo, tfSetFreeze),
            THISLINE);
        env.close();

        auto const before = env.balance(fixture.brokerPseudo, fixture.asset);
        env(loan::pay(borrower, loanID, fixture.asset(10'100)),
            fee(XRP(100)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        auto const after = env.balance(fixture.brokerPseudo, fixture.asset);

        BEAST_EXPECT(after.number() - before.number() == fixture.asset(100).number());
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

    void
    testDIDDirFullNoPartialMutation()
    {
        testcase("DIDSet tecDIR_FULL does not leave partial DID object");
        using namespace jtx;

        Account const alice{"alice"};
        Env env(*this, testable_amendments() - fixDirectoryLimit);

        env.fund(XRP(100'000), alice);
        env.close();

        env(ticket::create(alice, 63), THISLINE);
        env.close();

        auto const res = directory::bumpLastPage(
            env,
            directory::maximumPageIndex(env),
            keylet::ownerDir(alice.id()),
            directory::adjustOwnerNode);
        BEAST_EXPECT(res);

        env(ticket::create(alice, 1), THISLINE);

        auto const didKey = keylet::did(alice.id());
        BEAST_EXPECT(!env.le(didKey));

        env(did::setValid(alice), ter(tecDIR_FULL), THISLINE);
        BEAST_EXPECT(!env.le(didKey));
    }

    void
    testBatchAllOrNothingExpiredCredentialRollback()
    {
        testcase("Batch all-or-nothing rolls back expired CredentialAccept deletion");
        using namespace jtx;

        char const credType[] = "batch-expired-credential";
        Account const issuer{"issuer"};
        Account const subject{"subject"};
        Env env(*this);

        env.fund(XRP(100'000), issuer, subject);
        env.close();

        auto jv = credentials::create(subject, issuer, credType);
        uint32_t const t = env.current()
                               ->info()
                               .parentCloseTime.time_since_epoch()
                               .count();
        jv[sfExpiration.jsonName] = t;
        env(jv, THISLINE);
        env.close();

        auto const credentialKey = credentials::keylet(subject, issuer, credType);
        BEAST_EXPECT(env.le(credentialKey));

        auto const seq = env.seq(subject);
        auto const beforeIssuer = env.balance(issuer);
        auto const beforeSubject = env.balance(subject);
        auto const batchFee = batch::calcBatchFee(env, 0, 2);

        env(batch::outer(subject, seq, batchFee, tfAllOrNothing),
            batch::inner(credentials::accept(subject, issuer, credType), seq + 1),
            batch::inner(pay(subject, issuer, XRP(1)), seq + 2),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        BEAST_EXPECT(env.le(credentialKey));
        BEAST_EXPECT(env.balance(issuer) == beforeIssuer);
        BEAST_EXPECT(env.balance(subject) == beforeSubject - batchFee);
    }

    void
    testPermissionedDomainTicketSequencePrefixCollision()
    {
        testcase("PermissionedDomainSet pre-fix ticket sequence collision candidate");
        using namespace jtx;

        FeatureBitset const preFix{
            (testable_amendments() | featurePermissionedDomains |
             featureCredentials) -
            fixCleanup3_1_3};

        Account const alice{"alice"};
        Env env(*this, preFix);

        env.fund(XRP(100'000), alice);
        env.close();

        auto const firstTicket = env.seq(alice) + 1;
        env(ticket::create(alice, 2), THISLINE);
        env.close();

        pdomain::Credentials credentials{{alice, "first credential"}};
        auto const zeroKey = keylet::permissionedDomain(alice.id(), 0);
        BEAST_EXPECT(!env.le(zeroKey));

        env(pdomain::setTx(alice, credentials),
            ticket::use(firstTicket),
            THISLINE);
        env.close();
        BEAST_EXPECT(env.le(zeroKey));

        auto const ownerCountAfterFirst = ownerCount(env, alice);

        env(pdomain::setTx(alice, credentials),
            ticket::use(firstTicket + 1),
            ter(tefEXCEPTION),
            THISLINE);
        env.close();

        BEAST_EXPECT(env.le(zeroKey));
        BEAST_EXPECT(ownerCount(env, alice) == ownerCountAfterFirst);

        FeatureBitset const withFix{
            testable_amendments() | featurePermissionedDomains |
            featureCredentials | fixCleanup3_1_3};

        Account const bob{"bob"};
        Env fixedEnv(*this, withFix);

        fixedEnv.fund(XRP(100'000), bob);
        fixedEnv.close();

        auto const bobFirstTicket = fixedEnv.seq(bob) + 1;
        fixedEnv(ticket::create(bob, 2), THISLINE);
        fixedEnv.close();

        pdomain::Credentials fixedCredentials{{bob, "first credential"}};
        fixedEnv(pdomain::setTx(bob, fixedCredentials),
            ticket::use(bobFirstTicket),
            THISLINE);
        fixedEnv.close();
        fixedEnv(pdomain::setTx(bob, fixedCredentials),
            ticket::use(bobFirstTicket + 1),
            THISLINE);
        fixedEnv.close();

        BEAST_EXPECT(!fixedEnv.le(keylet::permissionedDomain(bob.id(), 0)));
        BEAST_EXPECT(fixedEnv.le(keylet::permissionedDomain(bob.id(), bobFirstTicket)));
        BEAST_EXPECT(fixedEnv.le(keylet::permissionedDomain(bob.id(), bobFirstTicket + 1)));
    }

    void
    testExpiredCredentialDeleteFailurePreFix()
    {
        testcase("Credentials pre-fix — expired cleanup ignores deleteSLE failure");
        using namespace jtx;

        auto const run = [&](FeatureBitset features,
                             TER expectedDeposit,
                             TER expectedDomain) {
            char const credType[] = "abcde";
            Account const issuer{"issuer"};
            Account const subject{"subject"};
            Account const becky{"becky"};

            Env env{*this, features};
            env.fund(XRP(10000), issuer, subject, becky);
            env.close();

            auto jv = credentials::create(subject, issuer, credType);
            std::uint32_t const expiration =
                env.current()->info().parentCloseTime.time_since_epoch().count() +
                40;
            jv[sfExpiration.jsonName] = expiration;
            env(jv, THISLINE);
            env.close();

            auto const credLE =
                credentials::ledgerEntry(env, subject, issuer, credType);
            std::string const credIdx =
                credLE[jss::result][jss::index].asString();

            env(credentials::accept(subject, issuer, credType), THISLINE);
            env.close();

            auto const credKeylet = keylet::credential(
                subject.id(),
                issuer.id(),
                Slice(credType, sizeof(credType) - 1));
            BEAST_EXPECT(env.current()->exists(credKeylet));

            env(deposit::authCredentials(becky, {{subject, credType}}),
                THISLINE);
            env.close();
            auto jtx =
                env.jt(pay(subject, becky, XRP(100)), credentials::ids({credIdx}));
            if (!BEAST_EXPECT(jtx.stx))
                return;
            auto const stx = std::make_shared<STTx>(*jtx.stx);

            env(pdomain::setTx(becky, {{issuer, credType}}), THISLINE);
            env.close();
            auto const domain = pdomain::getObjects(becky, env).begin()->first;

            using namespace std::chrono_literals;
            env.close(50s);
            auto const sleCred = env.current()->read(credKeylet);
            BEAST_EXPECT(
                sleCred &&
                ripple::credentials::checkExpired(
                    *sleCred, env.current()->info().parentCloseTime));

            ApplyViewImpl av(&*env.current(), tapNONE);
            auto sleIssuer = av.peek(keylet::account(issuer.id()));
            if (!BEAST_EXPECT(sleIssuer))
                return;
            av.erase(sleIssuer);
            BEAST_EXPECT(!av.exists(keylet::account(issuer.id())));
            BEAST_EXPECT(av.exists(credKeylet));

            beast::Journal const j{beast::Journal::getNullSink()};
            auto const dpTer =
                verifyDepositPreauth(*stx, av, subject, becky, {}, j);
            auto const domTer = verifyValidDomain(av, subject.id(), domain, j);

            BEAST_EXPECT(dpTer == expectedDeposit);
            BEAST_EXPECT(domTer == expectedDomain);
            BEAST_EXPECT(av.exists(credKeylet));
        };

        FeatureBitset const base =
            testable_amendments() | featureCredentials |
            featurePermissionedDomains | featureDepositPreauth;

        run(base - fixCleanup3_1_3, tecEXPIRED, tesSUCCESS);
        run(base | fixCleanup3_1_3, tecINTERNAL, tecINTERNAL);
    }

    void
    testPermissionedDexEmptyAdditionalBooksPreFix()
    {
        testcase("Permissioned DEX pre-fix — empty AdditionalBooks hides malformed hybrid offer");
        using namespace jtx;

        auto const run = [&](FeatureBitset features) {
            Env env{*this, features};
            Account const a1{"A1"};
            Account const a2{"A2"};

            env.fund(XRP(1000), a1, a2);
            env.close();
            pdomain::Credentials const credentials{{a2, "cred_type0"}};
            env(pdomain::setTx(a1, credentials), THISLINE);
            uint256 const domain = pdomain::getNewDomain(env.meta());
            env.close();

            auto offer = std::make_shared<SLE>(keylet::offer(a2.id(), 10));
            offer->setAccountID(sfAccount, a2);
            offer->setFieldAmount(sfTakerPays, a1["USD"](10));
            offer->setFieldAmount(sfTakerGets, XRP(1));
            offer->setFlag(lsfHybrid);
            offer->setFieldH256(sfDomainID, domain);
            offer->setFieldArray(sfAdditionalBooks, STArray{sfAdditionalBooks});

            ValidPermissionedDEX invariant;
            invariant.visitEntry(false, {}, offer);

            return invariant.finalize(
                STTx{ttOFFER_CREATE,
                     [&](STObject& tx) { tx.setFieldH256(sfDomainID, domain); }},
                tesSUCCESS,
                XRPAmount{},
                *env.current(),
                env.app().journal("OpenP0Repro"));
        };

        FeatureBitset const base =
            testable_amendments() | featurePermissionedDomains |
            featureCredentials;

        BEAST_EXPECT(run(base - fixCleanup3_1_3));
        BEAST_EXPECT(!run(base | fixCleanup3_1_3));
    }

    void
    testMPTMultiSendMaximumAmountPreFix()
    {
        testcase("MPT multi-send pre-fix aggregate MaximumAmount bypass");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        std::uint64_t constexpr maxAmt = 150;

        auto run = [&](FeatureBitset features, TER expectedTer) {
            Env env{*this, features};

            MPTTester mpt{env, issuer, {.holders = {alice, bob}}};
            mpt.create(
                {.maxAmt = maxAmt, .ownerCount = 1, .flags = tfMPTCanTransfer});
            mpt.authorize({.account = alice});
            mpt.authorize({.account = bob});

            Asset const asset{MPTIssue{mpt.issuanceID()}};
            ApplyViewImpl av(&*env.current(), tapNONE);
            auto const ter = accountSendMulti(
                av,
                issuer.id(),
                asset,
                MultiplePaymentDestinations{{alice.id(), 100}, {bob.id(), 100}},
                env.app().journal("View"));

            BEAST_EXPECT(ter == expectedTer);
            if (ter == tesSUCCESS)
            {
                auto const issuance =
                    av.peek(keylet::mptIssuance(mpt.issuanceID()));
                BEAST_EXPECT(issuance);
                if (issuance)
                    BEAST_EXPECT(issuance->getFieldU64(sfOutstandingAmount) == 200);
            }
        };

        run(testable_amendments() - fixCleanup3_1_3, tesSUCCESS);
        run(testable_amendments() | fixCleanup3_1_3, tecPATH_DRY);
    }

    void
    testVaultWithdrawShareLimitBypassPreFix()
    {
        testcase("VaultWithdraw pre-fix share-denominated trustline limit bypass");
        using namespace jtx;

        auto run = [&](FeatureBitset features) {
            bool const withFix = features[fixCleanup3_1_3];

            Account const owner{"owner"};
            Account const issuer{"issuer"};
            Account const depositor{"depositor"};
            Account const charlie{"charlie"};
            Env env{*this, features};
            Vault vault{env};

            env.fund(XRP(1000), issuer, owner, depositor, charlie);
            env(fset(issuer, asfAllowTrustLineClawback));
            env.close();

            PrettyAsset const asset = issuer["IOU"];
            env.trust(asset(1000), owner);
            env.trust(asset(1000), depositor);
            env(pay(issuer, owner, asset(200)));
            env(pay(issuer, depositor, asset(200)));
            env.close();

            env.trust(asset(5), charlie);
            env.close();

            auto const [tx, keylet] = vault.create({.owner = owner, .asset = asset});
            env(tx);
            env.close();

            env(vault.deposit(
                    {.depositor = depositor,
                     .id = keylet.key,
                     .amount = asset(100)}));
            env.close();

            auto const vaultSle = env.le(keylet);
            if (!BEAST_EXPECT(vaultSle))
                return;
            PrettyAsset const share = MPTIssue(vaultSle->at(sfShareMPTID));

            auto assetWithdraw = vault.withdraw(
                {.depositor = depositor, .id = keylet.key, .amount = asset(10)});
            assetWithdraw[sfDestination] = charlie.human();
            env(assetWithdraw, ter(tecNO_LINE));
            env.close();

            auto const before = env.balance(charlie, asset.raw().get<Issue>());
            auto shareWithdraw = vault.withdraw(
                {.depositor = depositor,
                 .id = keylet.key,
                 .amount = STAmount(share, 10'000'000)});
            shareWithdraw[sfDestination] = charlie.human();
            env(shareWithdraw, ter(withFix ? TER{tecNO_LINE} : TER{tesSUCCESS}));
            env.close();

            auto const after = env.balance(charlie, asset.raw().get<Issue>());
            if (withFix)
                BEAST_EXPECT(after == before);
            else
                BEAST_EXPECT(after > before);
        };

        run(testable_amendments() | featureSingleAssetVault | fixCleanup3_1_3);
        run((testable_amendments() | featureSingleAssetVault) - fixCleanup3_1_3);
    }

    void
    testVaultShareMPTLockedEscrowDeletionPreFix()
    {
        testcase("VaultWithdraw pre-fix deletes share MPToken with locked escrow");
        using namespace jtx;
        using namespace std::literals;

        auto run = [&](FeatureBitset features) {
            Account const issuer{"issuer"};
            Account const owner{"owner"};
            Account const depositor{"depositor"};
            Account const bob{"bob"};
            Env env{*this, features};
            auto const baseFee = env.current()->fees().base;
            Vault vault{env};

            env.fund(XRP(100000), issuer, owner, depositor, bob);
            env.close();

            MPTTester mpt{env, issuer, mptInitNoFund};
            mpt.create({.flags = tfMPTCanTransfer | tfMPTCanLock});
            PrettyAsset const asset = mpt.issuanceID();
            mpt.authorize({.account = owner});
            mpt.authorize({.account = depositor});
            env(pay(issuer, depositor, asset(1000)));
            env.close();

            auto const [tx, keylet] = vault.create({.owner = owner, .asset = asset});
            env(tx);
            env.close();

            env(vault.deposit(
                {.depositor = depositor, .id = keylet.key, .amount = asset(1000)}));
            env.close();

            auto const vaultSle = env.le(keylet);
            if (!BEAST_EXPECT(vaultSle))
                return;
            auto const shareID = vaultSle->at(sfShareMPTID);
            MPTIssue const shareIssue{shareID};

            env(escrow::create(depositor, bob, STAmount{shareIssue, 500}),
                escrow::condition(escrow::cb1),
                escrow::finish_time(env.now() + 1s),
                fee(baseFee * 150),
                ter(tesSUCCESS));
            env.close();

            auto const tokenBefore = env.le(keylet::mptoken(shareID, depositor));
            BEAST_EXPECT(tokenBefore);
            if (!tokenBefore)
                return;
            BEAST_EXPECT(tokenBefore->at(sfLockedAmount) == 500);
            BEAST_EXPECT(tokenBefore->at(sfMPTAmount) == 500);

            env(vault.withdraw(
                    {.depositor = depositor,
                     .id = keylet.key,
                     .amount = asset(500)}),
                ter(tesSUCCESS));
            env.close();

            auto const tokenAfter = env.le(keylet::mptoken(shareID, depositor));
            if (features[fixCleanup3_1_3])
            {
                BEAST_EXPECT(tokenAfter);
                if (tokenAfter)
                {
                    BEAST_EXPECT(tokenAfter->at(sfLockedAmount) == 500);
                    BEAST_EXPECT(tokenAfter->at(sfMPTAmount) == 0);
                }
            }
            else
            {
                BEAST_EXPECT(!tokenAfter);
            }
        };

        run(testable_amendments() - fixCleanup3_1_3);
        run(testable_amendments() | fixCleanup3_1_3);
    }

    void
    testVaultClawbackZeroAmountUnclampedPreFix()
    {
        testcase(
            "VaultClawback pre-fix zero-amount asset clawback is not "
            "clamped to assets available");
        using namespace jtx;

        auto run = [&](FeatureBitset features) {
            bool const withFix = features[fixCleanup3_1_3];

            Account const issuer{"issuer"};
            Account const owner{"owner"};
            Account const depositor{"depositor"};
            Env env{*this, features};
            Vault vault{env};

            env.fund(XRP(10'000), issuer, owner, depositor);
            env(fset(issuer, asfAllowTrustLineClawback));
            env.close();

            PrettyAsset const asset = issuer["IOU"];
            env.trust(asset(2'000), owner);
            env.trust(asset(2'000), depositor);
            env(pay(issuer, owner, asset(2'000)));
            env(pay(issuer, depositor, asset(2'000)));
            env.close();

            auto const [tx, vaultKeylet] =
                vault.create({.owner = owner, .asset = asset});
            env(tx, ter(tesSUCCESS), THISLINE);
            env.close();

            env(vault.deposit(
                    {.depositor = depositor,
                     .id = vaultKeylet.key,
                     .amount = asset(100)}),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            auto const vaultSle = env.le(vaultKeylet);
            if (!BEAST_EXPECT(vaultSle))
                return;
            env.memoize(Account{"vault", vaultSle->at(sfAccount)});
            PrettyAsset const shares = MPTIssue(vaultSle->at(sfShareMPTID));

            auto const brokerKeylet =
                keylet::loanbroker(owner.id(), env.seq(owner));
            env(loanBroker::set(owner, vaultKeylet.key), THISLINE);
            env.close();

            env(loan::set(depositor, brokerKeylet.key, asset(40).value()),
                loan::interestRate(TenthBips32(0)),
                loan::gracePeriod(60),
                loan::paymentInterval(120),
                loan::paymentTotal(10),
                sig(sfCounterpartySignature, owner),
                fee(env.current()->fees().base * 2),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            {
                auto const sle = env.le(vaultKeylet);
                if (!BEAST_EXPECT(sle))
                    return;
                BEAST_EXPECT(sle->at(sfAssetsAvailable) == asset(60).value());
                BEAST_EXPECT(sle->at(sfAssetsTotal) == asset(100).value());
            }

            auto const sharesBefore = env.balance(depositor, shares);
            env(vault.clawback({
                    .issuer = issuer,
                    .id = vaultKeylet.key,
                    .holder = depositor,
                }),
                ter(withFix ? TER{tesSUCCESS} : TER{tefINTERNAL}),
                THISLINE);
            env.close();

            auto const sle = env.le(vaultKeylet);
            if (!BEAST_EXPECT(sle))
                return;
            if (withFix)
            {
                BEAST_EXPECT(sle->at(sfAssetsAvailable) == asset(0).value());
                BEAST_EXPECT(sle->at(sfAssetsTotal) == asset(40).value());
                BEAST_EXPECT(env.balance(depositor, shares) < sharesBefore);
            }
            else
            {
                BEAST_EXPECT(sle->at(sfAssetsAvailable) == asset(60).value());
                BEAST_EXPECT(sle->at(sfAssetsTotal) == asset(100).value());
                BEAST_EXPECT(env.balance(depositor, shares) == sharesBefore);
            }
        };

        run((testable_amendments() | featureSingleAssetVault |
             featureLendingProtocol) -
            fixCleanup3_1_3);
        run(testable_amendments() | featureSingleAssetVault |
            featureLendingProtocol | fixCleanup3_1_3);
    }

    void
    testLoanPayFeeCapPreFix()
    {
        testcase("LoanPay pre-fix high-amount payment requires uncapped fee");
        using namespace jtx;

        auto run = [&](FeatureBitset features) {
            bool const withFix = features[fixCleanup3_1_3];

            Account const issuer{"issuer"};
            Account const lender{"lender"};
            Account const borrower{"borrower"};
            Env env{*this, features};

            env.fund(XRP(1'000'000), issuer, lender, borrower);
            env.close();

            auto const fixture = createBrokerWithCover(env, issuer, lender, 50'000);
            env(trust(borrower, fixture.asset(100'000'000)), THISLINE);
            env(pay(issuer, borrower, fixture.asset(100'000)), THISLINE);
            env.close();

            auto const brokerSleBefore =
                env.le(keylet::loanbroker(fixture.brokerID));
            if (!BEAST_EXPECT(brokerSleBefore))
                return;
            auto const loanSequence = brokerSleBefore->at(sfLoanSequence);
            auto const loanKeylet =
                keylet::loan(fixture.brokerID, loanSequence);

            env(loan::set(borrower, fixture.brokerID, Number{3959'37, -2}),
                sig(sfCounterpartySignature, lender),
                loan::closePaymentFee(0),
                loan::gracePeriod(60),
                loan::interestRate(TenthBips32(20930)),
                loan::lateInterestRate(TenthBips32(77049)),
                loan::latePaymentFee(0),
                loan::loanServiceFee(0),
                loan::overpaymentFee(TenthBips32(7)),
                loan::overpaymentInterestRate(TenthBips32(66653)),
                loan::paymentInterval(60),
                loan::paymentTotal(3'239'184),
                fee(env.current()->fees().base * 2),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            auto const loanSleBefore = env.le(loanKeylet);
            if (!BEAST_EXPECT(loanSleBefore))
                return;
            BEAST_EXPECT(loanSleBefore->at(sfPaymentRemaining) == 3'239'184);

            auto const roundedPayment = roundPeriodicPayment(
                fixture.asset,
                loanSleBefore->at(sfPeriodicPayment),
                loanSleBefore->at(sfLoanScale));

            std::int64_t constexpr maxFeeIncrements =
                Lending::loanMaximumPaymentsPerTransaction /
                Lending::loanPaymentsPerFeeIncrement;
            auto const payAmount = roundedPayment * 1'819'878;
            auto loanPayTx = env.json(loan::pay(
                borrower,
                loanKeylet.key,
                STAmount{fixture.asset, payAmount}));

            env(loanPayTx,
                fee(env.current()->fees().base * maxFeeIncrements),
                ter(withFix ? TER{tesSUCCESS} : TER{telINSUF_FEE_P}),
                THISLINE);
            env.close();

            auto const loanSleAfter = env.le(loanKeylet);
            if (!BEAST_EXPECT(loanSleAfter))
                return;
            BEAST_EXPECT(
                loanSleAfter->at(sfPaymentRemaining) ==
                (withFix ? 3'239'084 : 3'239'184));
        };

        run((testable_amendments() | featureSingleAssetVault |
             featureLendingProtocol) -
            fixCleanup3_1_3);
        run(testable_amendments() | featureSingleAssetVault |
            featureLendingProtocol | fixCleanup3_1_3);
    }

    void
    testVaultShareMPTUnderlyingTransferRestrictionCurrent()
    {
        testcase("Vault share MPT current — underlying CanTransfer is not inherited");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const owner{"owner"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        Env env{*this, testable_amendments() | featureMPTokensV1 |
                            featureSingleAssetVault};
        Vault vault{env};

        env.fund(XRP(10'000), issuer, owner, alice, bob);
        env.close();

        MPTTester mptt{env, issuer, mptInitNoFund};
        mptt.create(
            {.flags = tfMPTCanTransfer | tfMPTCanLock,
             .mutableFlags = tmfMPTCanMutateCanTransfer});
        PrettyAsset const asset = mptt.issuanceID();
        mptt.authorize({.account = owner});
        mptt.authorize({.account = alice});
        mptt.authorize({.account = bob});
        env(pay(issuer, alice, asset(1'000)), THISLINE);
        env(pay(issuer, bob, asset(1'000)), THISLINE);
        env.close();

        auto [tx, keylet] = vault.create({.owner = owner, .asset = asset});
        env(tx, THISLINE);
        env.close();

        env(vault.deposit(
                {.depositor = alice, .id = keylet.key, .amount = asset(500)}),
            THISLINE);
        env(vault.deposit(
                {.depositor = bob, .id = keylet.key, .amount = asset(500)}),
            THISLINE);
        env.close();

        auto const vaultSle = env.le(keylet);
        if (!BEAST_EXPECT(vaultSle))
            return;
        env.memoize(Account{"vault", vaultSle->at(sfAccount)});
        PrettyAsset const shares = MPTIssue(vaultSle->at(sfShareMPTID));

        env(pay(alice, bob, shares(1)), ter(tesSUCCESS), THISLINE);
        env.close();

        mptt.set({.mutableFlags = tmfMPTClearCanTransfer});
        env.close();

        auto const aliceSharesBefore = env.balance(alice, shares);
        auto const bobSharesBefore = env.balance(bob, shares);

        env(pay(alice, bob, shares(1)), ter(tesSUCCESS), THISLINE);
        env.close();

        // Current 3.1.3 lets vault shares move peer-to-peer after the
        // underlying MPT issuer has disabled transfer. Upstream PR #7077
        // changes share creation/reference metadata so this path returns
        // tecNO_AUTH instead.
        BEAST_EXPECT(env.balance(alice, shares) < aliceSharesBefore);
        BEAST_EXPECT(env.balance(bob, shares) > bobSharesBefore);
    }

    void
    testLoanBrokerCoverPrecisionCurrent()
    {
        testcase("LoanBrokerCover current — IOU precision drift");
        using namespace jtx;
        using namespace loanBroker;

        Account const issuer{"issuer"};
        Account const alice{"alice"};
        Env env{*this};

        env.fund(XRP(100'000), issuer, alice);
        env.close();

        auto const fixture = createBrokerWithCover(env, issuer, alice, 10);
        auto const brokerKeylet = keylet::loanbroker(fixture.brokerID);

        {
            auto const brokerBefore = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerBefore))
                return;
            Number const coverBefore = brokerBefore->at(sfCoverAvailable);

            env(coverDeposit(
                    alice,
                    fixture.brokerID,
                    fixture.asset(Number{18, -15})),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            auto const brokerAfter = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerAfter))
                return;

            // Current 3.1.3 credits cover with default round-to-nearest:
            // request 1.8e-14, broker cover increases by 2e-14.
            Number const coverDelta =
                brokerAfter->at(sfCoverAvailable) - coverBefore;
            Number const expectedDelta{2, -14};
            BEAST_EXPECT(coverDelta == expectedDelta);
        }

        {
            auto const brokerBefore = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerBefore))
                return;
            Number const coverBefore = brokerBefore->at(sfCoverAvailable);
            auto const aliceBalanceBefore = env.balance(alice, fixture.asset);

            env(coverDeposit(
                    alice,
                    fixture.brokerID,
                    fixture.asset(Number{1, -16})),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            auto const brokerAfter = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerAfter))
                return;

            // Current 3.1.3 silently accepts a positive deposit that rounds to
            // zero at the broker cover scale.
            BEAST_EXPECT(brokerAfter->at(sfCoverAvailable) == coverBefore);
            BEAST_EXPECT(env.balance(alice, fixture.asset) == aliceBalanceBefore);
        }

        {
            auto const brokerBefore = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerBefore))
                return;
            Number const coverBefore = brokerBefore->at(sfCoverAvailable);
            auto const aliceBalanceBefore = env.balance(alice, fixture.asset);

            env(coverWithdraw(
                    alice,
                    fixture.brokerID,
                    fixture.asset(Number{1, -16})),
                ter(tesSUCCESS),
                THISLINE);
            env.close();

            auto const brokerAfter = env.le(brokerKeylet);
            if (!BEAST_EXPECT(brokerAfter))
                return;

            // Later fixCleanup3_2_0 code rejects this as tecPRECISION_LOSS.
            // Current 3.1.3 accepts the withdrawal, but it rounds to zero at
            // the broker-cover scale and moves no funds.
            BEAST_EXPECT(brokerAfter->at(sfCoverAvailable) == coverBefore);
            BEAST_EXPECT(env.balance(alice, fixture.asset) == aliceBalanceBefore);
        }

        {
            Account const clawIssuer{"clawIssuer"};
            Account const clawOwner{"clawOwner"};
            Env clawEnv{*this};

            clawEnv.fund(XRP(100'000), clawIssuer, clawOwner);
            clawEnv(fset(clawIssuer, asfAllowTrustLineClawback), THISLINE);
            clawEnv.close();

            auto const clawFixture =
                createBrokerWithCover(clawEnv, clawIssuer, clawOwner, 10);
            auto const clawBrokerKeylet = keylet::loanbroker(clawFixture.brokerID);

            auto const brokerBefore = clawEnv.le(clawBrokerKeylet);
            if (!BEAST_EXPECT(brokerBefore))
                return;
            Number const coverBefore = brokerBefore->at(sfCoverAvailable);
            auto const issuerBalanceBefore =
                clawEnv.balance(clawIssuer, clawFixture.asset);

            clawEnv(coverClawback(clawIssuer),
                loanBrokerID(clawFixture.brokerID),
                amount(clawFixture.asset(Number{1, -16})),
                ter(tesSUCCESS),
                THISLINE);
            clawEnv.close();

            auto const brokerAfter = clawEnv.le(clawBrokerKeylet);
            if (!BEAST_EXPECT(brokerAfter))
                return;

            // Current 3.1.3 also accepts a positive clawback amount that is
            // zero at cover scale, so neither cover nor issuer balance moves.
            BEAST_EXPECT(brokerAfter->at(sfCoverAvailable) == coverBefore);
            BEAST_EXPECT(clawEnv.balance(clawIssuer, clawFixture.asset) == issuerBalanceBefore);
        }
    }

    void
    testLoanMinimumCoverScaleInconsistencyCurrent()
    {
        testcase("LoanPay current — broker minimum cover scale inconsistency");
        using namespace jtx;
        using namespace loan;
        using namespace loanBroker;

        auto run = [&](bool payBigLoan, bool expectPseudoFee) {
            Account const issuer{"issuer"};
            Account const lender{"lender"};
            Account const borrower{"borrower"};
            Env env{*this};

            env.fund(XRP(1'000'000'000), issuer, lender, borrower);
            env.close();

            env(fset(issuer, asfAllowTrustLineClawback), THISLINE);
            env.close();

            PrettyAsset const iou = issuer["IOU"];
            env(trust(lender, iou(1'000'000'000)), THISLINE);
            env(trust(borrower, iou(1'000'000'000)), THISLINE);
            env.close();
            env(pay(issuer, lender, iou(100'000'000)), THISLINE);
            env(pay(issuer, borrower, iou(100'000'000)), THISLINE);
            env.close();

            Vault vault{env};
            auto [vtx, vaultKeylet] = vault.create({.owner = lender, .asset = iou});
            env(vtx, THISLINE);
            env.close();
            env(vault.deposit({.depositor = lender, .id = vaultKeylet.key, .amount = iou(1'000)}),
                THISLINE);
            env.close();

            auto const brokerKeylet = keylet::loanbroker(lender.id(), env.seq(lender));
            env(loanBroker::set(lender, vaultKeylet.key),
                loanBroker::managementFeeRate(TenthBips16{500}),
                loanBroker::debtMaximum(Number{0}),
                loanBroker::coverRateMinimum(TenthBips32{13'370}),
                loanBroker::coverRateLiquidation(TenthBips32{25'000}),
                THISLINE);
            env.close();
            env(loanBroker::coverDeposit(lender, brokerKeylet.key, iou(5'000)), THISLINE);
            env.close();

            auto const tinySeq = [&]() {
                auto const brokerSle = env.le(keylet::loanbroker(brokerKeylet.key));
                if (!BEAST_EXPECT(brokerSle))
                    return std::uint32_t{0};
                return brokerSle->at(sfLoanSequence);
            }();
            auto const tinyLoanKeylet = keylet::loan(brokerKeylet.key, tinySeq);
            env(loan::set(borrower, brokerKeylet.key, Number{1, -2}),
                sig(sfCounterpartySignature, lender),
                loan::interestRate(TenthBips32{0}),
                loan::paymentTotal(1),
                loan::paymentInterval(86400 * 365),
                fee(XRP(10)),
                THISLINE);
            env.close();

            auto const bigSeq = [&]() {
                auto const brokerSle = env.le(keylet::loanbroker(brokerKeylet.key));
                if (!BEAST_EXPECT(brokerSle))
                    return std::uint32_t{0};
                return brokerSle->at(sfLoanSequence);
            }();
            auto const bigLoanKeylet = keylet::loan(brokerKeylet.key, bigSeq);
            env(loan::set(borrower, brokerKeylet.key, Number{500}),
                sig(sfCounterpartySignature, lender),
                loan::interestRate(TenthBips32{100'000}),
                loan::paymentTotal(20),
                loan::paymentInterval(86400 * 365),
                fee(XRP(10)),
                THISLINE);
            env.close();

            auto const tinyLoanSle = env.le(tinyLoanKeylet);
            auto const bigLoanSle = env.le(bigLoanKeylet);
            auto const vaultSle = env.le(keylet::vault(vaultKeylet.key));
            if (!BEAST_EXPECT(tinyLoanSle && bigLoanSle && vaultSle))
                return;
            BEAST_EXPECT(tinyLoanSle->at(sfLoanScale) == -12);
            BEAST_EXPECT(bigLoanSle->at(sfLoanScale) == -11);
            BEAST_EXPECT(getAssetsTotalScale(vaultSle) == -11);

            Number const expectedCoverAfter{1'330'651'855'688'458'000LL, -15};
            Number const clawbackAmount = Number{5'000} - expectedCoverAfter;
            Asset const rawAsset{iou};
            env(loanBroker::coverClawback(issuer),
                loanBroker::loanBrokerID(brokerKeylet.key),
                amount(STAmount{rawAsset, clawbackAmount}),
                THISLINE);
            env.close();

            auto const brokerSle = env.le(keylet::loanbroker(brokerKeylet.key));
            if (!BEAST_EXPECT(brokerSle))
                return;
            BEAST_EXPECT(brokerSle->at(sfCoverAvailable) == expectedCoverAfter);

            Account const pseudo{"pseudo", brokerSle->at(sfAccount)};
            auto const pseudoBefore = env.balance(pseudo, iou);
            auto const loanKeylet = payBigLoan ? bigLoanKeylet : tinyLoanKeylet;
            auto const loanSle = env.le(loanKeylet);
            if (!BEAST_EXPECT(loanSle))
                return;
            auto const periodicPayment = loanSle->at(sfPeriodicPayment);
            auto const serviceFee = loanSle->at(sfLoanServiceFee);
            auto const loanScale = loanSle->at(sfLoanScale);
            auto const payAmount =
                STAmount{rawAsset, roundPeriodicPayment(rawAsset, periodicPayment, loanScale) +
                        serviceFee};

            env(loan::pay(borrower, loanKeylet.key, payAmount), fee(XRP(10)), THISLINE);
            env.close();

            auto const pseudoAfter = env.balance(pseudo, iou);
            BEAST_EXPECT((pseudoAfter.number() > pseudoBefore.number()) == expectPseudoFee);
        };

        // Current 3.1.3 computes the minimum-cover threshold at loan scale
        // inside LoanPay.  The same broker cover therefore sends the big
        // loan's fee to the pseudo account while the tiny loan's fee goes to
        // the broker owner.
        run(/*payBigLoan=*/true, /*expectPseudoFee=*/true);
        run(/*payBigLoan=*/false, /*expectPseudoFee=*/false);
    }

    void
    testLoanBrokerDeleteLockedMPTCoverCurrent()
    {
        testcase("LoanBrokerDelete current — locked MPT cover is returned");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const alice{"alice"};

        Env env(*this);
        env.fund(XRP(100'000), issuer, alice);
        env.close();

        MPTTester mptt{env, issuer, mptInitNoFund};
        mptt.create(
            {.flags = tfMPTCanClawback | tfMPTCanTransfer | tfMPTCanLock});
        PrettyAsset const mpt{mptt.issuanceID()};

        mptt.authorize({.account = alice});
        env.close();
        env(pay(issuer, alice, mpt(100'000)));
        env.close();

        Vault vault{env};
        auto [tx, vaultKeylet] =
            vault.create({.owner = alice, .asset = mpt});
        env(tx);
        env.close();

        env(vault.deposit(
                {.depositor = alice,
                 .id = vaultKeylet.key,
                 .amount = mpt(10'000)}));
        env.close();

        auto const brokerKeylet = keylet::loanbroker(alice.id(), env.seq(alice));
        env(loanBroker::set(alice, vaultKeylet.key));
        env.close();

        env(loanBroker::coverDeposit(alice, brokerKeylet.key, mpt(5'000)));
        env.close();

        auto const broker = env.le(brokerKeylet);
        if (!BEAST_EXPECT(broker))
            return;
        BEAST_EXPECT(broker->at(sfCoverAvailable) > 0);

        auto const brokerPseudoID = broker->at(sfAccount);
        auto const pseudoMptKey =
            keylet::mptoken(mptt.issuanceID(), brokerPseudoID);
        auto const pseudoMpt = env.le(pseudoMptKey);
        if (!BEAST_EXPECT(pseudoMpt))
            return;

        Json::Value lock;
        lock[jss::Account] = issuer.human();
        lock[jss::TransactionType] = jss::MPTokenIssuanceSet;
        lock[sfMPTokenIssuanceID.jsonName] = to_string(mptt.issuanceID());
        lock[jss::Holder] = toBase58(brokerPseudoID);
        lock[jss::Flags] = tfMPTLock;
        env(lock);
        env.close();

        auto const lockedPseudoMpt = env.le(pseudoMptKey);
        if (!BEAST_EXPECT(lockedPseudoMpt))
            return;
        BEAST_EXPECT(lockedPseudoMpt->isFlag(lsfMPTLocked));

        auto const aliceBalanceBefore = env.balance(alice, mpt);
        env(loanBroker::del(alice, brokerKeylet.key),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        BEAST_EXPECT(!env.le(brokerKeylet));
        BEAST_EXPECT(!env.le(pseudoMptKey));
        BEAST_EXPECT(env.balance(alice, mpt) > aliceBalanceBefore);
    }

    void
    testVaultDepositOppositeLimitInternalCurrent()
    {
        testcase("VaultDeposit current — opposite trustline limit causes tefINTERNAL");
        using namespace jtx;

        Env env{*this};
        Account const gateway{"gateway"};
        Account const owner{"owner"};
        Account const depositor{"depositor"};

        env.fund(XRP(10'000), gateway, owner, depositor);
        env.close();

        env(fset(gateway, asfDefaultRipple));
        env.close();

        PrettyAsset const usd = gateway["USD"];
        env.trust(usd(1000), depositor);
        env(pay(gateway, depositor, usd(100)));
        env.close();

        env(trust(gateway, depositor["USD"](1000)));
        env.close();

        Vault vault{env};
        auto [vaultTx, vaultKeylet] =
            vault.create({.owner = owner, .asset = usd});
        env(vaultTx);
        env.close();

        env(vault.deposit(
                {.depositor = depositor,
                 .id = vaultKeylet.key,
                 .amount = usd(500)}),
            ter(tefINTERNAL),
            THISLINE);
        env.close();
    }

    void
    testEscrowCancelDeletedIOUTrustlineCurrent()
    {
        testcase("EscrowCancel current — deleted IOU trustline returns tefEXCEPTION");
        using namespace jtx;
        using namespace std::literals;

        Env env{*this};
        auto const baseFee = env.current()->fees().base;
        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const gateway{"gateway"};
        PrettyAsset const usd = gateway["USD"];

        env.fund(XRP(10'000), alice, bob, gateway);
        env.close();

        env(fset(gateway, asfAllowTrustLineLocking));
        env.close();

        env.trust(usd(100'000), alice);
        env.trust(usd(100'000), bob);
        env.close();

        env(pay(gateway, alice, usd(10'000)));
        env.close();

        auto const seq = env.seq(alice);
        env(escrow::create(alice, bob, usd(1'000)),
            escrow::finish_time(env.now() + 1s),
            escrow::cancel_time(env.now() + 2s),
            fee(baseFee),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        BEAST_EXPECT(env.balance(alice, usd) == usd(9'000));

        env(pay(alice, gateway, usd(9'000)));
        env.close();

        env(trust(alice, usd(0)));
        env.close();

        auto const trustLineKey =
            keylet::line(alice.id(), usd.raw().get<Issue>());
        BEAST_EXPECT(!env.current()->exists(trustLineKey));

        env.close();
        env.close();

        env(escrow::cancel(alice, alice, seq),
            fee(baseFee),
            ter(tefEXCEPTION),
            THISLINE);
        env.close();
    }

    void
    testAMMStaleAuthAccountsAfterReinitCurrent()
    {
        testcase("AMM current — stale AuthAccounts survive empty reinit");
        using namespace jtx;

        Account const gateway{"gateway"};
        Account const alice{"alice"};
        Account const carol{"carol"};
        Account const bob{"bob"};
        Account const dan{"dan"};
        Account const ed{"ed"};
        PrettyAsset const usd = gateway["USD"];

        Env env{
            *this,
            testable_amendments() - featureSingleAssetVault -
                featureLendingProtocol};
        fund(env,
             gateway,
             {alice, carol, bob, dan, ed},
             XRP(50'000),
             {usd(50'000)});

        AMM amm{env, alice, XRP(10'000), usd(10'000)};

        for (auto i = 0u; i < maxDeletableAMMTrustLines + 10; ++i)
        {
            Account const holder{std::string{"ammline"} + std::to_string(i)};
            env.fund(XRP(1'000), holder);
            env(trust(holder, STAmount{amm.lptIssue(), 10'000}));
            env.close();
        }

        amm.deposit(carol, 1'000'000);
        env(amm.bid({.account = carol, .bidMin = 100, .authAccounts = {bob, dan}}));
        env.close();

        BEAST_EXPECT(amm.expectAuctionSlot({bob.id(), dan.id()}));

        amm.withdrawAll(alice);
        amm.withdrawAll(carol);
        BEAST_EXPECT(amm.ammExists());
        BEAST_EXPECT(amm.getLPTokensBalance() == IOUAmount{0});
        BEAST_EXPECT(amm.expectAuctionSlot({bob.id(), dan.id()}));

        amm.deposit(
            ed,
            std::nullopt,
            XRP(10'000),
            usd(10'000),
            std::nullopt,
            tfTwoAssetIfEmpty,
            std::nullopt,
            std::nullopt,
            500);

        // Current 3.1.3 resets the slot owner/fee but leaves stale
        // sfAuthAccounts from the previous auction slot.
        BEAST_EXPECT(amm.expectAuctionSlot({bob.id(), dan.id()}));
        BEAST_EXPECT(!amm.expectAuctionSlot({}));
    }

    void
    testDelegateeAccountDeleteLeavesStaleDelegateCurrent()
    {
        testcase("Delegate current — delegatee account deletion leaves stale delegation");
        using namespace jtx;

        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const carol{"carol"};
        Env env{*this};

        env.fund(XRP(100'000), alice, bob, carol);
        env.close();

        env(delegate::set(alice, bob, {"Payment"}), THISLINE);
        env.close();

        auto const delegateKey = keylet::delegate(alice.id(), bob.id());
        BEAST_EXPECT(env.closed()->exists(delegateKey));
        BEAST_EXPECT(ownerCount(env, alice) == 1);

        for (std::uint32_t i = 0; i < 256; ++i)
            env.close();

        auto const bobBalance = env.balance(bob);
        auto const carolBalance = env.balance(carol);
        auto const deleteFee = drops(env.current()->fees().increment);

        env(acctdelete(bob, carol), fee(deleteFee), ter(tesSUCCESS), THISLINE);
        env.close();

        // Current 3.1.3 lets the authorized/delegatee account disappear while
        // the Delegate ledger entry and delegator reserve remain. Later
        // upstream fix 4da46d31 indexes Delegate entries in both owner
        // directories so this cleanup occurs atomically.
        BEAST_EXPECT(!env.closed()->exists(keylet::account(bob.id())));
        BEAST_EXPECT(env.closed()->exists(delegateKey));
        BEAST_EXPECT(ownerCount(env, alice) == 1);
        BEAST_EXPECT(env.balance(carol) == carolBalance + bobBalance - deleteFee);
    }

    void
    testMPTDomainRequireAuthCanBeClearedCurrent()
    {
        testcase("MPT current — domain-bound RequireAuth can be cleared");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const credIssuer{"credIssuer"};
        pdomain::Credentials const credentials{
            {.issuer = credIssuer, .credType = "domain credential"}};

        Env env{*this};
        env.fund(XRP(10'000), issuer, credIssuer);
        env.close();

        env(pdomain::setTx(credIssuer, credentials), THISLINE);
        env.close();
        auto const domainID = pdomain::getNewDomain(env.meta());

        MPTTester mpt{env, issuer, mptInitNoFund};
        mpt.create({
            .flags = tfMPTRequireAuth | MPTDEXFlags,
            .mutableFlags = tmfMPTCanMutateRequireAuth,
            .domainID = domainID,
        });

        auto const issuanceKey = keylet::mptIssuance(mpt.issuanceID());
        auto const before = env.le(issuanceKey);
        if (!BEAST_EXPECT(before))
            return;
        BEAST_EXPECT(before->isFlag(lsfMPTRequireAuth));
        BEAST_EXPECT(before->isFieldPresent(sfDomainID));

        env(MPTTester::setjv({
                .account = issuer,
                .id = mpt.issuanceID(),
                .mutableFlags = tmfMPTClearRequireAuth,
            }),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        // Current 3.1.3 lets the issuer remove RequireAuth while retaining a
        // DomainID on the issuance. Later upstream fix 366899d5 rejects this
        // with tecNO_PERMISSION because a domain-bound issuance must not be
        // made permissionless by clearing RequireAuth.
        auto const after = env.le(issuanceKey);
        if (!BEAST_EXPECT(after))
            return;
        BEAST_EXPECT(!after->isFlag(lsfMPTRequireAuth));
        BEAST_EXPECT(after->isFieldPresent(sfDomainID));
    }

    void
    testNumberMaxRepCuspUpwardRoundsDownCurrent()
    {
        testcase("Number current — upward rounding falls below exact at maxRep cusp");
        namespace mp = boost::multiprecision;

        NumberMantissaScaleGuard const mg{MantissaRange::large};
        NumberRoundModeGuard const rg{Number::upward};

        constexpr std::int64_t kAValue = 1'000'000'000'000'049'863LL;
        constexpr std::int64_t kBValue = 9'223'372'036'854'315'903LL;

        Number const a = kAValue;
        Number const b = kBValue;
        Number const product = a * b;

        mp::cpp_int const exactProduct =
            mp::cpp_int{kAValue} * mp::cpp_int{kBValue};
        mp::cpp_int storedValue = product.mantissa();
        for (int i = 0; i < product.exponent(); ++i)
            storedValue *= 10;

        // Current 3.1.3 violates upward rounding at this cusp: the stored
        // product is lower than the exact integer product. Upstream commit
        // 4094f7f6c changes normalization so upward rounding stays above exact.
        BEAST_EXPECT(storedValue < exactProduct);
    }

    void
    testNumberDivisionUpwardRoundsDownCurrent()
    {
        testcase("Number current - upward division falls below exact quotient");
        namespace mp = boost::multiprecision;
        using dec = mp::cpp_dec_float_50;

        auto const pow10 = [](int n) {
            dec result = 1;
            if (n >= 0)
            {
                for (int i = 0; i < n; ++i)
                    result *= 10;
            }
            else
            {
                for (int i = 0; i < -n; ++i)
                    result /= 10;
            }
            return result;
        };

        NumberMantissaScaleGuard const mg{MantissaRange::large};
        NumberRoundModeGuard const rg{Number::upward};

        constexpr std::int64_t aValue = 2LL;
        constexpr std::int64_t bValue = 1'000'000'000'000'000'007LL;

        Number const a{aValue, 0};
        Number const b{bValue, 0};
        Number const quotient = a / b;

        dec const exact = dec(aValue) / dec(bValue);
        dec const stored = dec(quotient.mantissa()) * pow10(quotient.exponent());

        // Current 3.1.3 loses the sub-correction remainder in operator/=, so
        // Upward returns a positive quotient below exact. Upstream commit
        // 48b1716e6 expands the correction precision and restores the invariant.
        BEAST_EXPECT(stored < exact);
    }

    void
    testMPTTransferRateLargeAmountOverflowCurrent()
    {
        testcase("MPT current — transfer-rate scaling overflows large integral amount");

        MPTIssue const asset{makeMptID(1, AccountID(0x4985601))};
        Rate const transferRate{1'500'000'000};
        STAmount const largeAmount{
            asset, std::uint64_t{1'230'000'000'000'000'000ULL}};

        bool threw = false;
        try
        {
            (void)multiplyRound(largeAmount, transferRate, true);
        }
        catch (std::overflow_error const&)
        {
            threw = true;
        }

        // Current 3.1.3 reaches the legacy scaled-mantissa path and overflows
        // on valid 63-bit MPT amounts with an issuer transfer rate. Upstream
        // commit 22fbf4d06 routes this case through Number arithmetic for the
        // MPT/V2 profile.
        BEAST_EXPECT(threw);
    }

    void
    testDelegatePaymentFeeReserveCouplingCurrent()
    {
        testcase("Delegate current — delegated fee is coupled to delegator reserve");
        using namespace jtx;

        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const carol{"carol"};

        Env env{*this};
        auto const baseFee = env.current()->fees().base;
        auto const reserve = env.current()->fees().accountReserve(1);
        auto const paymentAmount = XRP(1);
        auto const highFee = reserve + baseFee;

        env.fund(paymentAmount + reserve + baseFee, alice);
        env.fund(XRP(1'000), bob, carol);
        env.close();

        env(delegate::set(alice, bob, {"Payment"}), THISLINE);
        env.close();

        BEAST_EXPECT(env.balance(alice) == paymentAmount + reserve);
        auto const aliceBefore = env.balance(alice);
        auto const bobBefore = env.balance(bob);
        auto const carolBefore = env.balance(carol);

        env(pay(alice, carol, paymentAmount),
            delegate::as(bob),
            fee(highFee),
            ter(tecUNFUNDED_PAYMENT),
            THISLINE);

        // Bob is the delegated fee payer and has enough balance for the fee.
        // Current 3.1.3 still rejects because the delegator-side payment check
        // incorrectly couples fee size to Alice's reserve requirement. Later
        // upstream fix 17f26ba97 decouples the fee payer from the delegator's
        // reserve calculation so this transaction succeeds.
        BEAST_EXPECT(env.balance(alice) == aliceBefore);
        BEAST_EXPECT(env.balance(bob) == bobBefore - highFee);
        BEAST_EXPECT(env.balance(carol) == carolBefore);
    }

    void
    testDelegatedVaultCreateCurrent()
    {
        testcase("Delegate current — SAV transaction can be delegated");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const owner{"owner"};
        Account const bob{"bob"};
        Env env{*this};
        Vault vault{env};

        env.fund(XRP(100'000), issuer, owner, bob);
        env.close();

        PrettyAsset const asset = issuer["IOU"];

        env(delegate::set(owner, bob, {"VaultCreate"}), ter(tesSUCCESS), THISLINE);
        env.close();

        auto [tx, vaultKeylet] = vault.create({.owner = owner, .asset = asset});
        env(tx, delegate::as(bob), ter(tesSUCCESS), THISLINE);
        env.close();

        // Current 3.1.3 lets Single Asset Vault transactions enter the
        // delegation surface. Later upstream fix 46d5c67a marks SAV and
        // Lending transactions NotDelegable until their delegation semantics
        // are fully tested and validated.
        BEAST_EXPECT(env.le(vaultKeylet));
    }

    void
    testPermissionedDexHybridOfferQualityCurrent()
    {
        testcase("Permissioned DEX current — hybrid offer open-book quality mismatch");
        using namespace jtx;

        Env env{*this};
        auto const setup = PermissionedDEX(env);

        env(offer(setup.bob, XRP(100), setup.USD(40)),
            txflags(tfHybrid),
            domain(setup.domainID),
            THISLINE);
        env.close();

        auto const aliceOfferSeq = env.seq(setup.alice);
        env(offer(setup.alice, setup.USD(100), XRP(300)),
            txflags(tfHybrid),
            domain(setup.domainID),
            THISLINE);
        env.close();

        auto const sle = env.le(keylet::offer(setup.alice.id(), aliceOfferSeq));
        if (!BEAST_EXPECT(sle))
            return;
        BEAST_EXPECT(sle->isFieldPresent(sfAdditionalBooks));
        BEAST_EXPECT(sle->getFieldArray(sfAdditionalBooks).size() == 1);

        auto const domainDirKey = sle->getFieldH256(sfBookDirectory);
        auto const openDirKey =
            sle->getFieldArray(sfAdditionalBooks)[0].getFieldH256(sfBookDirectory);
        auto const domainQuality = getQuality(domainDirKey);
        auto const openQuality = getQuality(openDirKey);

        auto const domainDir = env.le(Keylet(ltDIR_NODE, domainDirKey));
        auto const openDir = env.le(Keylet(ltDIR_NODE, openDirKey));
        if (!BEAST_EXPECT(domainDir && openDir))
            return;

        // Current 3.1.3 places the open-book side of the partially-crossed
        // hybrid offer at the post-crossing quality, while sfExchangeRate
        // still records the pre-crossing quality. Later upstream fix
        // 28cc20c81 makes both directories use the same placement rate and
        // adds a LedgerStateFix for already-created bad directories.
        BEAST_EXPECT(domainQuality != openQuality);
        BEAST_EXPECT(domainDir->getFieldU64(sfExchangeRate) == domainQuality);
        BEAST_EXPECT(openDir->getFieldU64(sfExchangeRate) == domainQuality);
        BEAST_EXPECT(openDir->getFieldU64(sfExchangeRate) != openQuality);
    }

    void
    testPermissionedDexCancelRegularOfferInvariantCurrent()
    {
        testcase("Permissioned DEX current — cancel regular offer via domain offer invariant");
        using namespace jtx;

        Env env{*this};
        auto const setup = PermissionedDEX(env);

        auto const regularSeq = env.seq(setup.bob);
        env(offer(setup.bob, XRP(10), setup.USD(10)), THISLINE);
        env.close();
        BEAST_EXPECT(offerExists(env, setup.bob, regularSeq));

        auto const domainSeq = env.seq(setup.bob);
        auto tx = offer(setup.bob, XRP(20), setup.USD(20));
        tx[jss::OfferSequence] = regularSeq;
        env(tx, domain(setup.domainID), ter(tecINVARIANT_FAILED), THISLINE);
        env.close();

        // The valid intent is "cancel my regular offer and create a domain
        // offer." Current 3.1.3 flags the deleted regular offer as a
        // permissioned-DEX invariant failure, so the transaction cannot land.
        // Later upstream fix 8c0080020 ignores deleted regular offers after
        // fixCleanup3_2_0.
        BEAST_EXPECT(offerExists(env, setup.bob, regularSeq));
        BEAST_EXPECT(!offerExists(env, setup.bob, domainSeq));
    }

    void
    testLoanPaymentFactorCancellationCurrent()
    {
        testcase("Loan current — near-zero payment factor cancellation");

        Number const r = loanPeriodicRate(TenthBips32{1}, 600);
        std::uint32_t const n = 3;

        // Independent polynomial reference for
        // F(r,3) = r*(1+r)^3 / ((1+r)^3 - 1), with r factored out:
        // F(r,3) = (1 + 3r + 3r^2 + r^3) / (3 + 3r + r^2).
        Number const reference =
            (1 + 3 * r + 3 * r * r + r * r * r) / (3 + 3 * r + r * r);
        Number const currentFactor = detail::computePaymentFactor(r, n);

        // Current 3.1.3 uses the direct `(1+r)^n - 1` form. At this
        // near-zero rate it loses precision to cancellation; later upstream
        // fix ad2195f12 switches to a stable power-minus-one path.
        BEAST_EXPECT(currentFactor != reference);
        BEAST_EXPECT(abs(currentFactor - reference) > Number(1, -12));
    }

    void
    testVaultWithdrawScaleBoundaryInvariantCurrent()
    {
        testcase("Vault current — withdraw across IOU scale boundary invariant");
        using namespace jtx;

        Env env{*this};
        Account const issuer{"issuer"};
        Account const alice{"alice"};

        env.fund(XRP(100'000), issuer, alice);
        env.close();
        env(fset(issuer, asfDefaultRipple), THISLINE);
        env.close();

        PrettyAsset const usd{issuer["USD"]};
        STAmount const fundAndDeposit{usd.raw(), Number{1, 16}};

        env(trust(alice, STAmount{usd.raw(), 2, 16}), THISLINE);
        env.close();
        env(pay(issuer, alice, fundAndDeposit), THISLINE);
        env.close();

        Vault vault{env};
        auto [vaultTx, vaultKeylet] = vault.create({.owner = alice, .asset = usd});
        vaultTx[sfScale] = 0;
        env(vaultTx, THISLINE);
        env.close();

        env(vault.deposit({.depositor = alice, .id = vaultKeylet.key, .amount = fundAndDeposit}),
            THISLINE);
        env.close();

        // Later upstream fix 633ef4706 adds this exact boundary class as a
        // regression: current 3.1.3 rounds the vault delta at the anterior
        // IOU scale and fires ValidVault even though the withdraw is
        // consistent at the posterior precision.
        env(vault.withdraw({.depositor = alice, .id = vaultKeylet.key, .amount = usd(5)}),
            ter(tecINVARIANT_FAILED),
            THISLINE);
    }

    void
    testVaultIssuerDepositScaleBoundaryInvariantCurrent()
    {
        testcase("Vault current — issuer deposit at IOU edge invariant");
        using namespace jtx;

        Env env{*this};
        Account const issuer{"issuer"};
        Account const owner{"owner"};

        env.fund(XRP(100'000), issuer, owner);
        env.close();
        env(fset(issuer, asfDefaultRipple), THISLINE);
        env.close();

        PrettyAsset const usd{issuer["USD"]};
        STAmount const trustLimit{usd.raw(), 2, 16};
        STAmount const ownerFund{usd.raw(), Number{9'999'999'999'999'999LL}};

        env(trust(owner, trustLimit), THISLINE);
        env.close();
        env(pay(issuer, owner, ownerFund), THISLINE);
        env.close();

        Vault vault{env};
        auto [vaultTx, vaultKeylet] = vault.create({.owner = owner, .asset = usd});
        vaultTx[sfScale] = 0;
        env(vaultTx, THISLINE);
        env.close();

        env(vault.deposit({.depositor = owner, .id = vaultKeylet.key, .amount = ownerFund}),
            THISLINE);
        env.close();

        // Later upstream fix 633ef4706 changes this path to proactive
        // precision-loss rejection. Current 3.1.3 applies far enough to hit
        // the vault invariant at finalize.
        env(vault.deposit({.depositor = issuer, .id = vaultKeylet.key, .amount = usd(2)}),
            ter(tecINVARIANT_FAILED),
            THISLINE);
    }

    void
    testVaultSoleShareholderImpairedExitCurrent()
    {
        testcase("Vault current — sole shareholder impaired exit is stuck");
        using namespace jtx;

        Env env{
            *this,
            testable_amendments() | featureSingleAssetVault |
                featureLendingProtocol | featureMPTokensV1};
        Account const issuer{"issuer"};
        Account const lender{"lender"};
        Account const bob{"bob"};
        Account const borrower{"borrower"};
        PrettyAsset const asset = issuer["IOU"];

        env.fund(XRP(1'000'000), issuer, lender, bob, borrower);
        env.close();
        env(trust(lender, asset(10'000'000)), THISLINE);
        env(trust(bob, asset(10'000'000)), THISLINE);
        env(trust(borrower, asset(10'000'000)), THISLINE);
        env.close();
        env(pay(issuer, lender, asset(1'000'000)), THISLINE);
        env(pay(issuer, bob, asset(1'000'000)), THISLINE);
        env(pay(issuer, borrower, asset(100'000)), THISLINE);
        env.close();

        Vault vault{env};
        auto [createTx, vaultKeylet] = vault.create({.owner = lender, .asset = asset});
        env(createTx, THISLINE);
        env.close();

        env(vault.deposit({.depositor = lender, .id = vaultKeylet.key, .amount = asset(5'000)}),
            ter(tesSUCCESS),
            THISLINE);
        env(vault.deposit({.depositor = bob, .id = vaultKeylet.key, .amount = asset(5'000)}),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        auto const brokerID = keylet::loanbroker(lender.id(), env.seq(lender)).key;
        env(loanBroker::set(lender, vaultKeylet.key),
            loanBroker::debtMaximum(asset(33'330).value()),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        auto const brokerSle = env.le(keylet::loanbroker(brokerID));
        if (!BEAST_EXPECT(brokerSle))
            return;
        auto const loanKeylet = keylet::loan(brokerID, brokerSle->at(sfLoanSequence));

        env(loan::set(borrower, brokerID, 3'333),
            sig(sfCounterpartySignature, lender),
            loan::paymentTotal(2),
            loan::paymentInterval(600),
            fee(env.current()->fees().base * 2),
            ter(tesSUCCESS),
            THISLINE);
        env.close();
        env(loan::manage(lender, loanKeylet.key, tfLoanImpair), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const vaultAfterLoan = env.le(vaultKeylet);
        if (!BEAST_EXPECT(vaultAfterLoan))
            return;
        auto const shareID = vaultAfterLoan->at(sfShareMPTID);
        BEAST_EXPECT(vaultAfterLoan->at(sfLossUnrealized) == asset(3'333).value());

        auto const tokenBob = env.le(keylet::mptoken(shareID, bob.id()));
        if (!BEAST_EXPECT(tokenBob))
            return;
        STAmount const bobShares{MPTIssue{shareID}, Number(tokenBob->getFieldU64(sfMPTAmount))};

        // Bob exits as a non-sole shareholder at the discounted rate. The
        // lender is now the sole remaining shareholder while the impaired
        // receivable is still outstanding.
        env(vault.withdraw({.depositor = bob, .id = vaultKeylet.key, .amount = bobShares}),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        auto const tokenLender = env.le(keylet::mptoken(shareID, lender.id()));
        auto const vaultBefore = env.le(vaultKeylet);
        if (!BEAST_EXPECT(tokenLender && vaultBefore))
            return;
        std::uint64_t const lenderShares = tokenLender->getFieldU64(sfMPTAmount);
        Number const availableBefore = vaultBefore->at(sfAssetsAvailable);
        Number const totalBefore = vaultBefore->at(sfAssetsTotal);
        Number const lossBefore = vaultBefore->at(sfLossUnrealized);
        STAmount const requestAssets{asset.raw(), availableBefore};

        // Later upstream fix 49567e728 changes sole-shareholder impaired
        // withdraw math. Current 3.1.3 tries to redeem every remaining share
        // for available cash and hits the zero-sized-vault invariant.
        env(vault.withdraw({.depositor = lender, .id = vaultKeylet.key, .amount = requestAssets}),
            ter(tecINVARIANT_FAILED),
            THISLINE);
        env.close();

        auto const vaultAfter = env.le(vaultKeylet);
        auto const issuanceAfter = env.le(keylet::mptIssuance(shareID));
        if (!BEAST_EXPECT(vaultAfter && issuanceAfter))
            return;
        BEAST_EXPECT(issuanceAfter->getFieldU64(sfOutstandingAmount) == lenderShares);
        BEAST_EXPECT(vaultAfter->at(sfAssetsAvailable) == availableBefore);
        BEAST_EXPECT(vaultAfter->at(sfAssetsTotal) == totalBefore);
        BEAST_EXPECT(vaultAfter->at(sfLossUnrealized) == lossBefore);
    }

    void
    testDelegateMultisignDelegatorRejectedCurrent()
    {
        testcase("Delegate current — delegator multisign rejected for delegate");
        using namespace jtx;

        Env env{*this};
        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const carol{"carol"};
        Account const daria{"daria"};

        env.fund(XRP(100'000), alice, bob, carol, daria);
        env.close();

        env(delegate::set(alice, bob, {"Payment"}), ter(tesSUCCESS), THISLINE);
        env.close();

        // Bob's signer list includes the delegator Alice plus Daria. Later
        // upstream fix 9cb074067 permits this for delegated transactions
        // because Bob, not Alice, is the fee payer / delegate whose self-sign
        // participation must be forbidden.
        env(signers(bob, 2, {{alice, 1}, {daria, 1}}), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const aliceBefore = env.balance(alice);
        auto const bobBefore = env.balance(bob);
        auto const carolBefore = env.balance(carol);
        auto const dariaBefore = env.balance(daria);

        env(pay(alice, carol, XRP(100)),
            fee(XRP(10)),
            delegate::as(bob),
            msig(alice, daria),
            ter(telENV_RPC_FAILED),
            THISLINE);
        env.close();

        // Current 3.1.3 checks multisign self-signing against sfAccount
        // (Alice) instead of the delegate/fee payer (Bob), so a valid
        // delegated multisig path cannot be submitted.
        BEAST_EXPECT(env.balance(alice) == aliceBefore);
        BEAST_EXPECT(env.balance(bob) == bobBefore);
        BEAST_EXPECT(env.balance(carol) == carolBefore);
        BEAST_EXPECT(env.balance(daria) == dariaBefore);
    }

    void
    testMPTNonCanonicalIssuerPaymentCurrent()
    {
        testcase("MPT current — non-canonical amount reaches ledger engine");
        using namespace jtx;

        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const issuer{"issuer"};

        Env env{*this, testable_amendments()};
        env.fund(XRP(100'000), alice, bob, issuer);
        env.close();

        MPTTester const mpt{
            {.env = env,
             .issuer = issuer,
             .holders = {alice, bob},
             .pay = 10'000,
             .flags = tfMPTCanTransfer | tfMPTCanTrade | tfMPTCanEscrow | tfMPTCanClawback}};
        MPTIssue const issue{mpt.issuanceID()};

        BEAST_EXPECT(
            (env.balance(alice, issue).value() == STAmount{MPTAmount{10'000}, issue}));
        BEAST_EXPECT(
            (env.balance(issuer, issue).value() == STAmount{MPTAmount{-20'000}, issue}));

        auto const badAmount = STAmount{
            issue,
            std::numeric_limits<std::uint64_t>::max(),
            0,
            false,
            STAmount::unchecked{}};

        auto jt = env.jt(pay(issuer, alice, STAmount{issue, std::uint64_t{1}}));
        STTx tx{*jt.stx};
        tx.setFieldAmount(sfAmount, badAmount);
        tx.sign(issuer.pk(), issuer.sk());
        jt.stx = std::make_shared<STTx const>(tx);
        jt.ter = tecPATH_PARTIAL;

        env.submit(jt);
        env.close();

        // Current 3.1.3 accepts the non-canonical amount far enough into the
        // transaction engine to consume sequence/fee and return a tec code.
        // Later upstream fix dcd2ff0b5 rejects this class as temBAD_AMOUNT.
        BEAST_EXPECT(
            (env.balance(alice, issue).value() == STAmount{MPTAmount{10'000}, issue}));
        BEAST_EXPECT(
            (env.balance(issuer, issue).value() == STAmount{MPTAmount{-20'000}, issue}));
    }

    void
    testMPTSTIssueLegacyWireOrderCurrent()
    {
        testcase("MPT current — STIssue sequence bytes are legacy-swapped");
        using namespace jtx;

        Account const alice{"alice"};

        // Use a non-palindromic sequence so byte-order inversion is visible.
        // makeMptID stores the sequence in canonical big-endian byte order:
        // de ad be ef || issuer.
        MPTID const canonical = makeMptID(0xDEADBEEF, alice.id());
        STIssue const original(sfAsset, Asset{MPTIssue{canonical}});

        Serializer legacyWire;
        original.add(legacyWire);
        Slice const legacy = legacyWire.slice();

        BEAST_EXPECT(legacy.size() == 44);
        BEAST_EXPECT(legacy[40] == 0xEF);
        BEAST_EXPECT(legacy[41] == 0xBE);
        BEAST_EXPECT(legacy[42] == 0xAD);
        BEAST_EXPECT(legacy[43] == 0xDE);

        // The internal codec round-trip is symmetrically wrong, so it hides
        // the wire-format defect inside a homogeneous rippled process.
        SerialIter legacyIter(legacy);
        BEAST_EXPECT(STIssue(legacyIter, sfAsset) == Asset{MPTIssue{canonical}});

        // A canonical client path that embeds the raw MPTID sequence bytes
        // into a V1 STIssue payload does not parse back to the same MPTID.
        Serializer canonicalWire;
        canonicalWire.addBitString(alice.id());
        canonicalWire.addBitString(noAccount());
        canonicalWire.addRaw(canonical.data(), sizeof(std::uint32_t));

        SerialIter canonicalIter(canonicalWire.slice());
        BEAST_EXPECT(STIssue(canonicalIter, sfAsset) != Asset{MPTIssue{canonical}});
    }

    void
    testMPTLockedHolderUnauthorizeWithoutSavCurrent()
    {
        testcase("MPT current — locked holder can delete lock state without SAV");
        using namespace jtx;

        FeatureBitset const features =
            (testable_amendments() | featureMPTokensV1) - featureSingleAssetVault;
        Account const issuer{"issuer"};
        Account const holder{"holder"};

        Env env{*this, features};
        MPTTester mpt{env, issuer, {.holders = {holder}}};
        mpt.create({.ownerCount = 1, .holderCount = 0, .flags = tfMPTCanLock});
        mpt.authorize({.account = holder, .holderCount = 1});

        auto const holderKey = keylet::mptoken(mpt.issuanceID(), holder.id());
        BEAST_EXPECT(env.le(holderKey));

        mpt.set({.account = issuer, .holder = holder, .flags = tfMPTLock});
        auto const locked = env.le(holderKey);
        if (!BEAST_EXPECT(locked))
            return;
        BEAST_EXPECT(locked->isFlag(lsfMPTLocked));

        // Current 3.1.3 gates the locked-holder deletion check on
        // featureSingleAssetVault. With MPTokensV1 active and SAV inactive,
        // the holder can delete the locked MPToken and erase the issuer's lock.
        mpt.authorize({.account = holder, .holderCount = 0, .flags = tfMPTUnauthorize});
        BEAST_EXPECT(!env.le(holderKey));

        mpt.authorize({.account = holder, .holderCount = 1});
        auto const recreated = env.le(holderKey);
        if (!BEAST_EXPECT(recreated))
            return;
        BEAST_EXPECT(!recreated->isFlag(lsfMPTLocked));
    }

    void
    testTrustlinePositiveBalanceNoOwnerReserveCurrent()
    {
        testcase("TrustLine current — offer crossing creates positive balance without reserve");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const market{"market"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this};
        env.fund(XRP(100'000), gw, alice, market);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(market, USD(1'000)), THISLINE);
        env.close();

        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, market, USD(1'000)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        env(offer(market, XRP(100), USD(100)), THISLINE);
        env.close();

        env(offer(alice, USD(50), XRP(50)), THISLINE);
        env.close();

        auto const crossed = env.le(lineKey);
        if (!BEAST_EXPECT(crossed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!crossed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceOfferExistingOwnersCurrent()
    {
        testcase("TrustLine current — offer crossing leaves positive balance unowned with existing owner objects");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const market{"market"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this};
        env.fund(XRP(100'000), gw, alice, market);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(market, USD(1'000)), THISLINE);
        env.close();

        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, market, USD(1'000)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        env(ticket::create(alice, 2), THISLINE);
        env.close();
        BEAST_EXPECT(ownerCount(env, alice) == 2);

        env(offer(market, XRP(100), USD(100)), THISLINE);
        env.close();

        env(offer(alice, USD(50), XRP(50)), THISLINE);
        env.close();

        auto const crossed = env.le(lineKey);
        if (!BEAST_EXPECT(crossed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 2);
        BEAST_EXPECT(!crossed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceOfferReserveBoundaryCurrent()
    {
        testcase("TrustLine current — offer crossing succeeds below missing owner reserve");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const market{"market"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this};
        env.fund(XRP(100'000), gw, alice, market);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(market, USD(1'000)), THISLINE);
        env.close();

        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, market, USD(1'000)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        env(ticket::create(alice, 2), THISLINE);
        env.close();
        BEAST_EXPECT(ownerCount(env, alice) == 2);

        env(offer(market, XRP(100), USD(100)), THISLINE);
        env.close();

        // Leave alice with enough XRP for the crossing payment plus fee while
        // remaining below the reserve that would be required if the trustline
        // owner count were correctly raised from 2 to 3.
        auto const feeAmount = drops(env.current()->fees().base);
        auto const targetBeforeCross =
            env.current()->fees().accountReserve(2) + XRP(50) + feeAmount;
        auto const drain = env.balance(alice) - targetBeforeCross - feeAmount;
        env(pay(alice, env.master, drain), THISLINE);
        env.close();
        BEAST_EXPECT(env.balance(alice) == targetBeforeCross);

        env(offer(alice, USD(50), XRP(50)), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const crossed = env.le(lineKey);
        if (!BEAST_EXPECT(crossed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 2);
        BEAST_EXPECT(env.balance(alice) < env.current()->fees().accountReserve(3));
        BEAST_EXPECT(!crossed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceCheckCashCurrent()
    {
        testcase("TrustLine current — CheckCash creates positive balance without reserve");
        using namespace jtx;

        FeatureBitset const features =
            testable_amendments() | featureChecks | featureCheckCashMakesTrustLine;
        Account const gw{"gateway"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this, features};
        env.fund(XRP(100'000), gw, alice);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        uint256 const chkId{keylet::check(gw.id(), env.seq(gw)).key};
        env(check::create(gw, alice, USD(50)), THISLINE);
        env.close();

        env(check::cash(alice, chkId, USD(50)), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const cashed = env.le(lineKey);
        if (!BEAST_EXPECT(cashed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cashed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceCheckCashExistingOwnersCurrent()
    {
        testcase("TrustLine current — CheckCash leaves positive balance unowned with existing owner objects");
        using namespace jtx;

        FeatureBitset const features =
            testable_amendments() | featureChecks | featureCheckCashMakesTrustLine;
        Account const gw{"gateway"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this, features};
        env.fund(XRP(100'000), gw, alice);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        env(ticket::create(alice, 2), THISLINE);
        env.close();
        BEAST_EXPECT(ownerCount(env, alice) == 2);

        uint256 const chkId{keylet::check(gw.id(), env.seq(gw)).key};
        env(check::create(gw, alice, USD(50)), THISLINE);
        env.close();

        env(check::cash(alice, chkId, USD(50)), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const cashed = env.le(lineKey);
        if (!BEAST_EXPECT(cashed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 2);
        BEAST_EXPECT(!cashed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceCheckCashReserveBoundaryCurrent()
    {
        testcase("TrustLine current — CheckCash succeeds below missing owner reserve");
        using namespace jtx;

        FeatureBitset const features =
            testable_amendments() | featureChecks | featureCheckCashMakesTrustLine;
        Account const gw{"gateway"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this, features};
        env.fund(XRP(100'000), gw, alice);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const lineKey = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(lineKey);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        env(ticket::create(alice, 2), THISLINE);
        env.close();
        BEAST_EXPECT(ownerCount(env, alice) == 2);

        uint256 const chkId{keylet::check(gw.id(), env.seq(gw)).key};
        env(check::create(gw, alice, USD(50)), THISLINE);
        env.close();

        // Leave alice with exactly enough XRP to pay the CheckCash fee while
        // remaining below the reserve that would be required if the trustline
        // owner count were correctly raised from 2 to 3.
        auto const feeAmount = drops(env.current()->fees().base);
        auto const targetBeforeCash =
            env.current()->fees().accountReserve(2) + feeAmount;
        auto const drain = env.balance(alice) - targetBeforeCash - feeAmount;
        env(pay(alice, env.master, drain), THISLINE);
        env.close();
        BEAST_EXPECT(env.balance(alice) == targetBeforeCash);

        env(check::cash(alice, chkId, USD(50)), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const cashed = env.le(lineKey);
        if (!BEAST_EXPECT(cashed))
            return;
        BEAST_EXPECT(env.balance(alice, USD) == USD(50));
        BEAST_EXPECT(ownerCount(env, alice) == 2);
        BEAST_EXPECT(env.balance(alice) < env.current()->fees().accountReserve(3));
        BEAST_EXPECT(!cashed->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceTokenEscrowCurrent()
    {
        testcase("TrustLine current — TokenEscrow creates positive balance without reserve");
        using namespace jtx;
        using namespace std::chrono_literals;

        FeatureBitset const features = testable_amendments() | featureTokenEscrow;
        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];
        bool const bobHigh = bob.id() > gw.id();

        Env env{*this, features};
        auto const baseFee = env.current()->fees().base;
        env.fund(XRP(400'000), gw, alice, bob);
        env(fset(gw, asfAllowTrustLineLocking), THISLINE);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(bob, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, bob, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(bob, USD(0)), THISLINE);
        env.close();
        env(pay(bob, gw, USD(100)), THISLINE);
        env.close();

        auto const bobLine = keylet::line(bob, gw, to_currency("USD"));
        auto const cleared = env.le(bobLine);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, bob) == 0);
        BEAST_EXPECT(!cleared->isFlag(bobHigh ? lsfHighReserve : lsfLowReserve));

        auto const escrowSeq = env.seq(alice);
        env(escrow::create(alice, bob, USD(40)),
            escrow::condition(escrow::cb1),
            escrow::finish_time(env.now() + 1s),
            fee(baseFee * 150),
            THISLINE);
        env.close();

        env(escrow::finish(bob, alice, escrowSeq),
            escrow::condition(escrow::cb1),
            escrow::fulfillment(escrow::fb1),
            fee(baseFee * 150),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        auto const finished = env.le(bobLine);
        if (!BEAST_EXPECT(finished))
            return;
        BEAST_EXPECT(env.balance(bob, USD) == USD(40));
        BEAST_EXPECT(ownerCount(env, bob) == 0);
        BEAST_EXPECT(!finished->isFlag(bobHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceNFTokenAcceptCurrent()
    {
        testcase("TrustLine current — NFToken AcceptOffer creates positive balance without reserve");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const seller{"seller"};
        Account const buyer{"buyer"};
        auto const USD = gw["USD"];
        bool const sellerHigh = seller.id() > gw.id();

        Env env{*this};
        env.fund(XRP(400'000), gw, seller, buyer);
        env.close();

        env(trust(seller, USD(1'000)), THISLINE);
        env(trust(buyer, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, seller, USD(100)), THISLINE);
        env(pay(gw, buyer, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(seller, USD(0)), THISLINE);
        env.close();
        env(pay(seller, gw, USD(100)), THISLINE);
        env.close();

        auto const sellerLine = keylet::line(seller, gw, to_currency("USD"));
        auto const cleared = env.le(sellerLine);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, seller) == 0);
        BEAST_EXPECT(!cleared->isFlag(sellerHigh ? lsfHighReserve : lsfLowReserve));

        auto const nftID = token::getNextID(env, seller, 0u, tfTransferable);
        env(token::mint(seller, 0u), txflags(tfTransferable), THISLINE);
        env.close();

        auto const sellIdx = keylet::nftoffer(seller, env.seq(seller)).key;
        env(token::createOffer(seller, nftID, USD(40)),
            txflags(tfSellNFToken),
            THISLINE);
        env.close();

        env(token::acceptSellOffer(buyer, sellIdx), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const settled = env.le(sellerLine);
        if (!BEAST_EXPECT(settled))
            return;
        env.require(balance(seller, USD(40)));
        BEAST_EXPECT(ownerCount(env, seller) == 0);
        BEAST_EXPECT(!settled->isFlag(sellerHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceNFTokenBrokerFeeCurrent()
    {
        testcase("TrustLine current — NFToken broker fee creates positive balance without reserve");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const seller{"seller"};
        Account const buyer{"buyer"};
        Account const broker{"broker"};
        auto const USD = gw["USD"];
        bool const brokerHigh = broker.id() > gw.id();

        Env env{*this};
        env.fund(XRP(400'000), gw, seller, buyer, broker);
        env.close();

        env(trust(seller, USD(1'000)), THISLINE);
        env(trust(buyer, USD(1'000)), THISLINE);
        env(trust(broker, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, buyer, USD(100)), THISLINE);
        env(pay(gw, broker, USD(100)), THISLINE);
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(broker, USD(0)), THISLINE);
        env.close();
        env(pay(broker, gw, USD(100)), THISLINE);
        env.close();

        auto const brokerLine = keylet::line(broker, gw, to_currency("USD"));
        auto const cleared = env.le(brokerLine);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, broker) == 0);
        BEAST_EXPECT(!cleared->isFlag(brokerHigh ? lsfHighReserve : lsfLowReserve));

        auto const nftID = token::getNextID(env, seller, 0u, tfTransferable);
        env(token::mint(seller, 0u), txflags(tfTransferable), THISLINE);
        env.close();

        auto const sellIdx = keylet::nftoffer(seller, env.seq(seller)).key;
        env(token::createOffer(seller, nftID, USD(40)),
            txflags(tfSellNFToken),
            THISLINE);
        env.close();

        auto const buyIdx = keylet::nftoffer(buyer, env.seq(buyer)).key;
        env(token::createOffer(buyer, nftID, USD(60)),
            token::owner(seller),
            THISLINE);
        env.close();

        env(token::brokerOffers(broker, buyIdx, sellIdx),
            token::brokerFee(USD(10)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        auto const paid = env.le(brokerLine);
        if (!BEAST_EXPECT(paid))
            return;
        env.require(balance(broker, USD(10)));
        BEAST_EXPECT(ownerCount(env, broker) == 0);
        BEAST_EXPECT(!paid->isFlag(brokerHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testTrustlinePositiveBalanceAMMWithdrawCurrent()
    {
        testcase("TrustLine current — AMMWithdraw creates positive balance without reserve");
        using namespace jtx;

        FeatureBitset const features = testable_amendments() | featureAMM;
        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];
        bool const aliceHigh = alice.id() > gw.id();

        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice, bob);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(bob, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, bob, USD(1'000)), THISLINE);
        env.close();

        AMM amm{env, bob, XRP(1'000), USD(100)};
        env.close();

        env(fclear(gw, asfDefaultRipple), THISLINE);
        env.close();

        env(trust(alice, USD(0)), THISLINE);
        env.close();
        env(pay(alice, gw, USD(100)), THISLINE);
        env.close();

        auto const aliceLine = keylet::line(alice, gw, to_currency("USD"));
        auto const cleared = env.le(aliceLine);
        if (!BEAST_EXPECT(cleared))
            return;
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!cleared->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));

        auto const aliceTokens = amm.deposit(alice, XRP(100));
        env.close();
        BEAST_EXPECT(amm.getLPTokensBalance(alice.id()) > IOUAmount{0});

        amm.withdraw(alice, aliceTokens, USD(0), std::nullopt, ter(tesSUCCESS));
        env.close();

        auto const withdrawn = env.le(aliceLine);
        if (!BEAST_EXPECT(withdrawn))
            return;
        BEAST_EXPECT(env.balance(alice, USD).number() > beast::zero);
        BEAST_EXPECT(ownerCount(env, alice) == 0);
        BEAST_EXPECT(!withdrawn->isFlag(aliceHigh ? lsfHighReserve : lsfLowReserve));
    }

    void
    testDisallowIncomingTrustlineOfferCreateCurrent()
    {
        testcase("TrustLine current — OfferCreate bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];

        Env env{*this};
        env.fund(XRP(400'000), gw, alice, bob);
        env.close();

        env(trust(alice, USD(100)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(50)), THISLINE);
        env.close();

        env(offer(alice, XRP(4'000), USD(40)), THISLINE);
        env.close();
        env.require(offers(alice, 1));

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const bobLine = keylet::line(bob, gw, to_currency("USD"));
        BEAST_EXPECT(!env.le(bobLine));

        // Current 3.1.3 blocks direct TrustSet-created incoming trustlines
        // but OfferCreate can still cross into the issuer's asset and create
        // the trustline for the taker.
        env(offer(bob, USD(40), XRP(4'000)), THISLINE);
        env.close();

        env.require(offers(alice, 0));
        env.require(offers(bob, 0));
        env.require(balance(bob, USD(40)));
        BEAST_EXPECT(env.le(bobLine));
    }

    void
    testDisallowIncomingTrustlineNFTokenAcceptCurrent()
    {
        testcase("NFToken current — AcceptOffer bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const seller{"seller"};
        Account const buyer{"buyer"};
        auto const USD = gw["USD"];

        Env env{*this};
        env.fund(XRP(400'000), gw, seller, buyer);
        env.close();

        env(trust(buyer, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, buyer, USD(100)), THISLINE);
        env.close();

        auto const nftID = token::getNextID(env, seller, 0u, tfTransferable);
        env(token::mint(seller, 0u), txflags(tfTransferable), THISLINE);
        env.close();

        auto const sellIdx = keylet::nftoffer(seller, env.seq(seller)).key;
        env(token::createOffer(seller, nftID, USD(40)),
            txflags(tfSellNFToken),
            THISLINE);
        env.close();

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const sellerLine = keylet::line(seller, gw, to_currency("USD"));
        BEAST_EXPECT(!env.le(sellerLine));

        // Control: direct TrustSet honors asfDisallowIncomingTrustline.
        env(trust(seller, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(sellerLine));

        // Current 3.1.3 NFTokenAcceptOffer checks RequireAuth and deep-freeze,
        // but not DisallowIncomingTrustline, before paying the seller.
        env(token::acceptSellOffer(buyer, sellIdx), ter(tesSUCCESS), THISLINE);
        env.close();

        env.require(balance(seller, USD(40)));
        BEAST_EXPECT(env.le(sellerLine));
    }

    void
    testDisallowIncomingTrustlineNFTokenBrokerFeeCurrent()
    {
        testcase("NFToken current — broker fee bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const seller{"seller"};
        Account const buyer{"buyer"};
        Account const broker{"broker"};
        auto const USD = gw["USD"];

        Env env{*this};
        env.fund(XRP(400'000), gw, seller, buyer, broker);
        env.close();

        env(trust(buyer, USD(1'000)), THISLINE);
        env(trust(seller, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, buyer, USD(100)), THISLINE);
        env.close();

        auto const nftID = token::getNextID(env, seller, 0u, tfTransferable);
        env(token::mint(seller, 0u), txflags(tfTransferable), THISLINE);
        env.close();

        auto const sellIdx = keylet::nftoffer(seller, env.seq(seller)).key;
        env(token::createOffer(seller, nftID, USD(40)),
            txflags(tfSellNFToken),
            THISLINE);
        env.close();

        auto const buyIdx = keylet::nftoffer(buyer, env.seq(buyer)).key;
        env(token::createOffer(buyer, nftID, USD(60)),
            token::owner(seller),
            THISLINE);
        env.close();

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const brokerLine = keylet::line(broker, gw, to_currency("USD"));
        env(trust(broker, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(brokerLine));

        env(token::brokerOffers(broker, buyIdx, sellIdx),
            token::brokerFee(USD(10)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        env.require(balance(broker, USD(10)));
        BEAST_EXPECT(env.le(brokerLine));
    }

    void
    testDisallowIncomingTrustlineCheckCashCurrent()
    {
        testcase("CheckCash current — bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureCheckCashMakesTrustLine |
            featureDisallowIncoming;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice, bob);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        auto const checkID = keylet::check(alice.id(), env.seq(alice)).key;
        env(check::create(alice, bob, USD(40)), THISLINE);
        env.close();

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const bobLine = keylet::line(bob, gw, to_currency("USD"));
        BEAST_EXPECT(!env.le(bobLine));

        // Control: direct TrustSet honors asfDisallowIncomingTrustline.
        env(trust(bob, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(bobLine));

        // Current CheckCash auto-creates the trustline after checking
        // RequireAuth and freeze, but without checking DisallowIncoming.
        env(check::cash(bob, checkID, USD(40)), ter(tesSUCCESS), THISLINE);
        env.close();

        env.require(balance(bob, USD(40)));
        BEAST_EXPECT(env.le(bobLine));
    }

    void
    testDisallowIncomingTrustlineEscrowFinishCurrent()
    {
        testcase("TokenEscrow current — Finish bypasses DisallowIncomingTrustline");
        using namespace jtx;
        using namespace std::chrono_literals;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureTokenEscrow | featureDisallowIncoming;
        Env env{*this, features};
        auto const baseFee = env.current()->fees().base;
        env.fund(XRP(400'000), gw, alice, bob);
        env(fset(gw, asfAllowTrustLineLocking), THISLINE);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        auto const escrowSeq = env.seq(alice);
        env(escrow::create(alice, bob, USD(40)),
            escrow::condition(escrow::cb1),
            escrow::finish_time(env.now() + 1s),
            fee(baseFee * 150),
            THISLINE);
        env.close();

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const bobLine = keylet::line(bob, gw, to_currency("USD"));
        BEAST_EXPECT(!env.le(bobLine));

        // Control: direct TrustSet honors asfDisallowIncomingTrustline.
        env(trust(bob, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(bobLine));

        // Current EscrowFinish auto-creates the destination trustline after
        // checking RequireAuth and deep-freeze, but without checking
        // DisallowIncoming.
        env(escrow::finish(bob, alice, escrowSeq),
            escrow::condition(escrow::cb1),
            escrow::fulfillment(escrow::fb1),
            fee(baseFee * 150),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        env.require(balance(bob, USD(40)));
        BEAST_EXPECT(env.le(bobLine));
    }

    void
    testDisallowIncomingTrustlineAMMWithdrawCurrent()
    {
        testcase("AMM current — Withdraw bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureAMM | featureDisallowIncoming;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        AMM amm{env, alice, XRP(1'000), USD(100)};

        auto const aliceLine = keylet::line(alice, gw, to_currency("USD"));
        env(trust(alice, USD(0)), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(aliceLine));

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        env(trust(alice, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(aliceLine));

        amm.withdraw(alice, USD(10), std::nullopt, std::nullopt, ter(tesSUCCESS));
        env.close();

        BEAST_EXPECT(env.balance(alice, USD).number() > beast::zero);
        BEAST_EXPECT(env.le(aliceLine));
    }

    void
    testDisallowIncomingTrustlineAMMCreateCurrent()
    {
        testcase("AMM current — Create bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureAMM | featureDisallowIncoming;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env.close();

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const probe{"probe"};
        env.fund(XRP(400'000), probe);
        env.close();
        env(trust(probe, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();

        AMM amm{env, alice, XRP(1'000), USD(100)};

        auto const ammLine = keylet::line(amm.ammAccount(), gw, to_currency("USD"));
        BEAST_EXPECT(env.le(ammLine));

        auto const [amount, amount2, _] = amm.balances();
        BEAST_EXPECT(amount == XRP(1'000) || amount2 == XRP(1'000));
        BEAST_EXPECT(amount.issue() == USD.issue() || amount2.issue() == USD.issue());
    }

    void
    testDisallowIncomingTrustlineAMMEmptyDepositCurrent()
    {
        testcase("AMM current — Empty deposit bypasses DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureAMM | featureDisallowIncoming;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice, bob);
        env.close();

        env(trust(alice, USD(1'000)), THISLINE);
        env(trust(bob, USD(1'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(100)), THISLINE);
        env(pay(gw, bob, USD(100)), THISLINE);
        env.close();

        AMM amm{env, alice, XRP(1'000), USD(100)};
        for (auto i = 0u; i < maxDeletableAMMTrustLines + 10; ++i)
        {
            Account const holder{std::string{"emptyammline"} + std::to_string(i)};
            env.fund(XRP(1'000), holder);
            env(trust(holder, STAmount{amm.lptIssue(), 10'000}));
            env.close();
        }

        amm.withdrawAll(alice);
        BEAST_EXPECT(amm.ammExists());
        BEAST_EXPECT(amm.getLPTokensBalance() == IOUAmount{0});

        auto const ammLine = keylet::line(amm.ammAccount(), gw, to_currency("USD"));
        // This must be absent for this to be a disallow-incoming creation path.
        BEAST_EXPECT(!env.le(ammLine));

        env(fset(gw, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        auto const probe{"probe"};
        env.fund(XRP(400'000), probe);
        env.close();
        env(trust(probe, USD(1'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();

        amm.deposit(
            bob,
            std::nullopt,
            XRP(1'000),
            USD(100),
            std::nullopt,
            tfTwoAssetIfEmpty,
            std::nullopt,
            std::nullopt,
            0);

        BEAST_EXPECT(env.le(ammLine));
    }

    void
    testDisallowIncomingTrustlineAMMClawbackPairedAssetCurrent()
    {
        testcase(
            "AMM current — Clawback returns paired asset through DisallowIncomingTrustline");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const gw2{"gateway2"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];
        auto const EUR = gw2["EUR"];

        FeatureBitset const features = testable_amendments() | featureAMM |
            featureAMMClawback | featureDisallowIncoming;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, gw2, alice);
        env.close();

        env(fset(gw, asfAllowTrustLineClawback), THISLINE);
        env.close();

        env(trust(alice, USD(10'000)), THISLINE);
        env(trust(alice, EUR(10'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(3'000)), THISLINE);
        env(pay(gw2, alice, EUR(1'000)), THISLINE);
        env.close();

        AMM amm{env, alice, EUR(1'000), USD(2'000)};
        env.close();

        auto const aliceEurLine = keylet::line(alice, gw2, to_currency("EUR"));
        env(trust(alice, EUR(0)), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(aliceEurLine));

        env(fset(gw2, asfDisallowIncomingTrustline), THISLINE);
        env.close();

        env(trust(alice, EUR(10'000)), ter(tecNO_PERMISSION), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(aliceEurLine));

        env(amm::ammClawback(gw, alice, USD, EUR, USD(1'000)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        BEAST_EXPECT(env.le(aliceEurLine));
        BEAST_EXPECT(env.balance(alice, EUR).number() > beast::zero);
    }

    void
    testAMMClawbackDepositAuthPairedAssetCandidate()
    {
        testcase("AMM current — Clawback bypasses DepositAuth paired asset");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const gw2{"gateway2"};
        Account const alice{"alice"};
        auto const USD = gw["USD"];
        auto const EUR = gw2["EUR"];

        FeatureBitset const features = testable_amendments() | featureAMM |
            featureAMMClawback | featureDepositAuth;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, gw2, alice);
        env.close();

        env(fset(gw, asfAllowTrustLineClawback), THISLINE);
        env.close();

        env(trust(alice, USD(10'000)), THISLINE);
        env(trust(alice, EUR(10'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(3'000)), THISLINE);
        env(pay(gw2, alice, EUR(1'000)), THISLINE);
        env.close();

        AMM amm{env, alice, EUR(1'000), USD(2'000)};
        env.close();

        auto const aliceEurLine = keylet::line(alice, gw2, to_currency("EUR"));
        env(trust(alice, EUR(0)), THISLINE);
        env.close();
        BEAST_EXPECT(!env.le(aliceEurLine));

        env(fset(alice, asfDepositAuth), THISLINE);
        env.close();

        env(pay(gw2, alice, EUR(1)), ter(tecNO_PERMISSION), THISLINE);
        BEAST_EXPECT(!env.le(aliceEurLine));

        env(amm::ammClawback(gw, alice, USD, EUR, USD(1'000)),
            ter(tesSUCCESS),
            THISLINE);
        env.close();

        BEAST_EXPECT(env.le(aliceEurLine));
        BEAST_EXPECT(env.balance(alice, EUR).number() > beast::zero);
    }

    void
    testAMMBidDepositAuthRefundCandidate()
    {
        testcase("AMM current — Bid refund bypasses DepositAuth");
        using namespace jtx;

        Account const gw{"gateway"};
        Account const alice{"alice"};
        Account const bob{"bob"};
        auto const USD = gw["USD"];

        FeatureBitset const features =
            testable_amendments() | featureAMM | featureDepositAuth;
        Env env{*this, features};
        env.fund(XRP(400'000), gw, alice, bob);
        env.close();

        env(trust(alice, USD(20'000'000)), THISLINE);
        env(trust(bob, USD(20'000'000)), THISLINE);
        env.close();
        env(pay(gw, alice, USD(10'000'000)), THISLINE);
        env(pay(gw, bob, USD(10'000'000)), THISLINE);
        env.close();

        AMM amm{env, gw, XRP(10), USD(1'000)};
        auto const lpIssue = amm.lptIssue();
        env.trust(STAmount{lpIssue, 10'000'000}, alice);
        env.trust(STAmount{lpIssue, 10'000'000}, bob);
        env.close();

        amm.deposit(alice, 1'000'000);
        amm.deposit(bob, 1'000'000);
        env.close();

        env(amm.bid({.account = alice, .bidMin = 100}), THISLINE);
        env.close();

        env(fset(alice, asfDepositAuth), THISLINE);
        env.close();

        env(pay(bob, alice, STAmount{lpIssue, 1}),
            ter(tecNO_PERMISSION),
            THISLINE);
        env.close();

        auto const before = amm.getLPTokensBalance(alice.id());
        env(amm.bid({.account = bob}), ter(tesSUCCESS), THISLINE);
        env.close();

        BEAST_EXPECT(amm.getLPTokensBalance(alice.id()) > before);
    }

    void
    testDelegatedMPTGranularMutationCurrent()
    {
        testcase("Delegate current — MPT granular lock permission mutates issuance fields");
        using namespace jtx;

        Account const issuer{"issuer"};
        Account const delegate{"delegate"};

        Env env{*this};
        env.fund(XRP(100'000), issuer, delegate);
        env.close();

        MPTTester mpt{env, issuer, mptInitNoFund};
        mpt.create(
            {.flags = tfMPTCanLock | tfMPTCanTransfer,
             .mutableFlags =
                 tmfMPTCanMutateMetadata | tmfMPTCanMutateTransferFee});

        auto const issuanceKey = keylet::mptIssuance(mpt.issuanceID());
        auto const before = env.le(issuanceKey);
        if (!BEAST_EXPECT(before))
            return;
        BEAST_EXPECT(!before->isFieldPresent(sfMPTokenMetadata));
        BEAST_EXPECT(!before->isFieldPresent(sfTransferFee));

        env(delegate::set(issuer, delegate, {"MPTokenIssuanceLock"}));
        env.close();

        // Current 3.1.3 authorizes this because
        // MPTokenIssuanceSet::checkPermission only inspects lock/unlock flags.
        // A granular lock delegate can therefore mutate issuance metadata and
        // transfer fee without transaction-level MPTokenIssuanceSet authority.
        mpt.set(
            {.account = issuer,
             .transferFee = 100,
             .metadata = "delegate changed issuance",
             .delegate = delegate});

        BEAST_EXPECT(mpt.checkTransferFee(100));
        BEAST_EXPECT(mpt.checkMetadata("delegate changed issuance"));
    }

    void
    testDelegatedEmptyAccountSetCurrent()
    {
        testcase("Delegate current — empty AccountSet with unrelated permission consumes principal sequence");
        using namespace jtx;

        Account const alice{"alice"};
        Account const bob{"bob"};
        Env env(*this);

        env.fund(XRP(10'000), alice, bob);
        env.close();

        env(delegate::set(alice, bob, {"Payment"}), ter(tesSUCCESS), THISLINE);
        env.close();

        auto const aliceSeqBefore = env.seq(alice);
        auto const bobSeqBefore = env.seq(bob);
        auto const aliceBalanceBefore = env.balance(alice);
        auto const bobBalanceBefore = env.balance(bob);

        env(noop(alice), delegate::as(bob), ter(tesSUCCESS), THISLINE);
        env.close();

        BEAST_EXPECT(env.seq(alice) == aliceSeqBefore + 1);
        BEAST_EXPECT(env.seq(bob) == bobSeqBefore);
        BEAST_EXPECT(env.balance(alice) == aliceBalanceBefore);
        BEAST_EXPECT(env.balance(bob) < bobBalanceBefore);
    }

    void
    testBatchSignerOuterAccountReplayCurrent()
    {
        testcase("Batch current — batch signer signatures replay across outer account");
        using namespace jtx;

        Env env{*this};
        Account const alice{"alice"};
        Account const bob{"bob"};
        Account const carol{"carol"};
        Account const eve{"eve"};

        env.fund(XRP(10'000), alice, bob, carol, eve);
        env.close();

        auto const aliceSeq = env.seq(alice);
        auto const bobSeq = env.seq(bob);
        auto const carolSeq = env.seq(carol);
        auto const batchFee1 = batch::calcBatchFee(env, 2, 2);

        auto jt1 = env.jt(
            batch::outer(alice, aliceSeq, batchFee1, tfOnlyOne),
            batch::inner(pay(bob, alice, XRP(100)), bobSeq),
            batch::inner(pay(carol, alice, XRP(50)), carolSeq),
            batch::sig(bob, carol));
        auto const capturedSigners = jt1.jv[sfBatchSigners.jsonName];

        env(jt1, ter(tesSUCCESS), THISLINE);
        env.close();

        BEAST_EXPECT(env.seq(bob) == bobSeq + 1);
        BEAST_EXPECT(env.seq(carol) == carolSeq);

        auto const carolBalanceBeforeReplay = env.balance(carol);
        auto const eveSeq = env.seq(eve);
        auto const batchFee2 = batch::calcBatchFee(env, 2, 2);

        auto jt2 = env.jtnofill(
            batch::outer(eve, eveSeq, batchFee2, tfOnlyOne),
            batch::inner(pay(bob, alice, XRP(100)), bobSeq),
            batch::inner(pay(carol, alice, XRP(50)), carolSeq));
        jt2.jv[sfBatchSigners.jsonName] = capturedSigners;

        // Current 3.1.3 signs BatchSigners over flags and inner transaction
        // IDs only. Later upstream fix 7618b726b adds the outer account and
        // sequence to the signed message, which turns this replay into
        // temBAD_SIGNATURE.
        env(jt2.jv, ter(tesSUCCESS), THISLINE);
        env.close();

        BEAST_EXPECT(env.seq(carol) == carolSeq + 1);
        BEAST_EXPECT(env.balance(carol) < carolBalanceBeforeReplay);
    }

    void
    testInvariantBoolOverwritePreFix()
    {
        testcase("Invariant pre-fix — later good entries hide earlier bad entries");
        using namespace jtx;

        auto const finalize = [](auto& invariant, Env& env) {
            return invariant.finalize(
                STTx{ttACCOUNT_SET, [](STObject&) {}},
                tesSUCCESS,
                XRPAmount{},
                *env.current(),
                env.app().journal("OpenP0Repro"));
        };

        auto const runXrpTrustLine = [&](FeatureBitset features) {
            Env env{*this, features};
            CurrentTransactionRulesGuard const rg(env.current()->rules());
            Account const a1{"A1"};
            Account const a2{"A2"};
            Account const a3{"A3"};

            auto const k1 = keylet::line(a1, a2, a1["AAA"].currency);
            auto const k2 = keylet::line(a1, a3, a1["ZZZ"].currency);
            bool const k1First = k1.key < k2.key;
            auto const& badKey = k1First ? k1 : k2;
            auto const& goodKey = k1First ? k2 : k1;
            char const* const goodCurrency = k1First ? "ZZZ" : "AAA";

            auto const bad = std::make_shared<SLE>(badKey);
            bad->setFieldAmount(sfLowLimit, STAmount{xrpIssue(), 0});
            bad->setFieldAmount(sfHighLimit, a1["BAD"](0));

            auto const good = std::make_shared<SLE>(goodKey);
            good->setFieldAmount(sfLowLimit, a1[goodCurrency](0));
            good->setFieldAmount(sfHighLimit, a1[goodCurrency](0));

            NoXRPTrustLines invariant;
            invariant.visitEntry(false, {}, bad);
            invariant.visitEntry(false, {}, good);
            return finalize(invariant, env);
        };

        auto const runDeepFreeze = [&](FeatureBitset features) {
            Env env{*this, features};
            CurrentTransactionRulesGuard const rg(env.current()->rules());
            Account const a1{"A1"};
            Account const a2{"A2"};
            Account const a3{"A3"};

            auto const k1 = keylet::line(a1, a2, a1["USD"].currency);
            auto const k2 = keylet::line(a1, a3, a1["EUR"].currency);
            bool const k1First = k1.key < k2.key;
            auto const& badKey = k1First ? k1 : k2;
            auto const& goodKey = k1First ? k2 : k1;

            auto const bad = std::make_shared<SLE>(badKey);
            bad->setFieldAmount(
                sfLowLimit, STAmount(Issue(a1["USD"].currency, a1.id()), 0));
            bad->setFieldAmount(
                sfHighLimit, STAmount(Issue(a1["USD"].currency, a1.id()), 0));
            bad->setFieldU32(sfFlags, lsfLowDeepFreeze);

            auto const good = std::make_shared<SLE>(goodKey);
            good->setFieldAmount(
                sfLowLimit, STAmount(Issue(a1["EUR"].currency, a1.id()), 0));
            good->setFieldAmount(
                sfHighLimit, STAmount(Issue(a1["EUR"].currency, a1.id()), 0));
            good->setFieldU32(sfFlags, 0);

            NoDeepFreezeTrustLinesWithoutFreeze invariant;
            invariant.visitEntry(false, {}, bad);
            invariant.visitEntry(false, {}, good);
            return finalize(invariant, env);
        };

        auto const runMptIssuance = [&](FeatureBitset features) {
            Env env{*this, features};
            CurrentTransactionRulesGuard const rg(env.current()->rules());
            MPTIssue const mpt{makeMptID(1, AccountID(0x4985601))};

            auto const issuance =
                std::make_shared<SLE>(keylet::mptIssuance(mpt.getMptID()));
            issuance->setFieldU64(sfOutstandingAmount, maxMPTokenAmount + 1);
            issuance->setFieldU64(sfLockedAmount, 10);

            NoZeroEscrow invariant;
            invariant.visitEntry(false, {}, issuance);
            return finalize(invariant, env);
        };

        FeatureBitset const preFix = testable_amendments() - fixCleanup3_1_3;
        FeatureBitset const withFix = testable_amendments() | fixCleanup3_1_3;

        BEAST_EXPECT(runXrpTrustLine(preFix));
        BEAST_EXPECT(!runXrpTrustLine(withFix));
        BEAST_EXPECT(runDeepFreeze(preFix));
        BEAST_EXPECT(!runDeepFreeze(withFix));
        BEAST_EXPECT(runMptIssuance(preFix));
        BEAST_EXPECT(!runMptIssuance(withFix));
    }

public:
    void
    run() override
    {
        testF33_coverWithdrawRegularFreezeOnly();
        testF33_controlDeepFreezeBlocks();
        testF35_brokerDeleteRegularFrozenOwner();
        testF39_coverDepositRegularFrozenBrokerPseudo();
        testF37_loanSetRegularFrozenBrokerOwnerOriginationFee();
        testF36_loanPayRegularFrozenBrokerOwnerServiceFee();
        testF38_loanPayRegularFrozenVaultPseudo();
        testF310_loanPayRegularFrozenBrokerPseudoFallbackFee();
        testF61_setTrustMissingIssuerSavOnControl();
        testDIDDirFullNoPartialMutation();
        testBatchAllOrNothingExpiredCredentialRollback();
        testPermissionedDomainTicketSequencePrefixCollision();
        testExpiredCredentialDeleteFailurePreFix();
        testPermissionedDexEmptyAdditionalBooksPreFix();
        testMPTMultiSendMaximumAmountPreFix();
        testVaultWithdrawShareLimitBypassPreFix();
        testVaultShareMPTLockedEscrowDeletionPreFix();
        testVaultClawbackZeroAmountUnclampedPreFix();
        testVaultShareMPTUnderlyingTransferRestrictionCurrent();
        testLoanPayFeeCapPreFix();
        testLoanBrokerCoverPrecisionCurrent();
        testLoanMinimumCoverScaleInconsistencyCurrent();
        testLoanBrokerDeleteLockedMPTCoverCurrent();
        testVaultDepositOppositeLimitInternalCurrent();
        testEscrowCancelDeletedIOUTrustlineCurrent();
        testAMMStaleAuthAccountsAfterReinitCurrent();
        testDelegateeAccountDeleteLeavesStaleDelegateCurrent();
        testMPTDomainRequireAuthCanBeClearedCurrent();
        testNumberMaxRepCuspUpwardRoundsDownCurrent();
        testNumberDivisionUpwardRoundsDownCurrent();
        testMPTTransferRateLargeAmountOverflowCurrent();
        testDelegatePaymentFeeReserveCouplingCurrent();
        testDelegatedVaultCreateCurrent();
        testPermissionedDexHybridOfferQualityCurrent();
        testPermissionedDexCancelRegularOfferInvariantCurrent();
        testLoanPaymentFactorCancellationCurrent();
        testVaultWithdrawScaleBoundaryInvariantCurrent();
        testVaultIssuerDepositScaleBoundaryInvariantCurrent();
        testVaultSoleShareholderImpairedExitCurrent();
        testDelegateMultisignDelegatorRejectedCurrent();
        testMPTNonCanonicalIssuerPaymentCurrent();
        testMPTSTIssueLegacyWireOrderCurrent();
        testMPTLockedHolderUnauthorizeWithoutSavCurrent();
        testTrustlinePositiveBalanceNoOwnerReserveCurrent();
        testTrustlinePositiveBalanceOfferExistingOwnersCurrent();
        testTrustlinePositiveBalanceOfferReserveBoundaryCurrent();
        testTrustlinePositiveBalanceCheckCashCurrent();
        testTrustlinePositiveBalanceCheckCashExistingOwnersCurrent();
        testTrustlinePositiveBalanceCheckCashReserveBoundaryCurrent();
        testTrustlinePositiveBalanceTokenEscrowCurrent();
        testTrustlinePositiveBalanceNFTokenAcceptCurrent();
        testTrustlinePositiveBalanceNFTokenBrokerFeeCurrent();
        testTrustlinePositiveBalanceAMMWithdrawCurrent();
        testDisallowIncomingTrustlineOfferCreateCurrent();
        testDisallowIncomingTrustlineNFTokenAcceptCurrent();
        testDisallowIncomingTrustlineNFTokenBrokerFeeCurrent();
        testDisallowIncomingTrustlineCheckCashCurrent();
        testDisallowIncomingTrustlineEscrowFinishCurrent();
        testDisallowIncomingTrustlineAMMWithdrawCurrent();
        testDisallowIncomingTrustlineAMMCreateCurrent();
        testDisallowIncomingTrustlineAMMEmptyDepositCurrent();
        testDisallowIncomingTrustlineAMMClawbackPairedAssetCurrent();
        testAMMClawbackDepositAuthPairedAssetCandidate();
        testAMMBidDepositAuthRefundCandidate();
        testDelegatedMPTGranularMutationCurrent();
        testDelegatedEmptyAccountSetCurrent();
        testBatchSignerOuterAccountReplayCurrent();
        testInvariantBoolOverwritePreFix();
    }
};

BEAST_DEFINE_TESTSUITE(OpenP0Repro, tx, ripple);

class OpenP0ReproCrash_test : public beast::unit_test::suite
{
    void
    testF61_setTrustMissingIssuerNoSavCrash()
    {
        testcase("F6.1 negative control — missing issuer returns tecNO_DST");
        using namespace jtx;

        FeatureBitset const noAmmSavDisallowIncoming{
            (jtx::testable_amendments() | featureMPTokensV1 | featureLendingProtocol) -
            featureAMM - featureSingleAssetVault - featureDisallowIncoming};

        Account const ghostIssuer{"ghostIssuer"};
        Account const alice{"alice"};
        Env env(*this, noAmmSavDisallowIncoming);

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

BEAST_DEFINE_TESTSUITE(OpenP0ReproCrash, tx, ripple);

}  // namespace test
}  // namespace ripple
