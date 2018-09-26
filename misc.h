#pragma once
#include "pluginmain.h"

enum class LL : int
{
	Error,
	Warning,
	Message,
	Debug,
	Max
};

void set_log_level(LL level);
int get_log_level();
void logprintf(LL level, const char * format, ...);
void logputs(LL level, const char * text);
void print_usages(const char * cmdname = nullptr);

bool eval(const char * expression, int& result);
bool eval(const char * expression, duint& result);
std::string eval_format(const char * text);

void str_fill(std::string& text, int maxlength, char ch);
void str_replace(std::string& str, const std::string& find, const char * replacedby);