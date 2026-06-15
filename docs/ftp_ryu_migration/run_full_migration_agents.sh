#!/usr/bin/env bash
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/runner_config.env"

if [[ ! -f "$CONFIG_FILE" ]]; then
    printf 'ERROR: missing config: %s\n' "$CONFIG_FILE" >&2
    exit 1
fi

# shellcheck disable=SC1090
source "$CONFIG_FILE"

PROJECT_ROOT="${PROJECT_ROOT:-/mnt/c/Users/energ/Desktop/ACL/GS/5th_GS}"
PROMPT_DIR="$PROJECT_ROOT/docs/ftp_ryu_migration/prompts"
LOG_DIR="$PROJECT_ROOT/docs/ftp_ryu_migration/logs"
STATE_DIR="$PROJECT_ROOT/docs/ftp_ryu_migration/state"
RUNNER_STATE="$STATE_DIR/runner_state.md"
ITERATION_LOG="$STATE_DIR/iteration_log.md"
FINAL_REPORT="$PROJECT_ROOT/docs/ftp_ryu_migration/reports/final_report.md"
CODEX_BIN="${CODEX_BIN:-codex}"
MODE="full"
RUN_ID="$(date +%Y%m%d_%H%M%S)"
RUNNER_LOG="$LOG_DIR/${RUN_ID}_runner.full.log"
START_STATUS_FILE="$LOG_DIR/${RUN_ID}_git_status_start.txt"
START_DIFF_FILE="$LOG_DIR/${RUN_ID}_git_diff_start.patch"
END_STATUS_FILE="$LOG_DIR/${RUN_ID}_git_status_end.txt"
END_DIFF_FILE="$LOG_DIR/${RUN_ID}_git_diff_end.patch"
ROLLBACK_FILE="$LOG_DIR/rollback_instructions_${RUN_ID}.md"
TOTAL_RUNS=0
CURRENT_PHASE="initialization"
CURRENT_AGENT="none"
LAST_MARKER="none"
LAST_OUTPUT="none"
RISK_FLAG="none"
START_BRANCH="unknown"
START_COMMIT="unknown"
FINALIZED=0

declare -A RETRY_COUNTS=()
declare -A LAST_OUTPUTS=()
declare -A LAST_MARKERS=()

case "${1:-}" in
    "") ;;
    --phase1-only) MODE="phase1" ;;
    --supervisor-only) MODE="supervisor" ;;
    *)
        printf 'Usage: %s [--phase1-only|--supervisor-only]\n' "$0" >&2
        exit 2
        ;;
esac

mkdir -p "$LOG_DIR" "$STATE_DIR" "$PROJECT_ROOT/docs/ftp_ryu_migration/reports"

timestamp() {
    date '+%Y-%m-%d %H:%M:%S %z'
}

log() {
    local message="$*"
    printf '[%s] %s\n' "$(timestamp)" "$message" | tee -a "$RUNNER_LOG"
}

