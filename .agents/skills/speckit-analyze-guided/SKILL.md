---
name: "speckit-analyze-guided"
description: "Guided wrapper around speckit-analyze: runs the read-only cross-artifact analysis, then resolves every finding — fixing obvious defects itself and walking the user through each decision one at a time, in Ukrainian, with explained options — applies the chosen fixes to spec/plan/tasks, re-verifies, and records the resolution."
argument-hint: "Optional: focus areas for the analysis, a severity floor (e.g. 'MEDIUM+'), or 'report-only'"
compatibility: "Requires spec-kit project structure with .specify/ directory, a complete tasks.md, and the speckit-analyze skill"
metadata:
  author: "CryptoLab"
  extends: "speckit-analyze"
user-invocable: true
disable-model-invocation: false
---

## Purpose

`speckit-analyze` is strictly read-only. It produces a findings report and then only *offers*
remediation. This skill runs the whole loop around it:

1. run `speckit-analyze` and get its report, without changing any file;
2. triage every finding:
   - fix the obvious ones directly;
   - take the user through each finding that needs a decision, one at a time, in Ukrainian, with
     explained options;
3. apply each chosen fix to the feature's artifacts;
4. re-run the detection passes on the changed artifacts, to confirm the findings are closed and no
   new ones appeared;
5. record the resolution, then commit and push.

**Approval.** Invoking this skill is the user's explicit approval to remediate, which
`speckit-analyze` requires before any edit. The analysis step itself stays read-only. Edits start
only after the full report has been shown. If the user input contains `report-only`, stop after
step 2 (the report and triage summary) and change nothing.

## User Input

```text
$ARGUMENTS
```

Treat any user input as an override:

- focus areas, passed through to `speckit-analyze`;
- a severity floor (for example `MEDIUM+`: findings below it are listed but not walked through, and
  stay open);
- `report-only`.

If empty, analyze everything and resolve every finding.

## Language rules

- **Everything shown to the user** is in **Ukrainian**, unless the user asks otherwise. That covers
  the report summary, explanations, options, questions, progress notes, and the final report.
  Finding IDs, severities, FR/SC/T IDs, paths, and code stay verbatim.
- **The resolution record** (`FEATURE_DIR/analysis-resolution.md`) is written in Ukrainian.
- **Edits to feature artifacts** (`spec.md`, `plan.md`, `research.md`, `data-model.md`, `contracts/`,
  `quickstart.md`, `tasks.md`, `checklists/`) use **each file's existing language**, which is
  usually English.

## Step 1 — Run speckit-analyze

1. Run `.specify/scripts/bash/check-prerequisites.sh --json --require-tasks --include-tasks` from the
   repo root. If it fails because `tasks.md` is missing, stop and tell the user to run
   `/speckit-tasks` first.
2. Invoke the `speckit-analyze` skill, passing through any focus areas from the user input. Follow
   its rules fully:
   - read-only;
   - constitution conflicts are always CRITICAL;
   - at most 50 findings;
   - stable finding IDs;
   - coverage table and metrics;
   - its `before_analyze` / `after_analyze` hook checks.
