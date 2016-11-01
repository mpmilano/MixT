
def bin_things(num_bins,times):
    times.sort()
    min=times[0]
    max=times[-1]
    bin_increment = (max-min)/num_bins
    bins=[]
    for i in range(0,num_bins):
        bins.append([])
    for time in times:
        index = int((time - min)/bin_increment)
        if index == num_bins: index = (num_bins-1)
        bins[index].append(time)

    for bin in bins:
        if len(bin) > 0:
            print("fall in range " + str(bin[0]) + "-" + str(bin[-1]) +": " + str(len(bin)))
