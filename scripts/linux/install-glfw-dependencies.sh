#!/bin/bash

set -e

# Function to install packages on Ubuntu/Debian
install_ubuntu_debian() {
    sudo apt-get update
    sudo apt-get install -y libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev
}

# Function to install packages on Fedora
install_fedora() {
    sudo dnf install -y libX11-devel libXcursor-devel libXrandr-devel libXinerama-devel libXi-devel
}

# Function to install packages on CentOS/RHEL
install_centos_rhel() {
    sudo yum install -y libX11-devel libXcursor-devel libXrandr-devel libXinerama-devel libXi-devel
}

# Function to install packages on Arch
install_arch() {
    sudo pacman -S --noconfirm libx11 libxcursor libxrandr libxinerama libxi
}

# Detect the distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    case "$ID" in
        ubuntu|debian|linuxmint)
            install_ubuntu_debian
            ;;
        fedora)
            install_fedora
            ;;
        centos|rhel)
            install_centos_rhel
            ;;
        arch)
            install_arch
            ;;
        *)
            echo "Unsupported distribution: $ID"
            exit 1
            ;;
    esac
else
    echo "Cannot detect the Linux distribution."
    exit 1
fi
