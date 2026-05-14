#!/usr/bin/env bash
set -euo pipefail

I2C_BUS="${I2C_BUS:-1}"
I2C_ADDR_HEX="${I2C_ADDR_HEX:-0x20}"
I2C_ADDR_SUFFIX="${I2C_ADDR_HEX#0x}"
I2C_ADDR_SUFFIX="${I2C_ADDR_SUFFIX#0X}"
I2C_ADDR_SUFFIX="$(printf '%04x' "0x${I2C_ADDR_SUFFIX}")"
BIND_CLIENT=1
DRY_RUN=0

SELECTOR_LINES=(RST_SEL PWM_SEL AN_SEL INT_SEL CIPO_SEL_0 CIPO_SEL_1)

usage() {
	cat <<'EOF_USAGE'
Usage:
  set-mux-profile.sh --host <host> [options]
  set-mux-profile.sh <host> [options]

Host profiles:
  byai-am67a, beagley-ai
    RST_SEL=1 PWM_SEL=1 AN_SEL=1 INT_SEL=1 CIPO_SEL_0=1 CIPO_SEL_1=0

  sk-am62, am62-sk
    RST_SEL=0 PWM_SEL=0 AN_SEL=0 INT_SEL=0 CIPO_SEL_0=0 CIPO_SEL_1=1

  sk-am68, am68-sk, sk-am69, am69-sk, sk-am68-9
    RST_SEL=1 PWM_SEL=1 AN_SEL=1 INT_SEL=1 CIPO_SEL_0=1 CIPO_SEL_1=1

Note:
  BeaglePlay / BP-AM62 is not listed because it does not use the mikroBUS HAT mux path.

Options:
  --host <host>       Select the host-board mux profile to apply.
  --i2c-bus <bus>     I2C bus number for the MSPM0 pca9538 target (default: $I2C_BUS or 1).
  --addr <addr>       I2C address for the MSPM0 pca9538 target (default: $I2C_ADDR_HEX or 0x20).
  --no-bind           Do not create a temporary pca9538 Linux client if one is missing.
  --dry-run           Print the resolved selector values without touching GPIOs.
  -h, --help          Show this help.

Environment:
  I2C_BUS             Default I2C bus number when --i2c-bus is not provided.
  I2C_ADDR_HEX        Default pca9538 I2C address when --addr is not provided.

Examples:
  set-mux-profile.sh --host byai-am67a
  set-mux-profile.sh sk-am62 --i2c-bus 1
  I2C_BUS=1 set-mux-profile.sh --host sk-am69
EOF_USAGE
}

require_cmd() {
	command -v "$1" >/dev/null 2>&1 || {
		echo "missing required command: $1" >&2
		exit 1
	}
}

refresh_i2c_paths() {
	I2C_ADDR_SUFFIX="${I2C_ADDR_HEX#0x}"
	I2C_ADDR_SUFFIX="${I2C_ADDR_SUFFIX#0X}"
	I2C_ADDR_SUFFIX="$(printf '%04x' "0x${I2C_ADDR_SUFFIX}")"
	CLIENT_PATH="/sys/bus/i2c/devices/${I2C_BUS}-${I2C_ADDR_SUFFIX}"
	NEW_DEVICE="/sys/bus/i2c/devices/i2c-${I2C_BUS}/new_device"
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

profile_values() {
	case "$1" in
	byai-am67a|beagley-ai|beagley-ai-am67a)
		printf '1 1 1 1 1 0\n'
		;;
	sk-am62|am62-sk|ti-am62-sk)
		printf '0 0 0 0 0 1\n'
		;;
	sk-am68|am68-sk|ti-am68-sk|sk-am69|am69-sk|ti-am69-sk|sk-am68-9|sk-am68\/9)
		printf '1 1 1 1 1 1\n'
		;;
	*)
		echo "unsupported host profile: $1" >&2
		usage >&2
		exit 1
		;;
	esac
}

