#!/usr/bin/env bash
set -euo pipefail

I2C_BUS="${I2C_BUS:-1}"
I2C_ADDR_HEX="0x20"
CLIENT_PATH="/sys/bus/i2c/devices/${I2C_BUS}-0020"
NEW_DEVICE="/sys/bus/i2c/devices/i2c-${I2C_BUS}/new_device"
DELETE_DEVICE="/sys/bus/i2c/devices/i2c-${I2C_BUS}/delete_device"

PIDS=()
BOUND_TEMP=0

usage() {
	cat <<'EOF'
Usage:
  validate-mux.sh hold <fixture> <state>
  validate-mux.sh sample <fixture>

Fixtures:
  an-int
    Required jumper: mikroBUS AN -> INT

  an-pwm
    Required jumper: mikroBUS AN -> PWM

  an-miso
    Required jumper: mikroBUS AN -> MISO

  an-rst-j7-pwm
    Required jumpers:
      mikroBUS AN -> RST
      J7/7 -> mikroBUS PWM

States:
  an-int: low | high
  an-pwm: low | high
  an-rst-j7-pwm: low | high

Examples:
  validate-mux.sh hold an-int low
  validate-mux.sh sample an-int
  validate-mux.sh sample an-miso
EOF
}

cleanup() {
	release_pids

	if [[ "${BOUND_TEMP}" -eq 1 ]]; then
		echo "${I2C_ADDR_HEX}" | sudo tee "${DELETE_DEVICE}" >/dev/null || true
	fi
}

trap cleanup EXIT INT TERM

require_cmd() {
	command -v "$1" >/dev/null 2>&1 || {
		echo "missing required command: $1" >&2
		exit 1
	}
}

ensure_client() {
	if [[ ! -e "${CLIENT_PATH}" ]]; then
		echo "binding temporary pca9538 client at ${I2C_ADDR_HEX} on i2c-${I2C_BUS}" >&2
		echo "pca9538 ${I2C_ADDR_HEX}" | sudo tee "${NEW_DEVICE}" >/dev/null
		BOUND_TEMP=1
	fi
}

detect_expander_chip() {
	local chip

	chip="$(gpiodetect | awk '$2 == "[1-0020]" {print $1; exit}')"
	if [[ -z "${chip}" ]]; then
		echo "unable to find gpio-pca953x chip for ${CLIENT_PATH}" >&2
		exit 1
	fi

	printf '%s\n' "${chip}"
}

line_exists() {
	gpioinfo "$1" >/dev/null 2>&1
}

selector_offset() {
	case "$1" in
	RST_SEL) printf '0\n' ;;
	PWM_SEL) printf '1\n' ;;
	AN_SEL) printf '2\n' ;;
	INT_SEL) printf '3\n' ;;
	CIPO_SEL_0) printf '4\n' ;;
	CIPO_SEL_1) printf '5\n' ;;
	*)
		echo "unknown selector line: $1" >&2
		exit 1
		;;
	esac
}

release_pids() {
	for pid in "${PIDS[@]:-}"; do
		kill "${pid}" 2>/dev/null || true
		wait "${pid}" 2>/dev/null || true
	done

	PIDS=()
}

expander_set_bg() {
	local consumer="$1"
	shift

	if line_exists "RST_SEL"; then
		gpioset -C "${consumer}" "$@" &
	else
		local chip
		local translated=()
		local arg name value

		chip="$(detect_expander_chip)"
		for arg in "$@"; do
			name="${arg%%=*}"
			value="${arg#*=}"
			translated+=("$(selector_offset "${name}")=${value}")
		done

		gpioset -C "${consumer}" -c "${chip}" "${translated[@]}" &
	fi

	PIDS+=("$!")
}

host_set_bg() {
	local consumer="$1"
	shift

	gpioset -C "${consumer}" "$@" &
	PIDS+=("$!")
}

probe_lines() {
	gpioget --numeric "$@"
}

hold_state() {
	local fixture="$1"
	local state="$2"

	case "${fixture}:${state}" in
	an-int:low)
		host_set_bg host-an-int-low GPIO13=0 GPIO20=1
		expander_set_bg mux-an-int-low AN_SEL=0 INT_SEL=0
		cat <<'EOF'
Fixture: AN -> INT
Driven state: AN_SEL=0, INT_SEL=0
Expected:
  GPIO16 low
  GPIO21 high
Press Ctrl-C to release the held lines.
EOF
		;;
	an-int:high)
		host_set_bg host-an-int-high GPIO13=1 GPIO20=0
		expander_set_bg mux-an-int-high AN_SEL=1 INT_SEL=1
		cat <<'EOF'
Fixture: AN -> INT
Driven state: AN_SEL=1, INT_SEL=1
Expected:
  GPIO16 high
  GPIO21 low
Press Ctrl-C to release the held lines.
EOF
		;;
	an-pwm:low)
		host_set_bg host-an-pwm GPIO13=0 GPIO20=1
		expander_set_bg mux-an-pwm-low AN_SEL=0 PWM_SEL=0
		cat <<'EOF'
Fixture: AN -> PWM
Driven state: AN_SEL=0, PWM_SEL=0
Expected:
  GPIO18 low
  GPIO17 high
