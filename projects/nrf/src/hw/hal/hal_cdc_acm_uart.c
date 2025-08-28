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
#include <zephyr/logging/log.h>

/* Private includes --------------------------------------------------------- */
#include "hal_cdc_acm_uart.h"

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>

LOG_MODULE_REGISTER(COM_cb, LOG_LEVEL_INF);

/* Private defines ---------------------------------------------------------- */
#define RING_BUF_SIZE (1024)

/* Private macros ----------------------------------------------------------- */
/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
const struct device *const uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
uint8_t                    ring_buffer[RING_BUF_SIZE];
struct ring_buf            ringbuf;
static bool                rx_throttled;

/* Private function prototypes ---------------------------------------------- */
static void interrupt_handler(const struct device *dev, void *user_data);

/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

void hal_uart_init(void)
{
  if (!device_is_ready(uart_dev))
  {
    LOG_ERR("CDC ACM device not ready");
  }

  /* Init ring buf */
  ring_buf_init(&ringbuf, sizeof(ring_buffer), ring_buffer);
}

void hal_uart_set_irq(void)
{

  int ret;

  /* They are optional, we use them to test the interrupt endpoint */
  ret = uart_line_ctrl_set(uart_dev, UART_LINE_CTRL_DCD, 1);
  if (ret)
  {
    LOG_WRN("Failed to set DCD, ret code %d", ret);
  }

  /* Wait 100ms for the host to do all settings */
  k_msleep(100);

  uart_irq_callback_set(uart_dev, interrupt_handler);

  /* Enable rx interrupts */
  uart_irq_rx_enable(uart_dev);
}

void hal_uart_deinit(void)
{
  return;
}

/*
 * STATIC FUNCTION
 * =============================================================================
 */

static void interrupt_handler(const struct device *dev, void *user_data)
{
  ARG_UNUSED(user_data);

  while (uart_irq_update(dev) && uart_irq_is_pending(dev))
  {
    if (!rx_throttled && uart_irq_rx_ready(dev))
    {
      int     recv_len, rb_len;
      uint8_t buffer[64];
      size_t  len = MIN(ring_buf_space_get(&ringbuf), sizeof(buffer));

      if (len == 0)
      {
        /* Throttle because ring buffer is full */
        uart_irq_rx_disable(dev);
        rx_throttled = true;
        continue;
      }

      recv_len = uart_fifo_read(dev, buffer, len);
      if (recv_len < 0)
      {
        LOG_ERR("Failed to read UART FIFO");
        recv_len = 0;
      };

      rb_len = ring_buf_put(&ringbuf, buffer, recv_len);
      if (rb_len < recv_len)
      {
        LOG_ERR("Drop %u bytes", recv_len - rb_len);
      }

      LOG_DBG("tty fifo -> ringbuf %d bytes", rb_len);
      if (rb_len)
      {
        uart_irq_tx_enable(dev);
      }
    }

    if (uart_irq_tx_ready(dev))
    {
      uint8_t buffer[64];
      int     rb_len, send_len;

      rb_len = ring_buf_get(&ringbuf, buffer, sizeof(buffer));
      if (!rb_len)
      {
        LOG_DBG("Ring buffer empty, disable TX IRQ");
        uart_irq_tx_disable(dev);
        continue;
      }

      if (rx_throttled)
      {
        uart_irq_rx_enable(dev);
        rx_throttled = false;
      }

      send_len = uart_fifo_fill(dev, buffer, rb_len);
      if (send_len < rb_len)
      {
        LOG_ERR("Drop %d bytes", rb_len - send_len);
      }

      LOG_DBG("ringbuf -> tty fifo %d bytes", send_len);
    }
  }
}

/* End of File -------------------------------------------------------------- */
