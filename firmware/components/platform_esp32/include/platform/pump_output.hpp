#ifndef PLATFORM_PUMP_OUTPUT_HPP
#define PLATFORM_PUMP_OUTPUT_HPP
#include "platform/startup_port.hpp"
#include "bridge/pump_output.hpp"
namespace platform {
struct PumpHardwareConfig {
  uint32_t clock_hz=80000000, frequency_hz=1000;
  uint8_t pin=0,resolution=13,timer=0,channel=0,hpoint=0;
};
class PumpSdk {
public:
  virtual uint64_t now_us()=0;
  virtual uint32_t faults()=0;
  virtual int32_t invoke(bridge::PumpOperation,uint32_t,const PumpHardwareConfig&)=0;
  virtual ~PumpSdk()=default;
};
class EspPumpOutput final : public OutputPort {
public:
  EspPumpOutput(PumpSdk&, bool bench, bridge::OutputEvidence* evidence=nullptr);
  OutputApplyResult apply_low() override;
  bool initialize();
  bool apply(const bridge::OutputRequest&);
  void cleanup();
  bridge::PumpApplied applied() const { return applied_; }
  uint8_t reasons() const { return reasons_; }
  bool terminal() const { return terminal_; }
private:
  int32_t call(bridge::PumpOperation,uint32_t,bool nonzero=false);
  bool low();
  void fail(uint8_t);
  PumpSdk& sdk_;
  bridge::OutputEvidence* evidence_;
  PumpHardwareConfig config_{};
  uint64_t generation_=0,deadline_=0, fault_at_=0;
  uint8_t percent_=0,reasons_=0;
  bridge::PumpApplied applied_=bridge::PumpApplied::Unknown;
  uint64_t last_time_=0;
  bool fault_set_=false,gpio_low_latched_=false,initialized_=false,has_low_request_=false;
  bool bench_,configured_=false,channel_=false,terminal_=false,has_generation_=false;
};
int format_output_archive(const bridge::OutputEvidence&,unsigned,const DiagnosticSnapshot&,char*,size_t);
int format_window_archive(const bridge::CaptureWindowEvidence&,const bridge::OutputHeader&,unsigned,const DiagnosticSnapshot&,char*,size_t);
// Owned only by the diagnostics writer. A short write retries the same frame;
// a complete cycle rotates to the next available immutable archive.
class ArchiveCursor {
public:
  unsigned select(bool trace,bool output,bool window) {
    const bool available[]={trace,output,window};
    for(unsigned i=0;i<3;++i) {
      if(available[kind_])return kind_;
      kind_=(kind_+1)%3;index_=0;
    }
    return 3;
  }
  unsigned index() const { return index_; }
  void written(bool complete,unsigned records) {
    if(!complete)return;
    if(++index_ >= (kind_==2?5:records+2)) {index_=0;kind_=(kind_+1)%3;}
  }
private:
  unsigned kind_=0,index_=0;
};
#ifdef ESP_PLATFORM
class EspPumpSdk final : public PumpSdk {
public:
  typedef uint32_t (*FaultReader)(void*);
  uint64_t now_us() override;
  uint32_t faults() override;
  int32_t invoke(bridge::PumpOperation,uint32_t,const PumpHardwareConfig&) override;
  void observer(FaultReader f,void* p) { reader_=f; context_=p; }
private:
  FaultReader reader_=nullptr;
  void* context_=nullptr;
};
#endif
}
#endif
