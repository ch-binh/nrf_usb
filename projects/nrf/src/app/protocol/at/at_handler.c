/**
 *******************************************************************************
 * @file    at_cmd.c
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
#include "at_handler.h"
#include <zephyr/logging/log.h>

/* Private includes --------------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
#define TAG "ATHDL"
LOG_MODULE_REGISTER(ATHDL, LOG_LEVEL_INF);

/* Private macros ----------------------------------------------------------- */

/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

AT_HDL_FUNC(AT_ID)
{
  LOG_INF("AT_ID handler called");

  memcpy(h->cmd.buf.out, "AT+OK\r\n", strlen("AT+OK\r\n"));
  return;
}

AT_HDL_FUNC(AT_VER)
{
  LOG_INF("AT_VER handler called");

  memcpy(h->cmd.buf.out, "AT+OK\r\n", strlen("AT+OK\r\n"));
  return;
}

AT_HDL_FUNC(AT_TEST)
{

  LOG_INF("AT_TEST handler called");

  memcpy(h->cmd.buf.out, "AT+OK\r\n", strlen("AT+OK\r\n"));
  return;
}

/* End of File -------------------------------------------------------------- */