3. Show its full report. In place of its closing question ("Would you like me to suggest concrete
   remediation edits…?"), continue with step 2 of this skill. The user already approved remediation
   by invoking it (see **Purpose**), unless the input says `report-only`.

## Step 2 — Triage every finding

For each finding, re-check it against the **actual text** of the artifacts: open the cited lines and
quote them. Analysis findings can be wrong. A finding that does not reproduce is its own bucket.

| Bucket | Meaning | Action |
|---|---|---|
| **0 — not a defect** | On re-reading, the artifacts already handle it (the analyzer missed a reference, a section, or a task) | Close with the evidence (file and line); change nothing |
| **A — one obvious fix** | A clear defect with a single sensible fix | Apply the fix yourself |
| **B — needs a decision** | Two or more reasonable answers with different consequences for scope, security, UX, tests, or effort, or a fact only the user knows | Queue for the user |

Examples of bucket A:

- a requirement with no task, where the task is implied by the plan;
- a task with a wrong or missing file path;
- terminology drift toward the term the spec defines;
- a missing FR/SC reference;
- a dependency-order error in `tasks.md`;
- a contradiction with a constitution MUST that has only one compliant fix;
- a duplicate that one artifact already states better.

Rules:

- **Constitution conflicts** (CRITICAL) are never a user option. Fix them toward the constitution
  (bucket A). If the fix needs a constitution change, queue it as bucket B and state that fact: the
  change itself belongs to a separate `/speckit-constitution` run, never to this skill.
- **Coverage gaps**: add the missing task in `tasks.md` in the correct phase, with the next free ID
  or an inserted ID consistent with the file's numbering, with its `[P]` and `[US#]` markers and
  file path, and update the dependency notes if needed. Never renumber existing tasks that other
  artifacts reference, unless every reference is updated too.
- **Merge** findings that one decision resolves into a single queued question.
- Apply the severity floor from the user input if given. Findings below it are listed as "open, below
  floor".
- Do all bucket-0 and bucket-A work **before** the first question. Then tell the user, in Ukrainian:
  - how many findings fell into each bucket;
  - the bucket-A fixes, in one short paragraph;
  - the bucket-0 closures with their evidence, in one line each.

## Step 3 — Walk the user through each decision, one at a time

Order the queued findings by severity (CRITICAL → HIGH → MEDIUM → LOW). Within one severity, order
them scope → security/privacy → correctness → tests → wording.

For each queued finding or merged group:

1. **Header**: `### Знахідка N з M — <ID> (+<ID>) [<SEVERITY>]: <коротка назва>`.
2. **Explanation in Ukrainian**, detailed but concrete, with these parts:
   - **Суть**: what the artifacts currently say, citing the exact file, section, and FR/SC/T IDs, and
     where they disagree or fall short.
   - **Чому це важливо**: what goes wrong in `/speckit-implement`, tests, verification evidence, or
     later iterations if left as is. Give a concrete example where possible.
   - **Що вже є**: relevant facts already in the artifacts, the constitution, the ADRs, or earlier
     owner decisions (Clarifications, checklist Resolution sections).
3. **Ask with the `AskUserQuestion` tool**: exactly one question with **two to four options**.
   - Put the recommended option first, suffixed "(Recommended)".
   - Write labels and descriptions in Ukrainian, and let each description say which files change and
     the trade-off.
   - Include an "accept as is / defer" option whenever leaving it is legitimate, meaning never for a
     constitution conflict.
   - The user can always type their own answer.
4. **After the answer**:
   - Apply the resolution to every affected artifact, keeping IDs, wording style, and structure.
     Change the minimum needed, but keep spec, plan, research, data-model, contracts, quickstart,
     tasks, and checklists consistent with each other.
   - If the answer is a fact rather than a change, record it in the spec's Clarifications or
     Assumptions with the date.
   - If the user chose "defer", record the finding as open, with the reason, and change nothing.
   - Verify each edit landed: re-read the changed lines, or grep for them.
   - Confirm in one or two Ukrainian sentences what changed and where.
5. Move on to the next finding. Never batch several questions into one message, and never reveal
   later queued findings in advance.

If the user says "stop", "досить", or similar, record the remaining findings as open and go to
step 4.

## Step 4 — Re-verify

Re-run `speckit-analyze`'s detection passes (A–F) over the artifacts as they are now. Then report,
in Ukrainian:

- which original finding IDs are closed;
- any finding still open;
- any **new** finding the edits introduced.

Rules for this pass:

- A new finding caused by this skill's own edit is fixed now (bucket A) or queued (step 3). It is
  never left silently.
- Recompute the metrics: total requirements, total tasks, coverage %, CRITICAL count.
- Do not claim zero CRITICAL issues unless this pass actually found none.

## Step 5 — Close out

1. Write or append `FEATURE_DIR/analysis-resolution.md`, in Ukrainian, with a
   `## Аналіз <YYYY-MM-DD>, перед /speckit-<next step>` section. It contains:
   - the original metrics and the re-verified metrics;
   - a table of all findings: ID, severity, bucket (0/A/B), resolution, and the files changed;
   - the owner's decisions, with the chosen option;
   - open findings (deferred, below the floor, or unresolved after a stop), each with its reason;
   - new findings from step 4 and how they were resolved.
2. **Commit and push** to the current branch:
   - include the resolution file and every artifact touched;
   - use a message such as `docs: resolve <feature> analysis findings`;
   - add the attribution trailer the environment requires.

   Never push to a different branch, and never commit unrelated changes. If nothing changed
   (`report-only`, or every finding was bucket 0), commit only the resolution file, and only if the
   user did not ask for a report-only run.
3. Report to the user in Ukrainian:
   - finding counts per bucket and per severity;
   - a table of the decisions;
   - the files changed;
   - the before/after metrics;
   - anything left open;
   - the next Spec Kit step. Usually that is `/speckit-implement`. If any CRITICAL remains open, say
     plainly that implementation should wait.

## Guardrails

- The analysis step (step 1) never writes files. Edits happen only in steps 2–4, after the report
  has been shown.
- Never edit the constitution's body, historical ADRs, or earlier features' `specs/` directories to
  resolve a finding. Record the conflict and ask, or point to `/speckit-constitution` / a new ADR.
- Never close a finding without either evidence (bucket 0), an applied and verified fix (A), or the
  user's recorded decision (B).
- Never invent facts: versions, test results, or command outcomes must come from sources actually
  read or commands actually run. Never turn a "Not run" command into a pass.
- Do not implement code. This skill changes requirement, planning, and task artifacts only.
- Keep the user-facing flow brief between questions: one explanation, one question, one
  confirmation.
