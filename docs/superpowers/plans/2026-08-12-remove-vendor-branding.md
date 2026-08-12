# Remove Vendor Branding Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove specified vendor attribution and promotional text from all code files without changing compiled program behavior or useful technical comments.

**Architecture:** Discover matching source files by extension and keyword, preserve each file's original encoding and newline style, and apply comment-only line removal or neutral wording replacement. Verify that executable tokens remain unchanged where practical, scan for residual terms, then rebuild both available H743 Keil projects.

**Tech Stack:** PowerShell, ripgrep, Git, Keil µVision/ARMCC.

## Global Constraints

- Only `.c`, `.h`, `.cpp`, `.hpp`, `.cc`, `.cxx`, `.s`, and `.asm` files are in scope.
- Do not edit documentation, PDFs, archives, spreadsheets, images, project configuration, symbols, constants, parameters, or runtime strings.
- Preserve useful technical meaning, source encoding, and newline style.
- Do not commit or push.

---

### Task 1: Establish the failing brand scan

**Files:**
- Inspect: all in-scope source files under `E:\git`

- [ ] Record every source file and matching line for `正点原子`, `ALIENTEK`, `alientek`, `openedv`, `yuanzige`, `星翼电子`.
- [ ] Confirm every match occurs inside a comment before editing.

### Task 2: Apply comment-only cleanup

**Files:**
- Modify: only matching source files identified by Task 1

- [ ] Delete pure author, copyright, company, purchase, forum, video, and website comment lines.
- [ ] Replace brand-qualified technical phrases with neutral component descriptions.
- [ ] Preserve original file encoding and newline convention.

### Task 3: Verify textual and semantic scope

**Files:**
- Inspect: all modified source files

- [ ] Re-run the complete source-only brand scan; expected result: zero matches.
- [ ] Run `git diff --check`; expected result: no whitespace errors.
- [ ] Inspect the diff and confirm changed lines are comments or adjacent comment whitespace only.
- [ ] Confirm no non-comment code token was changed.

### Task 4: Build verification

**Files:**
- Build: `fireware/STM32H743Project_LVGL8_Official/Projects/MDK-ARM/atk_h743.uvprojx`
- Build if available: the Keil project under `fireware/STM32H743Project`

- [ ] Run a clean full build for each usable H743 project.
- [ ] Require the current official H743 project to report `0 Error(s), 0 Warning(s)`.
- [ ] Report any older project that cannot be built separately, without weakening verification of the current project.
