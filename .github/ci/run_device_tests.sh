#!/usr/bin/env bash
set -uo pipefail

usage() {
  echo "Usage: $0 [--validate-only] <hggc-samples-810e.tar.gz>" >&2
}

validate_only=0
if [[ "${1:-}" == "--validate-only" ]]; then
  validate_only=1
  shift
fi

if [[ $# -ne 1 ]]; then
  usage
  exit 2
fi

archive=$1
if [[ ! -f "${archive}" ]]; then
  echo "archive not found: ${archive}" >&2
  exit 2
fi

root=$(mktemp -d)
trap 'rm -rf "${root}"' EXIT
if ! tar -xzf "${archive}" -C "${root}"; then
  echo "cannot unpack archive: ${archive}" >&2
  exit 1
fi

manifest="${root}/ci-cases.tsv"
bin_dir="${root}/bin"
if [[ ! -s "${manifest}" ]]; then
  echo "missing or empty case manifest: ci-cases.tsv" >&2
  exit 1
fi
if [[ ! -d "${bin_dir}" ]]; then
  echo "missing binary directory: bin" >&2
  exit 1
fi

failures=()
case_count=0
record_failure() {
  failures+=("$1")
}

validate_asset_list() {
  local assets=$1
  local asset
  local asset_path
  local -a asset_list

  [[ -n "${assets}" ]] || return
  IFS=',' read -r -a asset_list <<<"${assets}"
  for asset in "${asset_list[@]}"; do
    case "${asset}" in
      ""|*/*|*..*)
        record_failure "unsafe runtime asset in manifest: ${asset:-<empty>}"
        continue
        ;;
    esac
    asset_path="${bin_dir}/${asset}"
    if [[ ! -f "${asset_path}" ]]; then
      record_failure "missing runtime asset: ${asset}"
    fi
  done
}

while IFS=$'\t' read -r binary expected description assets || [[ -n "${binary}" ]]; do
  [[ -n "${binary}" ]] || continue
  case_count=$((case_count + 1))

  case "${binary}" in
    */*|*..*)
      record_failure "unsafe binary name in manifest: ${binary}"
      continue
      ;;
  esac
  if [[ -z "${expected}" || -z "${description}" ]]; then
    record_failure "invalid manifest entry for ${binary}"
    continue
  fi
  if [[ ! -x "${bin_dir}/${binary}" ]]; then
    record_failure "missing or non-executable binary: ${binary}"
  fi
  validate_asset_list "${assets}"
done <"${manifest}"

if [[ ${case_count} -eq 0 ]]; then
  record_failure "manifest contains no cases"
fi

if [[ ${#failures[@]} -ne 0 ]]; then
  echo "bundle validation failed:" >&2
  for failure in "${failures[@]}"; do
    echo "  - ${failure}" >&2
  done
  exit 1
fi

if [[ ${validate_only} -eq 1 ]]; then
  echo "validated ${case_count} device cases in ${archive}"
  exit 0
fi

if ! command -v timeout >/dev/null 2>&1; then
  echo "timeout command is required" >&2
  exit 1
fi

sdk_env="${PPU_SDK:-/usr/local/PPU_SDK}/envsetup.sh"
if [[ -f "${sdk_env}" ]]; then
  set +u
  if ! source "${sdk_env}"; then
    echo "warning: ${sdk_env} returned a non-zero status" >&2
  fi
  set -u
else
  echo "warning: SDK envsetup.sh not found at ${sdk_env}" >&2
fi

echo "node: ${NODE_NAME:-unknown}"
if command -v ppu-smi >/dev/null 2>&1; then
  ppu-smi || echo "warning: ppu-smi failed" >&2
else
  echo "warning: ppu-smi is not available" >&2
fi

failures=()
passed=0
while IFS=$'\t' read -r binary expected description assets || [[ -n "${binary}" ]]; do
  [[ -n "${binary}" ]] || continue
  echo
  echo "=== ${binary}: ${description} ==="

  lib_path="${bin_dir}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
  output=$(cd "${bin_dir}" && \
    LD_LIBRARY_PATH="${lib_path}" timeout -k 10 300 "./${binary}" 2>&1)
  rc=$?
  printf '%s\n' "${output}"

  if [[ ${rc} -ne 0 ]]; then
    echo "--- FAILED: ${binary} exited with ${rc}"
    failures+=("${binary}: exit ${rc}")
    continue
  fi
  if ! printf '%s\n' "${output}" | grep -qF -- "${expected}"; then
    echo "--- FAILED: ${binary} did not print '${expected}'"
    failures+=("${binary}: missing success marker")
    continue
  fi
  if printf '%s\n' "${output}" | grep -Eqi \
      '(^|[[:space:]=:])FAIL(ED)?([[:space:]]|$)|MISMATCH[[:space:]]*\[|Mismatches:[[:space:]]*[1-9][0-9]*|ERROR!'; then
    echo "--- FAILED: ${binary} printed a failure marker"
    failures+=("${binary}: failure marker in output")
    continue
  fi

  echo "--- passed: ${binary}"
  passed=$((passed + 1))
done <"${manifest}"

echo
echo "${case_count} device cases, ${passed} passed, ${#failures[@]} failed"
if [[ ${#failures[@]} -ne 0 ]]; then
  for failure in "${failures[@]}"; do
    echo "  - ${failure}"
  done
  exit 1
fi
