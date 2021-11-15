// 1. BecomeDaemon
// 2. Create PID file in etc? => for controlling daemon & unique daemon
// 3. If daemon is unique (true) => set signals handler => it is possible to control the process
// 4. 

#include "daemon.h"

enum copy_type COPY_TYPE = SHALLOW_COPY;

