#!/usr/bin/env python3

import quick_analyzer
from bin_things import bin_things

full_list = quick_analyzer.stream_operator(lambda log : log.remote_failure_string)
abort_list = [a for a in full_list if len(a) > 2 ]
print('number aborts: ')
print(len(abort_list))
print('number commits ')
print(len(full_list) - len(abort_list))
print('abort messages: ')
for message in abort_list:
    print(message)


