#!/bin/bash
ps -ef | grep -i $* | grep -v grep | awk '{print $5}' | xargs ./hours_minutes_to_minutes.sh
