#!/usr/bin/env python3

import quick_analyzer
from bin_things import bin_things

bin_things(10,quick_analyzer.stream_operator(lambda log : log.done_time - log.submit_time))
