---
name: "speckit-checklist-guided"
description: "Guided wrapper around speckit-checklist: picks the checklist focus and file name from the current feature, runs speckit-checklist, then resolves every item — fixing obvious gaps itself and walking the user through each decision one at a time, in Ukrainian, with explained options, applying the chosen fix to the spec artifacts."
argument-hint: "Optional: focus override, depth, or checklist language (default: Ukrainian)"
compatibility: "Requires spec-kit project structure with .specify/ directory and the speckit-checklist skill"
metadata:
  author: "CryptoLab"
  extends: "speckit-checklist"
user-invocable: true
disable-model-invocation: false
---

## Purpose

`speckit-checklist` only generates a requirements-quality checklist ("unit tests for English"). This
skill runs the whole loop around it:

1. choose the checklist's focus areas and file name from the current feature;
2. run `speckit-checklist`;
3. resolve every item: fix the obvious ones directly, and take the user through each item that needs
   a decision, one at a time, in Ukrainian, with explained options;
4. apply each chosen fix to the feature's spec artifacts, mark the item, and move on;
5. record a Resolution section, then commit and push.

## User Input

```text
$ARGUMENTS
```

Treat any user input as an override: a focus area, depth (`lightweight` / `standard` / `reviewer`),
audience, or checklist language. If empty, derive everything yourself (step 2).

## Language rules

- **Everything shown to the user** (focus summary, explanations, options, questions, progress notes,
  final report) is in **Ukrainian**, unless the user asks otherwise.
- **The checklist file** is written in Ukrainian by default. Identifiers, file paths, FR/SC IDs,
  code, and protocol names stay verbatim.
- **Edits to spec artifacts** (`spec.md`, `plan.md`, `research.md`, `data-model.md`, `contracts/`,
  `quickstart.md`, `tasks.md`) use **each file's existing language**, which is usually English.

## Step 1 — Load context

1. Run `.specify/scripts/bash/check-prerequisites.sh --json` from the repo root. Parse `FEATURE_DIR`
   and `AVAILABLE_DOCS`. If it fails, stop and tell the user to run `/speckit-specify` first.
2. Read `FEATURE_DIR/spec.md` (required), and `plan.md`, `research.md`, `data-model.md`,
   `contracts/`, and `tasks.md` if present.
3. Read `.specify/memory/constitution.md` and the project instructions (`CLAUDE.md`/`AGENTS.md`).
4. List the existing files in `FEATURE_DIR/checklists/`, so you don't duplicate an existing
   checklist's focus.
5. If a recent `/speckit-analyze` report exists in the conversation, note its open findings. The
   checklist should cover the requirement-quality side of them.

## Step 2 — Choose focus areas and a file name

Derive **two to four** focus areas from the feature's own risk signals. Examples of signals:

- security or trust boundaries (TLS, keys, credentials, logging exclusions);
- module or dependency boundaries;
- external protocols or formats;
- persistence;
- cross-document consistency;
- compatibility;
- evidence and verification gaps;
- scope boundaries (what must not change);
- constitution MUST rules the feature touches.

Prefer the areas where an ambiguous requirement would most likely become a wrong implementation, a
failing or missing test, or an overclaim.

Pick:

- **Depth**: `reviewer` by default, or `lightweight` for a docs-only slice with little risk.
- **Audience**: the product owner or a reviewer, before `/speckit-tasks` or `/speckit-implement`.
- **File name**: a short kebab-case domain name, `<domain>.md` (for example `transport-security.md`,
  `governance.md`, `mail-boundaries.md`). It must not equal an existing checklist file unless the
  user asked to append to it.

Show the user a short Ukrainian summary (focus areas, depth, file name, one line on why) and
continue **without waiting**, unless the user's input asked to confirm first.

## Step 3 — Run speckit-checklist

Invoke the `speckit-checklist` skill with an argument that already answers its clarifying questions,
so it does not need to ask them:

```text
<file-name-domain> — <depth> depth, audience: <audience>; focus: <focus area 1>; <focus area 2>; …;
must-have checks: <constitution MUST rules and analyze findings to cover>.
Write the checklist file in Ukrainian (identifiers, paths, and FR/SC IDs verbatim).
```

Follow `speckit-checklist`'s own rules fully. Items test **requirement quality**, never
implementation behavior. At least 80% of items must carry a traceability reference (`[Spec §…]`,
`[Gap]`, `[Ambiguity]`, `[Conflict]`, or `[Assumption]`). Then run its hook checks.

