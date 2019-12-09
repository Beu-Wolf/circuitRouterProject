#!/bin/bash

cpuInfo=$(lscpu)

modelName=$(echo "${cpuInfo}" | grep -E '^Model\sname:')
architecture=$(echo "${cpuInfo}" | grep -E '^Architecture:')
ncores=$(echo "${cpuInfo}" | grep -E '^CPU\(s\)')
frequency=$(echo "${cpuInfo}" | grep -E '^CPU\s+(\w{3}\s+)?MHz')

echo "Operating System:      $(uname -sv)"
echo "${modelName}"
echo "${architecture}"
echo "${ncores}"
echo "${frequency}"
