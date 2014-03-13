#!/bin/sh

./prefab_compiler.exe ../build/data/prefabs.db `find ../src/art/prefabs | grep .prfb`
