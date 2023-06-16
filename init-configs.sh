#!/usr/bin/env bash

# This script should only be run if you need to re-create all the configurations from scratch.

# This script should be run from the directory it is located in.

toolchain=$1
command=$2



# Stop on any error:
set -e

# Determine OS
if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
    platform=windows
else
    platform=unix
fi

# Delete all the previous configs.
if [[ $command = '--clean' ]]
then
    echo "Deleting previous configurations, install, etc..."
    rm -rf ./build-*/ ./install/ .bdep/
fi

echo "COMMAND = |$command|"
echo "OSTYPE = |$OSTYPE|"
echo "platform = |$platform|"

bdep init --empty

# Create all the configurations and initialize all the projects at the same time.
# The first configuration initialized will be considered the default config. We use Debug as default.
echo "Projects initialization in new configurations... (debug as default)"
install_config="config.install.root=./install/"
common_options="--options-file=./build2-configs/common.options"
platform_options="--options-file=./build2-configs/$platform.options"
bdep init --empty
bdep config create -V @$toolchain-debug build-$toolchain-debug     cc config.config.load=build2-configs/$toolchain-debug.config   $install_config$toolchain-debug   $common_options $platform_options
bdep config create -V @$toolchain-release build-$toolchain-release cc config.config.load=build2-configs/$toolchain-release.config $install_config$toolchain-release $common_options $platform_options
bdep init --all

echo "Configurations Ready:"
bdep config list

echo "Done. Use build2 to work."