## Step 4 — Triage every item

For each `- [ ] CHK###` item, check it against the **actual text** of the artifacts. Quote or cite
the lines; never judge from memory. Put it in exactly one bucket:

| Bucket | Meaning | Action |
|---|---|---|
| **A — already satisfied** | The artifacts already answer the question precisely | Mark `[x]`; note "already satisfied" with the location |
| **B — one obvious fix** | A clear defect with a single sensible fix (a missing reference, a wording gap, a contradiction with a constitution MUST, an inconsistency between two artifacts, a missing boundary number the artifacts already imply) | Apply the fix yourself, mark `[x]` |
| **C — needs a decision** | Two or more reasonable answers with different consequences for scope, security, UX, tests, or effort, or a fact only the user knows | Queue for the user |

Rules:

- A conflict with a constitution MUST is never a user option: fix it toward the constitution
  (bucket B), or, if fixing it needs a constitution change, queue it as bucket C with that fact
  stated.
- Merge items that one decision resolves (for example CHK003 + CHK004) into a single queued question.
- Do all bucket-A and bucket-B work **before** the first question. Then tell the user, in Ukrainian,
  how many items fell into each bucket and list the bucket-B fixes in one short paragraph.

## Step 5 — Walk the user through each decision, one at a time

For each queued item (or merged group), in order of impact (scope → security/privacy → correctness →
tests → wording):

1. **Header**: `### Пункт N з M — CHK### (+CHK###): <коротка назва>`.
2. **Explanation in Ukrainian**, detailed but concrete, with these parts:
   - **Суть** — what the requirement currently says or omits, citing the exact FR/SC/section.
   - **Чому це важливо** — what goes wrong in implementation, tests, or later iterations if left as
     is. Give a concrete example where possible.
   - **Що вже є** — relevant facts already in the artifacts, the constitution, or the ADRs.
3. **Ask with the `AskUserQuestion` tool**: exactly one question with **two to four options**.
   - Put the recommended option first, suffixed "(Recommended)".
   - Write labels and descriptions in Ukrainian, and let each description state what changes and its
     trade-off.
   - Include an "accept as is" option whenever leaving the requirement unchanged is legitimate.
   - The user can always type their own answer.
4. **After the answer**:
   - Apply the chosen resolution to every affected artifact, keeping the requirement IDs, wording
     style, and structure. Change the minimum needed, but keep all artifacts consistent: spec, plan,
     research, data-model, contracts, quickstart, and tasks.
   - If the answer is a fact rather than a change (for example "yes, I confirm"), record it in the
     spec's Assumptions or Clarifications with the date.
   - Verify each edit landed: re-read the changed lines, or grep for them.
   - Mark the item(s) `[x]` in the checklist.
   - Confirm in one or two Ukrainian sentences what changed and where.
5. Move on to the next item. Never batch several questions into one message, and never reveal later
   queued items in advance.

If the user says "stop", "досить", or similar: record the unresolved items as open in the Resolution
section, leave them `[ ]`, and finish at step 6.

## Step 6 — Close out

1. Append a `## Resolution (<YYYY-MM-DD>, before /speckit-<next step>)` section to the checklist file
   (in the checklist's language). It lists:
   - the owner's decisions per item (with the chosen option);
   - the bucket-B fixes;
   - the bucket-A items;
   - any analyze findings closed along the way;
   - any item left open.
2. Check the counts: every item is `[x]`, except items the user deliberately left open.
3. **Commit and push** to the current branch:
   - include the checklist file and every artifact touched;
   - use a message such as `docs: resolve <feature> <domain> checklist`;
   - add the attribution trailer the environment requires.

   Never push to a different branch, and never commit unrelated changes.
4. Report to the user in Ukrainian:
   - the checklist path;
   - item counts per bucket;
   - a table of the decisions;
   - the files changed;
   - anything left open;
   - the next Spec Kit step (usually `/speckit-tasks`, or `/speckit-implement` if tasks already
     exist).

## Guardrails

- Never edit the constitution's body, historical ADRs, or earlier features' `specs/` directories to
  resolve an item. Record the conflict and ask instead.
- Never mark an item `[x]` without either evidence (bucket A), an applied fix (B), or the user's
  recorded decision (C).
- Never invent facts: versions, test results, or command outcomes must come from sources actually
  read or commands actually run.
- Do not implement code. This skill changes requirement and planning artifacts only.
- Keep the user-facing flow brief between questions: one explanation, one question, one
  confirmation.
