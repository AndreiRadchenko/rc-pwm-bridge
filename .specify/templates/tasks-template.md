---

description: "Task list template for feature implementation"
---

# Tasks: [FEATURE NAME]

**Input**: Design documents from `specs/[iteration]/`

**Feature directory**: `SPECIFY_FEATURE_DIRECTORY=specs/[iteration]` (explicitly set)

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Meaningful behavior tests are required for changed control behavior, including
applicable boundaries and faults. Include separate host-test, target-build and physical
verification tasks where relevant. Test-first ordering and per-test approval are not
mandatory. Documentation-only tasks need appropriate document checks.

**Project rules**: Write generated tasks and operator instructions in Ukrainian. Follow
the constitution, roadmap dependencies and accepted scope; reference R/FR/SC IDs in tasks.
Keep input prompts and user changes. Invoke only the requested phase. Do not create/switch
branches, commit, push, flash or operate loads automatically. Physical checks are operator
steps requiring an explicit task and must remain open until performed.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **Portable core**: `firmware/components/bridge_core/`
- **ESP-IDF adapters**: `firmware/components/platform_esp32/`
- **Composition / profiles**: `firmware/main/`, `firmware/boards/`
- **Behavior tests**: `tests/`; **evidence**: `specs/[iteration]/verification/`
- Use only paths and components selected by the accepted iteration plan; these are examples.

<!--
  ============================================================================
  IMPORTANT: The tasks below are SAMPLE TASKS for illustration purposes only.

  The /speckit-tasks command MUST replace these with actual tasks based on:
  - User stories from spec.md (with their priorities P1, P2, P3...)
  - Feature requirements from plan.md
  - Entities from data-model.md
  - Applicable adapter/core interfaces from contracts/
  - Owned decision gates, requirement traceability and separate verification evidence

  Tasks MUST be organized by user story so each story can be:
  - Implemented independently
  - Tested independently
  - Delivered as an MVP increment

  DO NOT keep these sample tasks in the generated tasks.md file.
  ============================================================================
-->

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [ ] T001 Create project structure per implementation plan
- [ ] T002 Record selected pinned SDK/toolchain/test dependencies in [plan-selected file] (R02/Q01)
- [ ] T003 [P] Configure linting and formatting tools

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

Examples of foundational tasks (adjust based on your project):

Include only actual prerequisites of this iteration. Later-owned Q decisions must not
block unrelated work; a hardware-profile sample is not a prerequisite for portable-core
work that does not depend on that profile.

- [ ] T004 Record blocking Q decisions and references in [iteration spec path] before affected work
- [ ] T005 [P] Define portable-core contracts in [plan-selected core header path]
- [ ] T006 [P] Define platform adapter boundaries in [plan-selected adapter path]
- [ ] T007 Record approved profile assumptions in [profile path] without inventing GPIO
- [ ] T008 Define applicable fault/diagnostic responsibilities in [design path]
- [ ] T009 Document reproducible build/test commands in [iteration quickstart path]

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - [Title] (Priority: P1) 🎯 MVP

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Verification for User Story 1 (required for changed behavior)

> Test expected behavior and applicable boundaries/faults; record the evidence category.
> No mandatory test-first or separate approval cycle is imposed.

- [ ] T010 [P] [US1] Behavior boundary test for [requirement ID] in tests/[name].cpp
- [ ] T011 [P] [US1] Applicable fault/recovery test for [requirement ID] in tests/[name]_fault.cpp

### Implementation for User Story 1

- [ ] T012 [P] [US1] Define [core value type] in firmware/components/bridge_core/[header].hpp
- [ ] T013 [P] [US1] Define [adapter contract] in firmware/components/platform_esp32/[header].hpp
- [ ] T014 [US1] Implement [core behavior] in firmware/components/bridge_core/[source].cpp (depends on T012, T013)
- [ ] T015 [US1] Implement [planned integration] in firmware/main/[source].cpp
- [ ] T016 [US1] Add specified validation/fault handling in [plan-selected path] (applicable FR ID)
- [ ] T017 [US1] Add bounded nonblocking diagnostics in [plan-selected path] (R09/R11, if in scope)

