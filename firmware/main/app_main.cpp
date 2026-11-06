#include "platform/startup_port.hpp"
#include "platform/safe_output.hpp"
#include "platform/pump_output.hpp"
#include "platform/monotonic_clock.hpp"
#include "platform/local_diagnostics.hpp"
#include "board_profile.hpp"
#include "platform/rc_capture.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_system.h"
#if defined(CONFIG_BRIDGE_PUMP04_BENCH) && CONFIG_BRIDGE_PUMP04_SCENARIO == 3
#if !defined(CONFIG_BRIDGE_PUMP04_TRACE_STEP) || CONFIG_BRIDGE_PUMP04_TRACE_STEP < 0 || CONFIG_BRIDGE_PUMP04_TRACE_STEP > 5
#error "Accuracy requires an explicit valid trace step"
#endif
#endif
extern "C" void app_main() {
    static platform::EspMonotonicClock clock;
    static platform::EspBoardProfile profile;
    static platform::EspPumpSdk sdk;
#ifdef CONFIG_BRIDGE_PUMP04_BENCH
    const bridge::BenchConfig config=bridge::BenchConfig::bench(bridge::PumpScenario(CONFIG_BRIDGE_PUMP04_SCENARIO),
#ifdef CONFIG_BRIDGE_PUMP04_TRACE_STEP
        CONFIG_BRIDGE_PUMP04_TRACE_STEP
#else
        0
#endif
    );
    static bridge::OutputEvidence evidence;
    evidence.identity(BRIDGE_PUMP04_BUILD_FINGERPRINT,uint64_t(clock.read().raw_us),config.scenario);
    bridge::OutputEvidence* output_evidence=&evidence;
#if CONFIG_BRIDGE_PUMP04_SCENARIO == 2
    static bridge::CaptureWindowEvidence window;
    bridge::CaptureWindowEvidence* capture_window=&window;
#else
    bridge::CaptureWindowEvidence* capture_window=nullptr;
#endif
#else
    const bridge::BenchConfig config=bridge::BenchConfig::normal();
    bridge::OutputEvidence* output_evidence=nullptr;
    bridge::CaptureWindowEvidence* capture_window=nullptr;
#endif
    static platform::EspPumpOutput output(sdk,config.scenario!=bridge::PumpScenario::Normal,output_evidence);
    static platform::LocalDiagnostics diagnostic(config);
    static platform::StartupPolicy supervisor(clock, profile, output, diagnostic, diagnostic, BRIDGE_PROFILE_FINGERPRINT);
    static platform::RcCapture capture(diagnostic,output,sdk,config,output_evidence,capture_window);
    const int32_t reset_reason=int32_t(esp_reset_reason());
    bool handed_off=false;
    for (;;) {
        if(!handed_off) {
            supervisor.poll();
            if(supervisor.snapshot().phase==bridge::StartupPhase::ReadyDisabled) {
                capture.startup_context(supervisor.had_failure(),reset_reason);
                if(capture.start(uint64_t(clock.read().raw_us)))handed_off=true;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
