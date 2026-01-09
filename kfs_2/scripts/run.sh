#!/bin/bash
# **************************************************************************** #
#                                                                              #
#   KFS_1 - Kernel From Scratch                                                #
#                                                                              #
#   run.sh - Quick run script                                                  #
#                                                                              #
# **************************************************************************** #

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
KERNEL="$PROJECT_DIR/build/kernel.bin"
ISO="$PROJECT_DIR/kfs_1.iso"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

print_help() {
    echo -e "${CYAN}KFS_1 Runner${NC}"
    echo ""
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  -k, --kernel     Run kernel directly with QEMU"
    echo "  -K, --kernel-kvm Run kernel directly with KVM acceleration"
    echo "  -i, --iso        Boot from ISO image"
    echo "  -I, --iso-kvm    Boot from ISO with KVM acceleration"
    echo "  -d, --debug      Run with GDB server (port 1234)"
    echo "  -D, --debug-kvm  Debug with KVM acceleration"
    echo "  -b, --build      Build kernel only"
    echo "  -c, --check      Check kernel binary"
    echo "  -h, --help       Show this help"
    echo ""
    echo "Default: run kernel with KVM if available, else software emulation"
}

check_kvm() {
    if [ -r /dev/kvm ]; then
        return 0
    else
        return 1
    fi
}

build_kernel() {
    echo -e "${YELLOW}Building kernel...${NC}"
    cd "$PROJECT_DIR"
    make all
    echo -e "${GREEN}Build complete!${NC}"
}

build_iso() {
    echo -e "${YELLOW}Building ISO...${NC}"
    cd "$PROJECT_DIR"
    make iso
    echo -e "${GREEN}ISO created!${NC}"
}

run_kernel() {
    local use_kvm=$1
    
    if [ ! -f "$KERNEL" ]; then
        build_kernel
    fi
    
    if [ "$use_kvm" = "true" ] && check_kvm; then
        echo -e "${GREEN}Running with KVM acceleration...${NC}"
        qemu-system-i386 -enable-kvm -kernel "$KERNEL" -m 32M
    else
        echo -e "${YELLOW}Running with software emulation...${NC}"
        qemu-system-i386 -kernel "$KERNEL" -m 32M
    fi
}

run_iso() {
    local use_kvm=$1
    
    if [ ! -f "$ISO" ]; then
        build_iso
    fi
    
    if [ "$use_kvm" = "true" ] && check_kvm; then
        echo -e "${GREEN}Booting ISO with KVM...${NC}"
        qemu-system-i386 -enable-kvm -cdrom "$ISO" -m 32M
    else
        echo -e "${YELLOW}Booting ISO with software emulation...${NC}"
        qemu-system-i386 -cdrom "$ISO" -m 32M
    fi
}

run_debug() {
    local use_kvm=$1
    
    if [ ! -f "$KERNEL" ]; then
        build_kernel
    fi
    
    echo -e "${CYAN}Starting QEMU with GDB server on port 1234...${NC}"
    echo -e "${YELLOW}Connect with: gdb -ex 'target remote :1234' $KERNEL${NC}"
    
    if [ "$use_kvm" = "true" ] && check_kvm; then
        qemu-system-i386 -enable-kvm -kernel "$KERNEL" -m 32M -s -S
    else
        qemu-system-i386 -kernel "$KERNEL" -m 32M -s -S
    fi
}

# Parse arguments
case "${1:-}" in
    -k|--kernel)
        run_kernel false
        ;;
    -K|--kernel-kvm)
        run_kernel true
        ;;
    -i|--iso)
        run_iso false
        ;;
    -I|--iso-kvm)
        run_iso true
        ;;
    -d|--debug)
        run_debug false
        ;;
    -D|--debug-kvm)
        run_debug true
        ;;
    -b|--build)
        build_kernel
        ;;
    -c|--check)
        cd "$PROJECT_DIR"
        make check
        ;;
    -h|--help)
        print_help
        ;;
    "")
        # Default: try KVM, fallback to software
        run_kernel true
        ;;
    *)
        echo -e "${RED}Unknown option: $1${NC}"
        print_help
        exit 1
        ;;
esac
