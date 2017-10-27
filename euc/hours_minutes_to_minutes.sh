#!/bin/bash
echo $* | tr ':' ' ' | awk '{print $1*60 + $2}'
