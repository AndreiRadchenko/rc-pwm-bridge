#include "test_support.hpp"
#include "platform/startup_port.hpp"
#include <limits>
#include <string>
#include <vector>
using namespace bridge;
using namespace platform;
struct Clock : ClockPort { ClockReading reading{true,0}; ClockReading read() override { return reading; } };
struct Profile : ProfilePort { std::string& calls; bool ok=true; explicit Profile(std::string& s):calls(s){} bool matches() override { calls+='p'; return ok; } };
struct Output : OutputPort { std::string& calls; bool ok=true; explicit Output(std::string& s):calls(s){} OutputApplyResult apply_low() override { calls+='o'; return {ok}; } };
struct Setup : SetupPort { std::string& calls; bool transport=true, worker=true; bool installed=false; int installs=0; explicit Setup(std::string& s):calls(s){} bool transport_ready() override { calls+='t'; if (transport && !installed) { installed=true; ++installs; } return transport; } bool worker_ready() override { calls+='w'; return worker; } };
struct Diagnostic : DiagnosticPort { std::vector<DiagnosticSnapshot> events; void publish(const DiagnosticSnapshot& s) override { events.push_back(s); } };
struct Transport : TransportPort { int calls=0; size_t written=0; size_t write(const char*, size_t n) override { ++calls; return written < n ? written : n; } };
int main() {
 Checks c;
 StartupSupervisor a; CHECK(c, !a.snapshot().now_valid && a.snapshot().phase==StartupPhase::AwaitingAttempt);
 CHECK(c,a.tick(0)); a.result(10,{false,StartupError::Output}); CHECK(c,!a.tick(1000009)); CHECK(c,a.tick(1000010)); CHECK(c,!a.tick(1000010));
 a.result(2000010,{false,StartupError::Output}); CHECK(c,!a.tick(3000009)); CHECK(c,a.tick(4000000)); a.result(4000001,{true,StartupError::None}); CHECK(c,a.snapshot().phase==StartupPhase::ReadyDisabled); CHECK(c,!a.tick(9000000)); CHECK(c,!a.snapshot().work_permitted);
 StartupSupervisor b; for(int i=0;i<5;i++){ CHECK(c,b.tick(static_cast<uint64_t>(i)*1000000)); b.result(static_cast<uint64_t>(i)*1000000,{false,StartupError::Output}); CHECK(c,!b.snapshot().work_permitted); } CHECK(c,b.snapshot().phase==StartupPhase::Exhausted); CHECK(c,!b.tick(9000000));
 StartupSupervisor fresh; CHECK(c,fresh.snapshot().attempts_started==0); CHECK(c,fresh.tick(1)); CHECK(c,!fresh.tick(0)); CHECK(c,fresh.snapshot().phase==StartupPhase::ClockFault && !fresh.snapshot().now_valid && fresh.snapshot().now_us==1);
 StartupSupervisor over; CHECK(c,over.tick(UINT64_MAX-1)); over.result(UINT64_MAX-1,{false,StartupError::Output}); CHECK(c,over.snapshot().phase==StartupPhase::ClockFault);
 StartupSupervisor duplicate; CHECK(c,duplicate.tick(0)); duplicate.result(0,{true,StartupError::None}); duplicate.result(0,{true,StartupError::None}); CHECK(c,duplicate.snapshot().phase==StartupPhase::Exhausted && duplicate.snapshot().last_error==StartupError::ProtocolFault);
 std::string calls; Clock clock; Profile profile(calls); Output output(calls); Setup setup(calls); Diagnostic diag;
 StartupPolicy policy(clock,profile,output,setup,diag,123); CHECK(c,diag.events.size()==1); policy.poll(); CHECK(c,calls=="potw"); CHECK(c,policy.snapshot().phase==StartupPhase::ReadyDisabled); auto count=diag.events.size(); clock.reading.raw_us=10; policy.poll(); CHECK(c,diag.events.size()==count);
 Transport tx; DiagnosticConsumer consumer; CHECK(c,consumer.consume(diag.events.back(),tx)); CHECK(c,tx.calls==1); CHECK(c,!consumer.consume(diag.events.back(),tx)); CHECK(c,tx.calls==1); auto newer=diag.events.back(); ++newer.sequence; tx.written=1; CHECK(c,consumer.consume(newer,tx)); CHECK(c,!consumer.consume(newer,tx)); CHECK(c,tx.calls==2);
 calls.clear(); Clock bad; bad.reading={true,-1}; Diagnostic d2; StartupPolicy p2(bad,profile,output,setup,d2,1); p2.poll(); CHECK(c,calls.empty()); CHECK(c,p2.snapshot().phase==StartupPhase::ClockFault && !p2.snapshot().now_valid && p2.snapshot().now_us==0);
 calls.clear(); Clock fail; fail.reading={false,0}; Diagnostic d3; StartupPolicy p3(fail,profile,output,setup,d3,1); p3.poll(); CHECK(c,calls.empty() && p3.snapshot().phase==StartupPhase::ClockFault);
 calls.clear(); Clock good; profile.ok=false; Diagnostic d4; StartupPolicy p4(good,profile,output,setup,d4,1); p4.poll(); CHECK(c,calls=="p"); CHECK(c,p4.snapshot().last_error==StartupError::Profile);
 calls.clear(); profile.ok=true; output.ok=false; Diagnostic d5; StartupPolicy p5(good,profile,output,setup,d5,1); p5.poll(); CHECK(c,calls=="po"); CHECK(c,p5.snapshot().last_error==StartupError::Output);
 calls.clear(); output.ok=true; setup.transport=false; Diagnostic d6; StartupPolicy p6(good,profile,output,setup,d6,1); p6.poll(); CHECK(c,calls=="pot"); CHECK(c,p6.snapshot().last_error==StartupError::Transport);
 calls.clear(); setup.transport=true; setup.worker=false; Diagnostic d7; StartupPolicy p7(good,profile,output,setup,d7,1); p7.poll(); CHECK(c,calls=="potw"); CHECK(c,p7.snapshot().last_error==StartupError::Worker);
 calls.clear(); Clock later; later.reading={true,42}; Diagnostic d8; output.ok=false; StartupPolicy p8(later,profile,output,setup,d8,1); p8.poll(); later.reading={false,0}; p8.poll(); CHECK(c,p8.snapshot().phase==StartupPhase::ClockFault && !p8.snapshot().now_valid && p8.snapshot().now_us==42); CHECK(c,calls=="po");
 calls.clear(); Clock retry; Diagnostic d9; setup.worker=true; setup.transport=false; output.ok=true; StartupPolicy p9(retry,profile,output,setup,d9,1); p9.poll(); CHECK(c,calls=="pot"); retry.reading.raw_us=1000000; setup.transport=true; p9.poll(); CHECK(c,calls=="potpotw"); CHECK(c,p9.snapshot().phase==StartupPhase::ReadyDisabled && !p9.snapshot().work_permitted);
 calls.clear(); Clock again; Setup idempotent(calls); idempotent.worker=false; Diagnostic d10; StartupPolicy p10(again,profile,output,idempotent,d10,1); p10.poll(); CHECK(c,idempotent.installs==1); again.reading.raw_us=1000000; idempotent.worker=true; p10.poll(); CHECK(c,idempotent.installs==1 && p10.snapshot().phase==StartupPhase::ReadyDisabled); auto terminal_events=d10.events.size(); again.reading.raw_us=2000000; p10.poll(); CHECK(c,d10.events.size()==terminal_events); // Reconnect/polls do not reset the consumer marker.
 if(c.failures) std::fprintf(stderr,"%d failed checks\n",c.failures); return c.failures ? 1 : 0;
}
