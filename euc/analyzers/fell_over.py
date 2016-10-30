#!/usr/bin/env python3

import quick_analyzer

times = quick_analyzer.stream_operator(lambda log : log.done_time - log.submit_time)
times.sort()
min=times[0]
max=times[-1]
num_bins = 10
bin_increment = (max-min)/num_bins
bins=[]
for i in range(0,num_bins):
    bins.append([])
for time in times:
    index = int((time - min)/bin_increment)
    if index == 10: index = 9
    bins[index].append(time)

for bin in bins:
    print("fall in range " + str(bin[0]) + "-" + str(bin[-1]) +": " + str(len(bin)))


