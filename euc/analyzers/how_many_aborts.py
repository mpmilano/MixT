#!/usr/bin/env python3

import quick_analyzer
from bin_things import bin_things

print('number aborts: ')
print(len([x for x in quick_analyzer.stream_operator(lambda log : log.is_serialization_error) if x]))
print('number commits ')
print(len([x for x in quick_analyzer.stream_operator(lambda log : log.is_serialization_error) if not x]))
