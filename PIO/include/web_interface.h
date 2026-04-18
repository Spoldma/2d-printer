#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

#include "types.h"

namespace WebInterface {
typedef StatusCode (*CommandExecutor)(const String &line, const char *sourceTag);

void begin(CommandExecutor executor);
void handleClient();
}  // namespace WebInterface

#endif
