
# Initial environment bring-up for Lab 1.1

This file lists the command-line checks used for Lab 1.1. The commands are intended to be executed on the assigned lab machine.

The exact command used to load the Xilinx environment is provided by the instructors. Do not assume that the installation path is the same on every machine.

## 1. Identify the active session

```bash
hostname
whoami
pwd
echo "$SHELL"
```

Expected use: record the machine, user, current working directory, and shell.

## 3. Move to the lab working directory

```bash
cd <lab-working-directory>
pwd
```

Replace `<lab-working-directory>` with the directory specified by the instructors.

The working directory must be writable. Avoid running Xilinx tools from the tool installation directory. Our recommendation is to adding all the lab relevant files in one complete directory such as "labs/" and for each lab including directories for lab1, lab2. This is useful for avoiding any incomplete lab submissions.

## 4. Load the selected Xilinx environment

The Xilinx suite usually stays under /Software directory. The selected tool version must match the version announced for the lab. These guides are tested on 2022.2 but for other Xilinx versions, commands should be identical. 

## 4. Check tool availability

```bash
command -v vivado || true
command -v vitis_hls || true
command -v vitis || true
```

At minimum, Vivado, Vitis HLS, and Vitis should resolve after the Xilinx environment is loaded.

## 5. Check tool versions

```bash
vivado -version | head -n 5
vitis_hls -version | head -n 5
vitis -version | head -n 5
```

The reported versions should match the selected lab version.

## Check GUI forwarding if needed

```bash
echo "$DISPLAY"
xdpyinfo >/dev/null 2>&1 && echo "x11 works" || echo "x11 failed"
```

If X11 fails, record the result and continue with command-line checks where possible.

## Optional GUI launch for Xilinx applications

Use this script only instructed when the GUI workflow or X11 forwarding is required:

```bash
cat > ~/bin/xilinx-x11 <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

XILINX_VERSION="2022.2"
XILINX_ROOT="/Software/xilinx/${XILINX_VERSION}"

VIVADO_SETTINGS="${XILINX_ROOT}/Vivado/${XILINX_VERSION}/settings64.sh"
VITIS_SETTINGS="${XILINX_ROOT}/Vitis/${XILINX_VERSION}/settings64.sh"
HLS_SETTINGS="${XILINX_ROOT}/Vitis_HLS/${XILINX_VERSION}/settings64.sh"

usage() {
  cat <<USAGE
Usage:
  xilinx-x11 check
  xilinx-x11 env
  xilinx-x11 vivado [args...]
  xilinx-x11 vitis [args...]
  xilinx-x11 hls [args...]

Examples:
  xilinx-x11 check
  xilinx-x11 vivado
  xilinx-x11 vitis
  xilinx-x11 hls
  source <(xilinx-x11 env)
USAGE
}

require_file() {
  local file="$1"

  if [ ! -f "$file" ]; then
    echo "missing: $file" >&2
    exit 1
  fi
}

load_all() {
  require_file "$VIVADO_SETTINGS"
  require_file "$VITIS_SETTINGS"
  require_file "$HLS_SETTINGS"

  source "$VIVADO_SETTINGS"
  source "$VITIS_SETTINGS"
  source "$HLS_SETTINGS"
}

check_tool() {
  local tool="$1"

  printf "\n[%s]\n" "$tool"

  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "not found"
    return
  fi

  command -v "$tool"

  case "$tool" in
    vivado|vitis|vitis_hls)
      "$tool" -version 2>&1 | head -n 5
      ;;
    xsct)
      echo "xsct found"
      ;;
  esac
}

cmd="${1:-}"

case "$cmd" in
  check)
    echo "[xilinx ${XILINX_VERSION}]"
    echo "root: $XILINX_ROOT"

    echo
    echo "[settings scripts]"
    require_file "$VIVADO_SETTINGS"
    echo "ok: $VIVADO_SETTINGS"

    require_file "$VITIS_SETTINGS"
    echo "ok: $VITIS_SETTINGS"

    require_file "$HLS_SETTINGS"
    echo "ok: $HLS_SETTINGS"

    load_all

    check_tool vivado
    check_tool vitis_hls
    check_tool vitis
    check_tool xsct

    echo
    echo "[x11]"
    echo "DISPLAY=${DISPLAY:-}"
    if command -v xdpyinfo >/dev/null 2>&1; then
      xdpyinfo >/dev/null 2>&1 && echo "x11 works" || echo "x11 failed"
    else
      echo "xdpyinfo not found"
    fi
    ;;

  env)
    require_file "$VIVADO_SETTINGS"
    require_file "$VITIS_SETTINGS"
    require_file "$HLS_SETTINGS"

    cat <<ENV
source "$VIVADO_SETTINGS"
source "$VITIS_SETTINGS"
source "$HLS_SETTINGS"
ENV
    ;;

  vivado)
    shift
    require_file "$VIVADO_SETTINGS"
    source "$VIVADO_SETTINGS"
    exec vivado "$@"
    ;;

  vitis)
    shift
    require_file "$VITIS_SETTINGS"
    source "$VITIS_SETTINGS"
    exec vitis "$@"
    ;;

  hls|vitis_hls)
    shift
    require_file "$HLS_SETTINGS"
    source "$HLS_SETTINGS"
    exec vitis_hls "$@"
    ;;

  ""|-h|--help|help)
    usage
    ;;

  *)
    echo "unknown command: $cmd" >&2
    echo >&2
    usage >&2
    exit 1
    ;;
esac
EOF

chmod +x ~/bin/xilinx-x11
```

Close the applications after verifying that they start correctly.

## Useful diagnostic commands

```bash
find . -maxdepth 3 -type f \( -name "*.log" -o -name "*.jou" -o -name "*.rpt" \) | sort
grep -R "ERROR:" . 2>/dev/null || true
grep -R "CRITICAL WARNING:" . 2>/dev/null || true
```

Use these commands when a Xilinx command fails and log files are generated in the working directory.


```bash
ssh -Y <your_U_identifier>@<hostname_with_kit_domain>
```

The -Y parameter for ssh enables trusted X11 forwarding. This allows GUI applications on the remote machine to send windows to your local X server.


If your tool is opening but not opens the expected version:

```bash
vivado -version | head -n 5
vitis -version | head -n 5
vitis_hls -version | head -n 5
```

No hardware target found when board plugged in, the possible causes may be:

The board is not connected.
No USB/JTAG access.
Another user or process is using the board.
An attempt is being made to access the board from the wrong machine.

```bash
command -v hw_server || true
```