Press Ctrl-C to release the held lines.
EOF
		;;
	an-pwm:high)
		host_set_bg host-an-pwm GPIO13=0 GPIO20=1
		expander_set_bg mux-an-pwm-high AN_SEL=0 PWM_SEL=1
		cat <<'EOF'
Fixture: AN -> PWM
Driven state: AN_SEL=0, PWM_SEL=1
Expected:
  GPIO18 high
  GPIO17 low
Press Ctrl-C to release the held lines.
EOF
		;;
	an-rst-j7-pwm:low)
		host_set_bg host-an-rst GPIO13=0 GPIO20=1
		expander_set_bg mux-an-rst-low AN_SEL=0 PWM_SEL=0 RST_SEL=0
		cat <<'EOF'
Fixture: AN -> RST and J7/7 -> PWM
Driven state: AN_SEL=0, PWM_SEL=0, RST_SEL=0
Expected:
  GPIO19 low
  GPIO18 high
Press Ctrl-C to release the held lines.
EOF
		;;
	an-rst-j7-pwm:high)
		host_set_bg host-an-rst GPIO13=0 GPIO20=1
		expander_set_bg mux-an-rst-high AN_SEL=0 PWM_SEL=0 RST_SEL=1
		cat <<'EOF'
Fixture: AN -> RST and J7/7 -> PWM
Driven state: AN_SEL=0, PWM_SEL=0, RST_SEL=1
Expected:
  GPIO19 high
  GPIO18 low
Press Ctrl-C to release the held lines.
EOF
		;;
	*)
		echo "unsupported hold combination: ${fixture} ${state}" >&2
		exit 1
		;;
	esac

	wait
}

sample_an_int() {
	host_set_bg host-an-int-low GPIO13=0 GPIO20=1
	expander_set_bg mux-an-int-low AN_SEL=0 INT_SEL=0
	sleep 0.1
	printf 'AN->INT low state (expect GPIO16=0 GPIO21=1): '
	probe_lines GPIO16 GPIO21
	release_pids

	host_set_bg host-an-int-high GPIO13=1 GPIO20=0
	expander_set_bg mux-an-int-high AN_SEL=1 INT_SEL=1
	sleep 0.1
	printf 'AN->INT high state (expect GPIO16=1 GPIO21=0): '
	probe_lines GPIO16 GPIO21
}

sample_an_pwm() {
	host_set_bg host-an-pwm GPIO13=0 GPIO20=1
	expander_set_bg mux-an-pwm-low AN_SEL=0 PWM_SEL=0
	sleep 0.1
	printf 'AN->PWM low state (expect GPIO18=0 GPIO17=1): '
	probe_lines GPIO18 GPIO17
	release_pids

	host_set_bg host-an-pwm GPIO13=0 GPIO20=1
	expander_set_bg mux-an-pwm-high AN_SEL=0 PWM_SEL=1
	sleep 0.1
	printf 'AN->PWM high state (expect GPIO18=1 GPIO17=0): '
	probe_lines GPIO18 GPIO17
}

sample_an_miso() {
	local -a states=("0 0" "0 1" "1 0" "1 1")
	local idx=0

	for state in "${states[@]}"; do
		local sel0 sel1
		read -r sel0 sel1 <<<"${state}"
		host_set_bg host-an-miso GPIO13=0
		expander_set_bg mux-an-miso AN_SEL=0 CIPO_SEL_0="${sel0}" CIPO_SEL_1="${sel1}"
		sleep 0.1
		printf 'AN->MISO state %d (CIPO_SEL_0=%s CIPO_SEL_1=%s) -> GPIO16 GPIO20 GPIO17: ' \
			"${idx}" "${sel0}" "${sel1}"
		probe_lines GPIO16 GPIO20 GPIO17
		release_pids
		idx=$((idx + 1))
	done
}

sample_an_rst_j7_pwm() {
	host_set_bg host-an-rst GPIO13=0 GPIO20=1
	expander_set_bg mux-an-rst-low AN_SEL=0 PWM_SEL=0 RST_SEL=0
	sleep 0.1
	printf 'AN->RST / J7->PWM low state (expect GPIO19=0 GPIO18=1): '
	probe_lines GPIO19 GPIO18
	release_pids

	host_set_bg host-an-rst GPIO13=0 GPIO20=1
	expander_set_bg mux-an-rst-high AN_SEL=0 PWM_SEL=0 RST_SEL=1
	sleep 0.1
	printf 'AN->RST / J7->PWM high state (expect GPIO19=1 GPIO18=0): '
	probe_lines GPIO19 GPIO18
}

main() {
	local mode fixture state

	require_cmd gpioinfo
	require_cmd gpioget
	require_cmd gpioset
	require_cmd gpiodetect

	mode="${1:-}"
	fixture="${2:-}"
	state="${3:-}"

	if [[ -z "${mode}" || -z "${fixture}" ]]; then
		usage >&2
		exit 1
	fi

	ensure_client

	case "${mode}:${fixture}" in
	hold:*)
		if [[ -z "${state}" ]]; then
			usage >&2
			exit 1
		fi
		hold_state "${fixture}" "${state}"
		;;
	sample:an-int)
		sample_an_int
		;;
	sample:an-pwm)
		sample_an_pwm
		;;
	sample:an-miso)
		sample_an_miso
		;;
	sample:an-rst-j7-pwm)
		sample_an_rst_j7_pwm
		;;
	*)
		usage >&2
		exit 1
		;;
	esac
}

main "$@"
