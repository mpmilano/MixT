#!/bin/bash
[[ `free -m | awk '{print $7}' | grep [0-9]` -lt 800 ]]