**Checkpoint**: Verify User Story 1 within its declared dependencies; record unperformed checks.

---

## Phase 4: User Story 2 - [Title] (Priority: P2)

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Verification for User Story 2 (required for changed behavior)

- [ ] T018 [P] [US2] Behavior boundary test for [requirement ID] in tests/[name].cpp
- [ ] T019 [P] [US2] Applicable fault/recovery test for [requirement ID] in tests/[name]_fault.cpp

### Implementation for User Story 2

- [ ] T020 [P] [US2] Define [planned value type] in firmware/components/bridge_core/[header].hpp
- [ ] T021 [US2] Implement [planned behavior] in firmware/components/bridge_core/[source].cpp
- [ ] T022 [US2] Implement [planned integration] in firmware/main/[source].cpp
- [ ] T023 [US2] Integrate with User Story 1 in [plan-selected path] without changing agreed core behavior

**Checkpoint**: Verify User Stories 1 and 2; retain separate host/build/physical status.

---

## Phase 5: User Story 3 - [Title] (Priority: P3)

**Goal**: [Brief description of what this story delivers]

**Independent Test**: [How to verify this story works on its own]

### Verification for User Story 3 (required for changed behavior)

- [ ] T024 [P] [US3] Behavior boundary test for [requirement ID] in tests/[name].cpp
- [ ] T025 [P] [US3] Applicable fault/recovery test for [requirement ID] in tests/[name]_fault.cpp

### Implementation for User Story 3

- [ ] T026 [P] [US3] Define [planned value type] in firmware/components/bridge_core/[header].hpp
- [ ] T027 [US3] Implement [planned behavior] in firmware/components/bridge_core/[source].cpp
- [ ] T028 [US3] Implement [planned integration] in firmware/main/[source].cpp

**Checkpoint**: Compare each story with its criteria; incomplete physical acceptance remains open.

---

[Add more user story phases as needed, following the same pattern]

---

## Phase N: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] TXXX [P] Documentation updates in docs/
- [ ] TXXX Code cleanup and refactoring
- [ ] TXXX Performance optimization across all stories
- [ ] TXXX [P] Run relevant host behavior tests and record results in specs/[iteration]/verification/[host record]
- [ ] TXXX Run target build with pinned configuration and record evidence in specs/[iteration]/verification/[build record]
- [ ] TXXX Document applicable operator physical checks and predeclared limits in specs/[iteration]/verification/[physical procedure]; leave unperformed checks open
- [ ] TXXX Check R/FR/SC-to-task/evidence coverage in [iteration acceptance record]
- [ ] TXXX Run applicable quickstart.md validation without automatic flashing/load operation

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - May integrate with US1 but should be independently testable
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - May integrate with US1/US2 but should be independently testable

### Within Each User Story

- Resolve blocking Q decisions before affected implementation; retain deferred owners
- Verify behavior and applicable faults with meaningful tests; no mandatory test-first order
- Keep host-test results, target-build evidence and physical acceptance separate
- Core contracts before their consumers
- Adapter contracts before platform integration
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- All tests for a user story marked [P] can run in parallel
- Models within a story marked [P] can run in parallel
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Run independent host tests for User Story 1 (no hardware operation):
Task: "Behavior boundary test for [requirement ID] in tests/[name].cpp"
Task: "Applicable fault/recovery test for [requirement ID] in tests/[name]_fault.cpp"

# Independent contract-definition tasks for User Story 1:
Task: "Define [core value type] in firmware/components/bridge_core/[header].hpp"
Task: "Define [adapter contract] in firmware/components/platform_esp32/[header].hpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Record acceptance status; hardware operation requires its own explicit user task

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Verify → Record evidence and remaining checks
3. Add User Story 2 → Verify → Record evidence and remaining checks
4. Add User Story 3 → Verify → Record evidence and remaining checks
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1
   - Developer B: User Story 2
   - Developer C: User Story 3
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Check applicable constitutional gates and traceability at review
- Do not create/switch branches, commit or push without an explicit user task
- A checked code task or successful build does not establish physical acceptance
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
