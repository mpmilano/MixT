#!/usr/bin/env python3

import quick_analyzer
from bin_things import bin_things

abort_list = [(a,b) for (a,b) in quick_analyzer.stream_operator(lambda log : (log.is_serialization_error, log.remote_failure_string)) if a]
print('number aborts: ')
print(len(abort_list))
print('number commits ')
print(len([x for x in quick_analyzer.stream_operator(lambda log : log.is_serialization_error) if not x]))
print('abort messages: ')
for (ignore,message) in abort_list:
    print(message)