ensure_client() {
	if [[ "${BIND_CLIENT}" -eq 0 || -e "${CLIENT_PATH}" ]]; then
		return
	fi

	if [[ ! -w "${NEW_DEVICE}" ]]; then
		echo "binding temporary pca9538 client at ${I2C_ADDR_HEX} on i2c-${I2C_BUS}" >&2
		echo "pca9538 ${I2C_ADDR_HEX}" | sudo tee "${NEW_DEVICE}" >/dev/null
	else
		echo "binding temporary pca9538 client at ${I2C_ADDR_HEX} on i2c-${I2C_BUS}" >&2
		echo "pca9538 ${I2C_ADDR_HEX}" >"${NEW_DEVICE}"
	fi
}

detect_expander_chip() {
	local chip label

	label="[${I2C_BUS}-${I2C_ADDR_SUFFIX}]"
	chip="$(gpiodetect | awk -v label="${label}" '$2 == label {print $1; exit}')"
	if [[ -z "${chip}" ]]; then
		echo "unable to find gpio-pca953x chip for ${CLIENT_PATH}" >&2
		exit 1
	fi

	printf '%s\n' "${chip}"
}

line_exists() {
	gpioinfo "$1" >/dev/null 2>&1
}

build_gpio_args() {
	local values=($1)
	local args=()
	local idx

	for idx in "${!SELECTOR_LINES[@]}"; do
		args+=("${SELECTOR_LINES[$idx]}=${values[$idx]}")
	done

	printf '%s\n' "${args[@]}"
}

apply_profile() {
	local host="$1"
	local values
	local args=()
	local translated=()
	local arg name value chip

	values="$(profile_values "${host}")"
	mapfile -t args < <(build_gpio_args "${values}")

	printf 'Applying mux profile "%s" on i2c-%s address %s:\n' "${host}" "${I2C_BUS}" "${I2C_ADDR_HEX}"
	printf '  %s\n' "${args[@]}"

	if [[ "${DRY_RUN}" -eq 1 ]]; then
		return
	fi

	ensure_client

	if line_exists "RST_SEL"; then
		gpioset -C "mux-profile-${host}" "${args[@]}"
	else
		chip="$(detect_expander_chip)"
		for arg in "${args[@]}"; do
			name="${arg%%=*}"
			value="${arg#*=}"
			translated+=("$(selector_offset "${name}")=${value}")
		done

		gpioset -C "mux-profile-${host}" -c "${chip}" "${translated[@]}"
	fi
}

main() {
	local host=""

	while [[ $# -gt 0 ]]; do
		case "$1" in
		--host)
			host="${2:-}"
			if [[ -z "${host}" ]]; then
				echo "--host requires a value" >&2
				exit 1
			fi
			shift 2
			;;
		--i2c-bus)
			I2C_BUS="${2:-}"
			if [[ -z "${I2C_BUS}" ]]; then
				echo "--i2c-bus requires a value" >&2
				exit 1
			fi
			shift 2
			;;
		--addr)
			I2C_ADDR_HEX="${2:-}"
			if [[ -z "${I2C_ADDR_HEX}" ]]; then
				echo "--addr requires a value" >&2
				exit 1
			fi
			shift 2
			;;
		--no-bind)
			BIND_CLIENT=0
			shift
			;;
		--dry-run)
			DRY_RUN=1
			shift
			;;
		-h|--help)
			usage
			exit 0
			;;
		--*)
			echo "unknown option: $1" >&2
			usage >&2
			exit 1
			;;
		*)
			if [[ -n "${host}" ]]; then
				echo "unexpected argument: $1" >&2
				usage >&2
				exit 1
			fi
			host="$1"
			shift
			;;
		esac
	done

	if [[ -z "${host}" ]]; then
		usage >&2
		exit 1
	fi

	refresh_i2c_paths

	if [[ "${DRY_RUN}" -eq 0 ]]; then
		require_cmd gpioinfo
		require_cmd gpioset
		require_cmd gpiodetect
	fi

	apply_profile "${host}"
}

main "$@"
