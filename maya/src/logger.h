#pragma once
#include "spdlog/logger.h"

std::shared_ptr<spdlog::logger> standard_logger();

/*!
	When to use which logging level:
	critical: Some fatal error occured which causes the application to immediately terminate. There is no possibility to catch the error and get along with it.
	error: Some error occurred which prevents the current workflow from going on but might be catched and handled in some super routine. It is recommended to catch and handle the error, but it is allowed to not do it.
	warning: Some circumstances occurred that are unexpected or just not optimal and might be fixed, but the main current workflow can go on without havy exception handling.
	info: Some ordinary thing happened which does not happen often and marks some greater step that are of interest when understanding the overall workflow. Use this rarely to not spam the log. Thumb rule: On the long run each instance that uses info logging should not log more than 5 items per minute in normal usage scenarios.
	debug: Everything like an info can be logged, additionally also smaller steps that are not of interest when just looking at the overall workflow. Should not log extremely fast changing / often occurring events. Should not log events that are very difficult to interpret or where recognizing the meaning requires looking into the code.
	trace: May log more than 1 item per second and logging instance, may log data that can only be useful when having a deeper look into the code to understand further meaning.
*/

void init_logger();
