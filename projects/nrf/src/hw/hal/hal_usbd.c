/**
 *******************************************************************************
 * @file    hal_usbd.c
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

#include "hal_usbd.h"
#include "main.h"

#include <sample_usbd.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include <zephyr/drivers/uart.h>

/* Private includes --------------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
LOG_MODULE_REGISTER(hal_usbd, LOG_LEVEL_INF);

/* Private macros ----------------------------------------------------------- */
/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
static struct usbd_context *sample_usbd;

/* Private function prototypes ---------------------------------------------- */
static int         enable_usb_device_next(void);
static void        hal_usb_init_cb(struct usbd_context *const ctx, const struct usbd_msg *msg);
static inline void print_baudrate(const struct device *dev);

/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

void hal_usbd_init(void)
{
  return;
}

int hal_usbd_enable(void)
{
  int ret;

  ret = enable_usb_device_next();

  return ret;
}

/*
 *
 * STATIC FUNCTIONS
 * =============================================================================
 *
 */

/**
 * @brief
 *
 * @return int
 */
static int enable_usb_device_next(void)
{
  int err;

  sample_usbd = sample_usbd_init_device(hal_usb_init_cb);
  if (sample_usbd == NULL)
  {
    LOG_ERR("Failed to initialize USB device");
    return -ENODEV;
  }

  if (!usbd_can_detect_vbus(sample_usbd))
  {
    err = usbd_enable(sample_usbd);
    if (err)
    {
      LOG_ERR("Failed to enable device support");
      return err;
    }
  }

  LOG_INF("USB device support enabled");

  return 0;
}

/**
 * @brief
 *
 * @param[in] ctx
 * @param[in] msg
 */
static void hal_usb_init_cb(struct usbd_context *const ctx, const struct usbd_msg *msg)
{
  LOG_INF("USBD message: %s", usbd_msg_type_string(msg->type));

  if (usbd_can_detect_vbus(ctx))
  {
    if (msg->type == USBD_MSG_VBUS_READY)
    {
      if (usbd_enable(ctx))
      {
        LOG_ERR("Failed to enable device support");
      }
    }

    if (msg->type == USBD_MSG_VBUS_REMOVED)
    {
      if (usbd_disable(ctx))
      {
        LOG_ERR("Failed to disable device support");
      }
    }
  }

  if (msg->type == USBD_MSG_CDC_ACM_CONTROL_LINE_STATE)
  {
    uint32_t dtr = 0U;

    uart_line_ctrl_get(msg->dev, UART_LINE_CTRL_DTR, &dtr);
    if (dtr)
    {
      k_sem_give(&dtr_sem);
    }
  }

  if (msg->type == USBD_MSG_CDC_ACM_LINE_CODING)
  {
    print_baudrate(msg->dev);
  }
}

static inline void print_baudrate(const struct device *dev)
{
  uint32_t baudrate;
  int      ret;

  ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
  if (ret)
  {
    LOG_WRN("Failed to get baudrate, ret code %d", ret);
  }
  else
  {
    LOG_INF("Baudrate %u", baudrate);
  }
}

/* End of File -------------------------------------------------------------- */
