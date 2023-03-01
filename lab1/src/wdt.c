#include "wdt.h"

void wdt_restart() {
  *PM_WDOG = PM_PASSWORD | RESTART_TICK;
  *PM_RSTC = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
}
