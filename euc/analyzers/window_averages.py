#moving window averages.  Calculates the moving-window averages for MyriaStore

#all times are in miliseconds! 
#argument: duration of window
#argument: step-duration of window
#argument: list of (start_time, finish_time, is_error) tuples.
#probably list of tuples; regardless must allow destructuring bind. + index-based accesses 
def moving_window(window_size, window_step, time_couples):
    #sort by completion time, and then filter out any integer overflows in the log
    sorted_by_completion_time = [elem for elem in sorted(time_couples, key=(lambda x : x[1])) if elem[1] > 0]
    max_done_time = sorted_by_completion_time[-1][1]

    start_index = 0
    window_start = 0

    print(sorted_by_completion_time)
    print(max_done_time)

    averages = []
    window_end=window_step
    while window_end <= max_done_time:
        try:
            total_latency = 0
            total_events = 0
            for (start_time,done_time,is_error) in sorted_by_completion_time:
                if done_time < window_start:
                    start_index += 1
                    continue
                if done_time > window_end:
                    break
                if is_error:
                    continue
                total_latency += done_time - submit_time
                total_events += 1
            if total_events == 0:
                continue
            else:
                averages.append(tuple(total_events,total_latency))
        finally:
            window_start += window_step
            window_end += window_step
    return averages