append_state() {
    local status="$1"
    local next_action="${2:-none}"
    cat > "$RUNNER_STATE" <<EOF
# Runner State

- status: \`$status\`
- mode: \`$MODE\`
- run_id: \`$RUN_ID\`
- current_phase: \`$CURRENT_PHASE\`
- current_agent: \`$CURRENT_AGENT\`
- total_agent_runs: $TOTAL_RUNS
- last_marker: \`$LAST_MARKER\`
- last_output: \`$LAST_OUTPUT\`
- risk_flag: \`$RISK_FLAG\`
- next_action: $next_action
- final_report: \`docs/ftp_ryu_migration/reports/final_report.md\`
- updated_at: $(timestamp)
EOF
}

append_iteration() {
    local phase="$1"
    local agent="$2"
    local exit_code="$3"
    local marker="$4"
    local retry="$5"
    local output="$6"
    local next_action="$7"
    printf '| %s | %s | %s | %s | %s | `%s` | %s | `%s` | %s |\n' \
        "$(timestamp)" "$MODE" "$phase" "$agent" "$exit_code" "$marker" \
        "$retry" "${output#$PROJECT_ROOT/}" "$next_action" >> "$ITERATION_LOG"
}

fail_runner() {
    local message="$1"
    log "FAILED: $message"
    append_state "FAILED" "$message"
    finalize_git_state
    exit 1
}

check_boolean_config() {
    local name="$1"
    local value="${!name}"
    if [[ "$value" != "0" && "$value" != "1" ]]; then
        fail_runner "$name must be 0 or 1, got: $value"
    fi
}

check_prompt() {
    local agent="$1"
    local path="$PROMPT_DIR/${agent}.md"
    [[ -f "$path" ]] || fail_runner "missing prompt: $path"
}

detect_marker() {
    local kind="$1"
    local output_file="$2"
    local marker=""
    [[ -s "$output_file" ]] || {
        printf 'UNKNOWN_DECISION\n'
        return
    }

    case "$kind" in
        worker)
            marker="$(tail -n 20 "$output_file" | grep -Eo 'AGENT_STATUS: (DONE|RETRY_REQUIRED|FAILED)' | tail -n 1 || true)"
            ;;
        phase)
            marker="$(tail -n 20 "$output_file" | grep -Eo 'PHASE_DECISION: (PHASE_GO_WITH_NOTES|PHASE_RETRY_REQUIRED|PHASE_BLOCKED|PHASE_GO)' | tail -n 1 || true)"
            ;;
        supervisor)
            marker="$(tail -n 20 "$output_file" | grep -Eo 'SUPERVISOR_DECISION: (GO_WITH_RISKS|NO_GO|GO)' | tail -n 1 || true)"
            ;;
        final)
            marker="$(tail -n 20 "$output_file" | grep -Eo 'FINAL_MANAGER_STATUS: FINAL_REPORTED' | tail -n 1 || true)"
            ;;
        orchestrator)
            marker="$(tail -n 20 "$output_file" | grep -Eo 'ORCHESTRATOR_STATUS: (DONE|BLOCKED|FAILED)' | tail -n 1 || true)"
            ;;
        *)
            marker=""
            ;;
    esac

    if [[ -n "$marker" ]]; then
        printf '%s\n' "$marker"
    else
        printf 'UNKNOWN_DECISION\n'
    fi
}

agent_prompt_path() {
    printf '%s/%s.md\n' "$PROMPT_DIR" "$1"
}

reserve_output_paths() {
    local agent="$1"
    local stamp
    stamp="$(date +%Y%m%d_%H%M%S)"
    while [[ -e "$LOG_DIR/${stamp}_${agent}.out" ]]; do
        sleep 1
        stamp="$(date +%Y%m%d_%H%M%S)"
    done
    printf '%s|%s\n' \
        "$LOG_DIR/${stamp}_${agent}.out" \
        "$LOG_DIR/${stamp}_${agent}.full.log"
}

build_codex_command() {
    CODEX_COMMAND=("$CODEX_BIN" exec -C "$PROJECT_ROOT" -s "$SANDBOX_MODE")
    if [[ -n "${CODEX_MODEL:-}" ]]; then
        CODEX_COMMAND+=(-m "$CODEX_MODEL")
    fi
    if [[ -n "${CODEX_EXTRA_ARGS:-}" ]]; then
        local -a parsed_extra=()
        read -r -a parsed_extra <<< "$CODEX_EXTRA_ARGS"
        CODEX_COMMAND+=("${parsed_extra[@]}")
    fi
}

execute_agent_once() {
    local agent="$1"
    local output_file="$2"
    local full_log="$3"
    local context="$4"
    local prompt_file
    prompt_file="$(agent_prompt_path "$agent")"
    build_codex_command
    CODEX_COMMAND+=(-o "$output_file" -)

    {
        cat "$prompt_file"
        printf '\n\n## Runner Invocation Context\n%s\n' "$context"
        printf 'Runner mode: %s\nRun ID: %s\n' "$MODE" "$RUN_ID"
    } | "${CODEX_COMMAND[@]}" > "$full_log" 2>&1
}

record_agent_result() {
    local phase="$1"
    local agent="$2"
    local kind="$3"
    local output_file="$4"
    local full_log="$5"
    local exit_code="$6"
    local retry="$7"
    local marker
    marker="$(detect_marker "$kind" "$output_file")"

    LAST_OUTPUTS["$agent"]="$output_file"
    LAST_MARKERS["$agent"]="$marker"
    CURRENT_AGENT="$agent"
    LAST_OUTPUT="$output_file"
    LAST_MARKER="$marker"

    log "$agent exit=$exit_code marker=$marker retry=$retry output=$output_file full_log=$full_log"
    append_iteration "$phase" "$agent" "$exit_code" "$marker" "$retry" "$output_file" "evaluate marker"
    append_state "RUNNING" "evaluate $agent result"
}

run_agent() {
    local phase="$1"
    local agent="$2"
    local kind="$3"
    local context="$4"
    local paths output_file full_log exit_code retry

    check_prompt "$agent"
    if (( TOTAL_RUNS >= MAX_TOTAL_AGENT_RUNS )); then
        fail_runner "MAX_TOTAL_AGENT_RUNS=$MAX_TOTAL_AGENT_RUNS exceeded"
    fi

    retry="${RETRY_COUNTS[$agent]:-0}"
    TOTAL_RUNS=$((TOTAL_RUNS + 1))
    CURRENT_PHASE="$phase"
    CURRENT_AGENT="$agent"
    paths="$(reserve_output_paths "$agent")"
    output_file="${paths%%|*}"
    full_log="${paths#*|}"
    append_state "RUNNING" "execute $agent"
    log "START agent=$agent phase=$phase prompt=$(agent_prompt_path "$agent") output=$output_file retry=$retry"

    execute_agent_once "$agent" "$output_file" "$full_log" "$context"
    exit_code=$?
    record_agent_result "$phase" "$agent" "$kind" "$output_file" "$full_log" "$exit_code" "$retry"
    return "$exit_code"
}

worker_done() {
    [[ "${LAST_MARKERS[$1]:-}" == "AGENT_STATUS: DONE" ]]
}

run_worker_until_done() {
    local phase="$1"
    local agent="$2"
    local context="$3"
    local marker exit_code

    while true; do
        run_agent "$phase" "$agent" worker "$context"
        exit_code=$?
        marker="${LAST_MARKERS[$agent]:-UNKNOWN_DECISION}"
        if (( exit_code == 0 )) && [[ "$marker" == "AGENT_STATUS: DONE" ]]; then
            return 0
        fi
        if [[ "$marker" == "UNKNOWN_DECISION" && "$STOP_ON_UNKNOWN_DECISION" == "1" ]]; then
            fail_runner "$agent returned UNKNOWN_DECISION; inspect ${LAST_OUTPUTS[$agent]}"
        fi
        if (( ${RETRY_COUNTS[$agent]:-0} >= MAX_RETRIES_PER_AGENT )); then
            fail_runner "$agent exhausted retries; inspect ${LAST_OUTPUTS[$agent]}"
        fi
        RETRY_COUNTS["$agent"]=$(( ${RETRY_COUNTS[$agent]:-0} + 1 ))
        log "RETRY agent=$agent count=${RETRY_COUNTS[$agent]}"
        context="$context

Previous run did not complete. Review the latest output and full log, correct only the reported issue, and preserve prior valid work."
    done
}

run_parallel_workers() {
    local phase="$1"
    local context="$2"
    shift 2
    local -a agents=("$@")
    local -a pids=()
    local -a outputs=()
    local -a full_logs=()
    local agent paths output_file full_log pid idx exit_code marker retry

    CURRENT_PHASE="$phase"
    if (( TOTAL_RUNS + ${#agents[@]} > MAX_TOTAL_AGENT_RUNS )); then
        fail_runner "parallel group would exceed MAX_TOTAL_AGENT_RUNS=$MAX_TOTAL_AGENT_RUNS"
    fi
    for agent in "${agents[@]}"; do
        check_prompt "$agent"
        if (( TOTAL_RUNS >= MAX_TOTAL_AGENT_RUNS )); then
            fail_runner "MAX_TOTAL_AGENT_RUNS=$MAX_TOTAL_AGENT_RUNS exceeded"
        fi
        retry="${RETRY_COUNTS[$agent]:-0}"
        TOTAL_RUNS=$((TOTAL_RUNS + 1))
        paths="$(reserve_output_paths "$agent")"
        output_file="${paths%%|*}"
        full_log="${paths#*|}"
        outputs+=("$output_file")
        full_logs+=("$full_log")
        log "START parallel agent=$agent phase=$phase output=$output_file retry=$retry"
        execute_agent_once "$agent" "$output_file" "$full_log" "$context" &
        pid=$!
        pids+=("$pid")
    done

    for idx in "${!agents[@]}"; do
        agent="${agents[$idx]}"
        exit_code=0
        wait "${pids[$idx]}" || exit_code=$?
        record_agent_result "$phase" "$agent" worker "${outputs[$idx]}" "${full_logs[$idx]}" "$exit_code" "${RETRY_COUNTS[$agent]:-0}"
    done

    for agent in "${agents[@]}"; do
        if ! worker_done "$agent"; then
            if [[ "${LAST_MARKERS[$agent]:-}" == "UNKNOWN_DECISION" && "$STOP_ON_UNKNOWN_DECISION" == "1" ]]; then
                fail_runner "$agent returned UNKNOWN_DECISION; inspect ${LAST_OUTPUTS[$agent]}"
            fi
            RETRY_COUNTS["$agent"]=$(( ${RETRY_COUNTS[$agent]:-0} + 1 ))
            if (( RETRY_COUNTS[$agent] > MAX_RETRIES_PER_AGENT )); then
                fail_runner "$agent exhausted retries"
            fi
            run_worker_until_done "$phase" "$agent" "$context"

        fi
    done
}

extract_agent_targets() {
    local output_file="$1"
    local prefix="$2"
    tail -n 20 "$output_file" 2>/dev/null | \
        grep -E "^${prefix}:" | tail -n 1 | \
        grep -Eo 'agent_(0[1-9]|10)_[a-z0-9_]+' | \
        grep -Ev 'agent_08_phase_leader|agent_09_supervisor|agent_10_final_manager' | \
        awk '!seen[$0]++' || true
}

run_orchestrator() {
    local phase="$1"
    local context="$2"
    run_agent "$phase" agent_00_orchestrator orchestrator "$context"
    local exit_code=$?
    if (( exit_code != 0 )); then
        fail_runner "Orchestrator failed; inspect ${LAST_OUTPUTS[agent_00_orchestrator]}"
    fi
    if [[ "${LAST_MARKERS[agent_00_orchestrator]:-}" != "ORCHESTRATOR_STATUS: DONE" ]]; then
        fail_runner "Orchestrator did not return DONE; inspect ${LAST_OUTPUTS[agent_00_orchestrator]}"
    fi
}

run_phase_review() {
    local phase="$1"
    shift
    local -a default_workers=("$@")
    local marker phase_output

    while true; do
        run_agent "$phase review" agent_08_phase_leader phase \
            "Review $phase only. Use the corresponding checkpoint. If retry is required, include a machine-readable line 'RETRY_AGENTS: <space-separated agent ids>' in the last 20 lines."
        if (( $? != 0 )); then
            fail_runner "Phase Leader execution failed for $phase"
        fi
        marker="${LAST_MARKERS[agent_08_phase_leader]:-UNKNOWN_DECISION}"
        phase_output="${LAST_OUTPUTS[agent_08_phase_leader]}"
        case "$marker" in
            "PHASE_DECISION: PHASE_GO"|"PHASE_DECISION: PHASE_GO_WITH_NOTES")
                log "$phase approved: $marker"
                return 0
                ;;
            "PHASE_DECISION: PHASE_BLOCKED")
                fail_runner "$phase blocked by Phase Leader; inspect $phase_output"
                ;;
            "PHASE_DECISION: PHASE_RETRY_REQUIRED")
                local -a retry_agents=()
                mapfile -t retry_agents < <(extract_agent_targets "$phase_output" "RETRY_AGENTS")
                if (( ${#retry_agents[@]} == 0 )); then
                    run_orchestrator "$phase remediation" \
                        "Phase Leader requested retry for $phase but retry targets were unclear. Read $phase_output and output RETRY_TARGETS with exact worker agent ids."
                    mapfile -t retry_agents < <(extract_agent_targets "${LAST_OUTPUTS[agent_00_orchestrator]}" "RETRY_TARGETS")
                fi
                if (( ${#retry_agents[@]} == 0 )); then
                    fail_runner "No retry target identified for $phase"
                fi
                for local_agent in "${retry_agents[@]}"; do
                    RETRY_COUNTS["$local_agent"]=$(( ${RETRY_COUNTS[$local_agent]:-0} + 1 ))
                    if (( RETRY_COUNTS[$local_agent] > MAX_RETRIES_PER_AGENT )); then
                        fail_runner "$local_agent exceeded retry limit during $phase review"
                    fi
                    run_worker_until_done "$phase retry" "$local_agent" \
                        "Retry requested by Phase Leader for $phase. Read $phase_output and address only the identified deficiencies."
                done
                ;;
            *)
                fail_runner "Unknown Phase Leader decision for $phase; inspect $phase_output"
                ;;
        esac
    done
}

run_supervisor_cycle() {
    local marker supervisor_output

    while true; do
        run_agent "Phase 6 Final Audit" agent_09_supervisor supervisor \
            "Perform the final independent audit. If NO_GO, include 'REMEDIATION_AGENTS: <space-separated worker agent ids>' in the last 20 lines when identifiable."
        if (( $? != 0 )); then
            fail_runner "Supervisor execution failed"
        fi
        marker="${LAST_MARKERS[agent_09_supervisor]:-UNKNOWN_DECISION}"
        supervisor_output="${LAST_OUTPUTS[agent_09_supervisor]}"
        case "$marker" in
            "SUPERVISOR_DECISION: GO")
                RISK_FLAG="none"
                return 0
                ;;
            "SUPERVISOR_DECISION: GO_WITH_RISKS")
                RISK_FLAG="GO_WITH_RISKS"
                return 0
                ;;
            "SUPERVISOR_DECISION: NO_GO")
                RISK_FLAG="NO_GO"
                if [[ "$MODE" == "supervisor" ]]; then
                    return 2
                fi
                if [[ "$STOP_ON_SUPERVISOR_NO_GO" == "1" ]]; then
                    return 2
                fi
                if (( ${RETRY_COUNTS[agent_09_supervisor]:-0} >= MAX_RETRIES_PER_AGENT )); then
                    return 2
                fi
                RETRY_COUNTS[agent_09_supervisor]=$(( ${RETRY_COUNTS[agent_09_supervisor]:-0} + 1 ))
                run_orchestrator "Supervisor remediation" \
                    "Supervisor returned NO_GO. Read $supervisor_output, create a remediation plan, and list exact worker ids as RETRY_TARGETS."
                local -a targets=()
                mapfile -t targets < <(extract_agent_targets "${LAST_OUTPUTS[agent_00_orchestrator]}" "RETRY_TARGETS")
                if (( ${#targets[@]} == 0 )); then
                    mapfile -t targets < <(extract_agent_targets "$supervisor_output" "REMEDIATION_AGENTS")
                fi
                if (( ${#targets[@]} == 0 )); then
                    return 2
                fi
                for local_agent in "${targets[@]}"; do
                    RETRY_COUNTS["$local_agent"]=$(( ${RETRY_COUNTS[$local_agent]:-0} + 1 ))
                    if (( RETRY_COUNTS[$local_agent] > MAX_RETRIES_PER_AGENT )); then
                        return 2
                    fi
                    run_worker_until_done "Supervisor remediation" "$local_agent" \
                        "Remediate Supervisor NO_GO findings from $supervisor_output. Preserve valid prior work and rollback safety."
                done
                run_phase_review "Supervisor remediation review" "${targets[@]}"
                ;;
            *)
                fail_runner "Unknown Supervisor decision; inspect $supervisor_output"
                ;;
        esac
    done
}

run_final_manager() {
    local mode_context="$1"
    run_agent "Phase 7 Final Report" agent_10_final_manager final "$mode_context"
    if (( $? != 0 )); then
        fail_runner "Final Manager execution failed"
    fi
    if [[ "${LAST_MARKERS[agent_10_final_manager]:-}" != "FINAL_MANAGER_STATUS: FINAL_REPORTED" ]]; then
        fail_runner "Final Manager did not return FINAL_REPORTED"
    fi
}

log_git_start() {
    if ! git -C "$PROJECT_ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        if [[ "$REQUIRE_GIT_REPO" == "1" ]]; then
            fail_runner "$PROJECT_ROOT is not a git repository"
        fi
        log "WARNING: git repository not detected"
        return
    fi

    START_BRANCH="$(git -C "$PROJECT_ROOT" branch --show-current 2>/dev/null || true)"
    START_COMMIT="$(git -C "$PROJECT_ROOT" rev-parse HEAD 2>/dev/null || true)"
    git -C "$PROJECT_ROOT" status --short > "$START_STATUS_FILE"
    git -C "$PROJECT_ROOT" diff --binary > "$START_DIFF_FILE"
    if [[ "$REQUIRE_CLEAN_START" == "1" && -s "$START_STATUS_FILE" ]]; then
        fail_runner "working tree is dirty and REQUIRE_CLEAN_START=1"
    fi
    log "Git start branch=$START_BRANCH commit=$START_COMMIT status=$START_STATUS_FILE diff=$START_DIFF_FILE"
}

generate_rollback_instructions() {
    cat > "$ROLLBACK_FILE" <<EOF
# FTP_Ryu Migration Rollback Instructions

- Run ID: \`$RUN_ID\`
- Start branch: \`$START_BRANCH\`
- Start commit: \`$START_COMMIT\`
- Start status: \`${START_STATUS_FILE#$PROJECT_ROOT/}\`
- Start tracked diff: \`${START_DIFF_FILE#$PROJECT_ROOT/}\`
- End status: \`${END_STATUS_FILE#$PROJECT_ROOT/}\`
- End tracked diff: \`${END_DIFF_FILE#$PROJECT_ROOT/}\`

## Rollback

1. Inspect the start status and both diff files before changing the worktree.
2. Preserve pre-existing dirty files listed in the start status.
3. Revert only migration-run changes using targeted \`git restore <path>\` or a reviewed reverse patch.
4. Recover a deleted tracked file with \`git restore --source=$START_COMMIT -- <path>\`.
5. Do not use \`git reset --hard\`; it can destroy changes that existed before the runner.
6. Review untracked files separately because \`git diff\` does not contain their content.

The runner must not permanently delete source. Old FTP removal should remain git-recoverable and should normally use build exclusion or a documented rename.
EOF
}

finalize_git_state() {
    if (( FINALIZED == 1 )); then
        return
    fi
    FINALIZED=1
    if git -C "$PROJECT_ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        git -C "$PROJECT_ROOT" status --short > "$END_STATUS_FILE"
        git -C "$PROJECT_ROOT" diff --binary > "$END_DIFF_FILE"
    else
        : > "$END_STATUS_FILE"
        : > "$END_DIFF_FILE"
    fi
    generate_rollback_instructions
    log "Git end status=$END_STATUS_FILE diff=$END_DIFF_FILE rollback=$ROLLBACK_FILE"
}

validate_config() {
    check_boolean_config STOP_ON_UNKNOWN_DECISION
    check_boolean_config STOP_ON_SUPERVISOR_NO_GO
    check_boolean_config ALLOW_AUTONOMOUS_SOURCE_CHANGES
    check_boolean_config ALLOW_OLD_FTP_BUILD_EXCLUSION
    check_boolean_config ALLOW_FINAL_REPLACEMENT
    check_boolean_config REQUIRE_GIT_REPO
    check_boolean_config REQUIRE_CLEAN_START
    [[ "$MAX_RETRIES_PER_AGENT" =~ ^[0-9]+$ ]] || fail_runner "MAX_RETRIES_PER_AGENT must be a non-negative integer"
    [[ "$MAX_TOTAL_AGENT_RUNS" =~ ^[1-9][0-9]*$ ]] || fail_runner "MAX_TOTAL_AGENT_RUNS must be a positive integer"
    [[ "$SANDBOX_MODE" == "workspace-write" ]] || fail_runner "SANDBOX_MODE must remain workspace-write"
    command -v "$CODEX_BIN" >/dev/null 2>&1 || fail_runner "codex CLI not found"
}

check_phase1_scope() {
    local current_scope start_scope
    current_scope="$(mktemp)"
    start_scope="$(mktemp)"
    git -C "$PROJECT_ROOT" status --short | \
        grep -Ev '^[ MARCUD?!]{2} docs/ftp_ryu_migration/' | sort > "$current_scope" || true
    grep -Ev '^[ MARCUD?!]{2} docs/ftp_ryu_migration/' "$START_STATUS_FILE" | sort > "$start_scope" || true
    if ! cmp -s "$start_scope" "$current_scope"; then
        cp "$current_scope" "$LOG_DIR/${RUN_ID}_phase1_scope_violation.txt"
        rm -f "$current_scope" "$start_scope"
        fail_runner "Phase 1 changed paths outside docs/ftp_ryu_migration"
    fi
    rm -f "$current_scope" "$start_scope"
}

main() {
    validate_config
    log_git_start
    append_state "RUNNING" "start mode $MODE"
    log "Runner start mode=$MODE run_id=$RUN_ID"

    if [[ "$MODE" == "supervisor" ]]; then
        if run_supervisor_cycle; then
            run_final_manager "Normal final report mode. Preserve the Supervisor decision exactly."
        else
            run_final_manager "Failure report mode. Supervisor returned NO_GO; preserve that decision and report blockers without claiming migration success."
        fi
    else
        if [[ "$MODE" == "full" ]]; then
            run_orchestrator "Phase 0" "Inspect current workflow state and prepare Phase 1 dispatch. Do not perform Worker tasks."
        fi
        run_parallel_workers "Phase 1 Analysis" \
            "Perform only Phase 1 analysis. Do not modify source, headers, CMake, build scripts, or FTP implementation." \
            agent_01_structure_analysis agent_05_protocol_wire_check agent_06_runtime_connection_diag
        run_phase_review "Phase 1 Analysis" \
            agent_01_structure_analysis agent_05_protocol_wire_check agent_06_runtime_connection_diag
        check_phase1_scope

        if [[ "$MODE" == "phase1" ]]; then
            CURRENT_PHASE="Phase 1 complete"
            append_state "DONE" "Phase 1 analysis complete"
            finalize_git_state
            log "PHASE 1 COMPLETE. Review ${LAST_OUTPUTS[agent_08_phase_leader]}"
            exit 0
        fi

        if [[ "$ALLOW_AUTONOMOUS_SOURCE_CHANGES" != "1" ]]; then
            fail_runner "Phase 2 requires ALLOW_AUTONOMOUS_SOURCE_CHANGES=1"
        fi

        run_orchestrator "Phase 2 dispatch" "Phase 1 passed Phase Leader review. Dispatch Build/CMake Worker."
        run_worker_until_done "Phase 2 Build/CMake" agent_02_build_cmake \
            "Build the Ryu client as a separate target. Preserve rollback and do not mix it into client.c."
        run_phase_review "Phase 2 Build/CMake" agent_02_build_cmake

        run_orchestrator "Phase 3 dispatch" "Phase 2 passed. Dispatch literal Ryu Client Port Worker."
        run_worker_until_done "Phase 3 Ryu Client Port" agent_03_ryu_client_port \
            "Perform the literal port with checksum/diff tracking and adapter separation."
        run_phase_review "Phase 3 Ryu Client Port" agent_03_ryu_client_port

        run_orchestrator "Phase 4 dispatch" "Phase 3 passed. Dispatch Integration Worker."
        run_worker_until_done "Phase 4 Integration" agent_04_ftprdp_integration \
            "Integrate Ryu call paths within migration scope. Maintain rollback and prove there is no hidden mixture. ALLOW_OLD_FTP_BUILD_EXCLUSION=$ALLOW_OLD_FTP_BUILD_EXCLUSION and ALLOW_FINAL_REPLACEMENT=$ALLOW_FINAL_REPLACEMENT; do not exceed these flags."
        run_phase_review "Phase 4 Integration" agent_04_ftprdp_integration

        run_orchestrator "Phase 5 dispatch" "Phase 4 passed. Dispatch Runtime update and Parser Hardening Workers."
        run_parallel_workers "Phase 5 Hardening/Diagnostics" \
            "Perform Phase 5 execution/update only. Distinguish optional and applied patches and preserve rollback." \
            agent_06_runtime_connection_diag agent_07_parser_hardening
        run_phase_review "Phase 5 Hardening/Diagnostics" \
            agent_06_runtime_connection_diag agent_07_parser_hardening

        run_orchestrator "Phase 6 dispatch" "All Phase Leader reviews passed. Prepare the final independent Supervisor audit."
        if run_supervisor_cycle; then
            run_final_manager "Normal final report mode. Preserve the Supervisor decision exactly."
        else
            run_final_manager "Failure report mode after unresolved Supervisor NO_GO. Report blockers and rollback; do not claim success."
        fi
    fi

    CURRENT_PHASE="complete"
    CURRENT_AGENT="none"
    append_state "FINAL_REPORTED" "workflow complete"
    finalize_git_state
    log "FINAL REPORT: $FINAL_REPORT"
    log "GIT DIFF SUMMARY: $END_DIFF_FILE"
    git -C "$PROJECT_ROOT" diff --stat 2>/dev/null | tee -a "$RUNNER_LOG" || true
    log "SYMBOL CHECK RESULTS: inspect supervisor/final reports and agent logs under $LOG_DIR"
    log "BUILD/TEST RESULTS: inspect reports and logs under $LOG_DIR"
}

main "$@"
