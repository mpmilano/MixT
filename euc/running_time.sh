#!/bin/bash
echo $[`date +%H:%M | xargs ./hours_minutes_to_minutes.sh ` - `./time_launched.sh $*`]
