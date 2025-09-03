/**
 *******************************************************************************
 * @file    template_module.c
 * @brief   [Short module description, e.g.] Implementation for Template Module
 * @details This file implements functions declared in `template_module.h`.
 *          Structured for maintainable and modular embedded C development.
 *
 * @date    2025/07/04
 * @author  [Your Name]
 * @version 1.0.0
 * @license MIT
 *******************************************************************************
 */

/* Includes ----------------------------------------------------------------- */
#include "sys_err.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(SYS_ERR, LOG_LEVEL_INF);

/* Private includes --------------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
/* Private macros ----------------------------------------------------------- */
/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

void sys_err_hardfault_handler(const char *file, int line, const char *func, unsigned int reason)
{
  /* Infinite loop */

  LOG_ERR("HardFault in %s at %s:%d, reason: 0x%08X", func, file, line, reason);

  k_sleep(K_MSEC(100)); // Allow log to flush

  while (1)
    ;
}

/* End of File -------------------------------------------------------------- */
