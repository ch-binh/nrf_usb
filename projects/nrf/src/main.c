/**
 *******************************************************************************
 * @file
 * @brief Sample echo app for CDC ACM class
 *
 * Sample app for USB CDC ACM class driver. The received data is echoed back
 * to the serial port.
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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Private includes --------------------------------------------------------- */
#include "main.h"

#include "hal_usbd.h"
#include "hal_cdc_acm_uart.h"

LOG_MODULE_REGISTER(cdc_acm_echo, LOG_LEVEL_INF);

/* Private defines ---------------------------------------------------------- */

K_SEM_DEFINE(dtr_sem, 0, 1);

/* Private macros ----------------------------------------------------------- */
/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

int sys_init(void)
{
  int ret;

  /* Init UART*/
  hal_cdc_init();

  /* Init USBD */
  ret = hal_usbd_init();
  if (ret != 0)
  {
    LOG_ERR("Failed to init or enable USB");
    return 0;
  }

  return 1;
}

int main(void)
{
  LOG_INF("START");

  if (!sys_init())
  {
    LOG_ERR("Init module failed");
    return 0;
  }

  LOG_INF("Wait for DTR");

  k_sem_take(&dtr_sem, K_FOREVER);

  LOG_INF("DTR set");

  hal_cdc_enable_irq();

  // while (1)
  // {
  //   k_msleep(1000);
  // }

  return 0;
}

/* End of File -------------------------------------------------------------- */
