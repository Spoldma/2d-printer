#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include "types.h"

namespace CommandDispatcher {
StatusCode execute(const Command &command);
}  // namespace CommandDispatcher

#endif
