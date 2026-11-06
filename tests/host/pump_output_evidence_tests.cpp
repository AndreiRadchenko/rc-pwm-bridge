#include "bridge/pump_output.hpp"
#include "platform/pump_output.hpp"
#include <cstring>
#include <fstream>
#include "test_support.hpp"
using namespace bridge;
int main(int argc,char** argv){Checks c;
 OutputEvidence e;e.identity(0x1234,10,PumpScenario::Stress);e.begin(10);
 for(unsigned i=0;i<64;++i)CHECK(c,e.append({0,3,25,1,0,0,i,i+1,0}));
 CHECK(c,e.header().count==64);CHECK(c,!e.append({}));CHECK(c,e.header().overflow);CHECK(c,e.record(63).seq==64);
 e.freeze();auto h=e.digest();CHECK(c,e.append({}));CHECK(c,e.digest()==h);
 OutputEvidence errors;CHECK(c,errors.append({0,5,0,0,16,-7,0,1,0}));errors.fallback(PumpOperation::GpioLow,0);errors.fallback(PumpOperation::GpioDirection,-9);
 CHECK(c,errors.header().first_error==-7&&errors.header().fallback_attempted&&errors.header().fallback_error==-9);
 CaptureWindowEvidence w;CaptureSnapshot s={};w.start(0);w.observe(s,5000000);
 PulseMeasurement p={4998000,5000000,1000,20000,1,1,CaptureStatus::Valid,true,true};w.sample(p,100,100);
 p.end_us=606000000;w.sample(p,100,100);CHECK(c,w.header().counts[0]==1);
 p.end_us=605999999;p.seq=3;w.sample(p,100,100);CHECK(c,w.header().counts[0]==2);
 w.close(s,605000000,true,1,1);CHECK(c,!w.frozen());
 w.close(s,606005999,true,1,1);CHECK(c,!w.frozen());
 w.close(s,606006000,false,1,1);CHECK(c,!w.frozen());
 w.close(s,606017000,true,4,2);CHECK(c,w.frozen());CHECK(c,!(w.header().flags&WindowIncomplete));
 auto digest=w.digest();w.sample(p,100,100);CHECK(c,w.postfreeze_invalid());CHECK(c,w.digest()==digest);
 CaptureWindowEvidence late;late.start(0);late.observe(s,5000000);late.close(s,606017001,false,0,0);CHECK(c,late.frozen()&&(late.header().flags&WindowClosureLate));
 CaptureWindowEvidence pre;pre.start(0);pre.observe(s,5000000);p.end_us=4999999;pre.sample(p,100,100);CHECK(c,pre.header().counts[0]==0);
 platform::DiagnosticSnapshot snapshot={};snapshot.has_output=true;snapshot.sequence=snapshot.profile_fingerprint=UINT64_MAX;
 auto& capture=snapshot.capture;capture.started_at_us=capture.measurement.start_us=capture.measurement.end_us=capture.max_rearm_us=capture.max_processing_us=capture.max_fault_lateness_us=capture.threshold_at_us=capture.reported_at_us=UINT64_MAX;
 capture.measurement.width_us=capture.measurement.period_us=UINT32_MAX;capture.measurement.status=CaptureStatus::Error;capture.phase=CaptureState::Resync;capture.control.calculated_duty_percent=100;capture.fault_bits=0x7ff;
 capture.counters={UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,true};
 snapshot.startup.phase=StartupPhase::ClockFault;snapshot.startup.attempts_started=5;snapshot.startup.last_error=StartupError::ProtocolFault;snapshot.pwm_low_api_applied=true;
 snapshot.output.mode=PumpMode::Bench;snapshot.output.phase=PumpPhase::EvidenceFrozen;snapshot.output.requested_percent=100;snapshot.output.bench_permission=true;snapshot.output.applied=PumpApplied::High;snapshot.output.reasons=255;
 platform::DiagnosticStats stats={UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT64_MAX,true};char buffer[385];
 int n=platform::format_rc04(snapshot,stats,buffer,sizeof(buffer));CHECK(c,n>0&&n<=383);CHECK(c,int(std::strlen(buffer))==n);CHECK(c,std::strstr(buffer," o=0 a=0 ")!=nullptr);
 CHECK(c,platform::format_rc04(snapshot,stats,buffer,sizeof(buffer),true)==384);CHECK(c,platform::format_rc04(snapshot,stats,buffer,10)==0);
 snapshot.output.requested_percent=101;CHECK(c,platform::format_rc04(snapshot,stats,buffer,sizeof(buffer))==0);snapshot.output.requested_percent=100;
 for(unsigned i=0;i<errors.header().count+2;++i){CHECK(c,platform::format_output_archive(errors,i,snapshot,buffer,sizeof(buffer))==0);} // mutable archives cannot be exported
 errors.freeze();for(unsigned i=0;i<errors.header().count+2;++i){int len=platform::format_output_archive(errors,i,snapshot,buffer,sizeof(buffer));CHECK(c,len>0&&len<=384);}
 for(unsigned i=0;i<5;++i){int len=platform::format_window_archive(w,e.header(),i,snapshot,buffer,sizeof(buffer));CHECK(c,len>0&&len<=384);}
 // Exercise the same writer cursor used by LocalDiagnostics: complete cycles,
 // zero/partial writes, missing reader, late publication and rotation.
 platform::ArchiveCursor cursor;
 CHECK(c,cursor.select(false,false,false)==3);
 for(unsigned cycle=0;cycle<2;++cycle) {
   for(unsigned i=0;i<4;++i) {
     CHECK(c,cursor.select(false,true,true)==1&&cursor.index()==i);
     cursor.written(false,2);CHECK(c,cursor.index()==i);
     cursor.written(true,2);
   }
   for(unsigned i=0;i<5;++i) {
     CHECK(c,cursor.select(false,true,true)==2&&cursor.index()==i);
     cursor.written(true,0);
   }
 }
 platform::ArchiveCursor trace_cursor;
 for(unsigned i=0;i<130;++i){CHECK(c,trace_cursor.select(true,true,false)==0&&trace_cursor.index()==i);trace_cursor.written(true,128);}
 CHECK(c,trace_cursor.select(true,true,false)==1&&trace_cursor.index()==0);
 if(argc==3&&std::strcmp(argv[1],"--emit-fixture")==0) {
   OutputEvidence fixture;fixture.identity(0x1234,10,PumpScenario::Stress);fixture.begin(0);
   fixture.append({0,5,0,1,0,0,5000000,5000001,5000000});fixture.freeze();
   platform::DiagnosticSnapshot clean={};platform::ArchiveCursor writer;
   std::ofstream file(argv[2]);
   for(unsigned frame=0;frame<16;++frame) {
     const unsigned kind=writer.select(false,true,true);
     int len=kind==1?platform::format_output_archive(fixture,writer.index(),clean,buffer,sizeof(buffer)):platform::format_window_archive(w,fixture.header(),writer.index(),clean,buffer,sizeof(buffer));
     CHECK(c,len>0&&len<=384);file.write(buffer,len);writer.written(true,fixture.header().count);
   }
   CHECK(c,bool(file));
 }
 return c.failures?1:0;}
