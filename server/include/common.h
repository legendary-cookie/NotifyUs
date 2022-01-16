#pragma once

#include <eventpp/eventdispatcher.h>

extern eventpp::EventDispatcher<int, void(const std::string &)> dispatcher;
