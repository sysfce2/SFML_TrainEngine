#!/usr/bin/env bash

cd "$(dirname "$0")"

cd ..

rm -rf build/visual_studio_windows64/*
rm -rf build/visual_studio_windows32/*
rm -rf build/codeblocks_windows64/*
rm -rf build/codeblocks_windows32/*
rm -rf build/xcode/*
rm -rf build/codeblocks_linux/*
