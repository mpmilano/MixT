#!/usr/bin/env python3

import quick_analyzer
from bin_things import bin_things

bin_things(10,quick_analyzer.stream_operator(lambda log : log.run_time - log.submit_time if log.submit_time < (480000/2) else -1))
#bin_things(10,quick_analyzer.stream_operator(lambda log : log.submit_time))

