#!/bin/bash

PATH_ROOT=`dirname "$0"`
PATH_ROOT=`(cd "$PATH_ROOT" && pwd)`


cd $PATH_ROOT

if [ -e hotkey ]; then
    cd hotkey
    git pull

else
    git clone --depth 1 https://github.com/Skycoder42/QHotkey.git hotkey
fi
