#!/usr/bin/env python3

import quick_analyzer

res = quick_analyzer.stream_operator(lambda log : (log.submit_time,log.done_time))
short_res = res[int(len(res)/3):int(2*len(res)/3)]

max_done = -1
min_start = 99999999999999999999

for start,done in short_res:
    if start < min_start:
        min_start = start
    if done > max_done:
        max_done = done

print(1000*len(short_res) / (max_done - min_start))
