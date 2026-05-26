// Repro snippet for F3.3 — regular-freeze-only destination on LoanBrokerCoverWithdraw
//
// Drop into src/test/app/ (e.g. LoanBroker_test.cpp) or a new FreezeBypassRepro_test.cpp
// registered in the test runner, then:
//   ./rippled --unittest FreezeBypassRepro
//
// Expected TODAY (bug): tesSUCCESS — cover delivered to regular-frozen dest
// Expected AFTER FIX:  tecFROZEN
//
// Requires: rippled built with XLS-66 lending + dev env (jtx). NO mainnet XRP.

#include <test/jtx.h>
#include <test/jtx/Account.h>
#include <test/jtx/trust.h>
#include <test/jtx/Env.h>
#include <test/jtx/amt.h>

namespace ripple {
namespace test {

class FreezeBypassRepro_test : public beast::unit_test::suite
{
    void
    testCoverWithdrawRegularFreezeOnly()
    {
        testcase("F3.3 regular-freeze-only cover withdraw destination");
        using namespace jtx;

        Account issuer{"issuer"};
        Account brokerOwner{"brokerOwner"};
        Account dest{"dest"};
        Env env(*this, all);

        auto const asset = issuer["USD"];

        env.fund(XRP(10'000), issuer, brokerOwner, dest);
        env(trust(dest, asset(1'000)));
        env(trust(brokerOwner, asset(1'000)));

        // ... create vault + loan broker (see LoanBroker_test.cpp helpers) ...

        // Regular freeze ONLY — not deep freeze
        env(trust(issuer, asset(0), dest, tfSetFreeze));
        env.close();

        // BUG: succeeds today; should be ter(tecFROZEN)
        env(coverWithdraw(brokerOwner, brokerKeylet.key, asset(10)),
            destination(dest),
            ter(tesSUCCESS));  // change to ter(tecFROZEN) after fix
    }

public:
    void
    run() override
    {
        testCoverWithdrawRegularFreezeOnly();
    }
};

BEAST_DEFINE_TESTSUITE(FreezeBypassRepro, tx, ripple);

}  // namespace test
}  // namespace ripple
