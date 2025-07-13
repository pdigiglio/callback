#!/bin/bash

typeset -r script_folder="$(realpath "$(dirname "$0")")"
typeset -r buid_type="Debug"

function main() {
    typeset -r rel_buid_folder="build_$buid_type"
    typeset -r abs_buid_folder="$script_folder/$rel_buid_folder"

    mkdir -p "$abs_buid_folder"
    cd "$abs_buid_folder"
    cmake "$script_folder" \
        -DCMAKE_BUILD_TYPE="$abs_buid_folder" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    typeset -r compile_db="compile_commands.json"
    cd "$script_folder"
    rm --force "$compile_db"
    ln -s "$rel_buid_folder/$compile_db" "$compile_db"
}

main
