#pragma once

#define do_op(Name,arg...) do_op<RegisteredOperations::Name>(arg)
#define supports_operation(Name_and_types...) SupportedOperation<RegisteredOperations::Name_and_types>
