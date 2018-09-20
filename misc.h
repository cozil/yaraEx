#pragma once

enum class LL : int
{
	Error,
	Warning,
	Message,
	Debug,
	Max
};

void set_log_level(LL level);
void logprintf(LL level, const char * format, ...);
void logputs(LL level, const char * text);
void print_usages(const char * cmdname = nullptr);