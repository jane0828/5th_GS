# FTP_Ryu Autonomous Migration Runner

## What It Does

The runner invokes the existing Agent prompts with `codex exec`, records each final response and full CLI log, applies bounded retries, runs Phase Leader reviews after Phases 1-5, performs a final Supervisor audit, and asks Final Manager to write the final report.

`codex exec` is used because it supports non-interactive prompt execution, a fixed working directory, workspace sandbox selection, and saving the Agent's final response with `-o`.

## Before Running

- Run from a WSL environment with `codex` installed and authenticated.
- Confirm the project path is `/mnt/c/Users/energ/Desktop/ACL/GS/5th_GS`.
- Review `runner_config.env`.
- Ensure the prompt and state documents are correct.
- The default allows a dirty worktree but logs its starting state.
- If `REQUIRE_CLEAN_START=1`, commit or otherwise clean the runner documents first; untracked runner files also count as dirty.
- Keep enough Codex usage quota for repeated Agents and retries.

Do not use `--dangerously-bypass-approvals-and-sandbox`. The runner intentionally uses `-s workspace-write`.

## Commands

Full autonomous migration:

```bash
cd /mnt/c/Users/energ/Desktop/ACL/GS/5th_GS
bash docs/ftp_ryu_migration/run_full_migration_agents.sh
```

Phase 1 analysis only:

```bash
bash docs/ftp_ryu_migration/run_phase1_analysis_agents.sh
```

Supervisor and Final Manager only:

```bash
bash docs/ftp_ryu_migration/run_supervisor_only.sh
```

## Workflow

1. Orchestrator initializes state.
2. Structure, Protocol, and Runtime Analysis run as a parallel group.
3. Phase Leader reviews Phase 1.
4. Build/CMake, Ryu Port, and Integration run sequentially, each followed by Phase Leader review.
5. Runtime update and Parser Hardening run as a parallel group, followed by Phase Leader review.
6. Supervisor performs the independent final audit.
7. `NO_GO` triggers Orchestrator remediation when retry budget remains.
8. Final Manager writes a normal report for `GO`/`GO_WITH_RISKS`, or a failure report for unresolved `NO_GO`.

## Decisions and Retries

The runner reads machine-readable markers from the final 20 lines of each Agent's `-o` output. Missing markers become `UNKNOWN_DECISION`; the default is to stop.

Each Agent may be retried up to `MAX_RETRIES_PER_AGENT`. All invocations share `MAX_TOTAL_AGENT_RUNS`. Phase Leader retry output should name target Agent IDs. If it does not, Orchestrator is run to identify them. Supervisor `NO_GO` follows the same remediation pattern.

`CODEX_EXTRA_ARGS` is parsed as simple whitespace-separated arguments. Do not put quoted values containing spaces in it. Leave it empty unless necessary.

## Logs

Logs are stored under `docs/ftp_ryu_migration/logs/`:

- `TIMESTAMP_agent_name.out`: final Agent response from `codex exec -o`
- `TIMESTAMP_agent_name.full.log`: complete stdout/stderr
- `TIMESTAMP_runner.full.log`: runner events
- start/end git status and binary diff files
- `rollback_instructions_TIMESTAMP.md`

Current state is in `state/runner_state.md`; every run and retry is appended to `state/iteration_log.md`.

## Failure Handling

Inspect the `.out` file for the decision marker and the matching `.full.log` for CLI/tool failures. Correct configuration or prompt issues before restarting. The retry counters prevent infinite loops.

If Supervisor remains `NO_GO`, Final Manager may produce a failure report rather than claiming completion.

## Sandbox and Scope

`workspace-write` allows Agents to modify files under the project workspace when their prompt permits it. It does not authorize unrelated subsystem, OBC server, or CSP/libcsp core changes. Those remain user-confirmation checkpoints.

## Git and Rollback

The runner records branch, starting commit, starting status, starting tracked diff, ending status, and ending tracked diff. Read the generated rollback instructions before reverting anything.

Do not use `git reset --hard` in a dirty worktree. Revert only migration-owned paths and preserve changes listed in the starting status. Source removal must remain git-recoverable; build exclusion or documented rename is preferred.

## Final Report

Expected final report:

`docs/ftp_ryu_migration/reports/final_report.md`

The exact Agent logs, symbol evidence, and build/test reports remain under the migration docs and logs directories.
