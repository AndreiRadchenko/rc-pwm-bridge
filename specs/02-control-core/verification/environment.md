# Середовище 02

2026-10-28, T002: `python3 -c 'import runpy,sys; sys.exit(0 if runpy.run_path("scripts/build.py")["host_tools"]() else 3)'` — exit 0. `host_tools()` перевірив host paths/hashes та версії за `tools/environment/environment.lock.json`: Apple clang 21, CMake 3.30.2, Ninja 1.12.1, Python 3.11.7, SDK 26.5 і CLTools 26.6. Target gate T023 — ще не виконано.

T023: `python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json` — exit 0, повідомлено перевірку host, SDK, archives, wheel hashes і offline replay. Target build оцінюється окремо.
