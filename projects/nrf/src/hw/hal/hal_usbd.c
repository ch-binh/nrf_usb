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

#include <stdint.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/bos.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

/* Private includes --------------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
LOG_MODULE_REGISTER(hal_usbd, LOG_LEVEL_INF);

#define ZEPHYR_PROJECT_USB_VID (0x2fe3)

USBD_DEVICE_DEFINE(usbd_dev, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), ZEPHYR_PROJECT_USB_VID,
                   CONFIG_SAMPLE_USBD_PID);
USBD_DESC_LANG_DEFINE(sample_lang);
USBD_DESC_MANUFACTURER_DEFINE(sample_mfr, CONFIG_SAMPLE_USBD_MANUFACTURER);
USBD_DESC_PRODUCT_DEFINE(sample_product, CONFIG_SAMPLE_USBD_PRODUCT);
IF_ENABLED(CONFIG_HWINFO, (USBD_DESC_SERIAL_NUMBER_DEFINE(sample_sn)));

USBD_DESC_CONFIG_DEFINE(fs_cfg_desc, "FS Configuration");
USBD_DESC_CONFIG_DEFINE(hs_cfg_desc, "HS Configuration");

static const uint8_t attributes = (IS_ENABLED(CONFIG_SAMPLE_USBD_SELF_POWERED) ? USB_SCD_SELF_POWERED : 0) |
                                  (IS_ENABLED(CONFIG_SAMPLE_USBD_REMOTE_WAKEUP) ? USB_SCD_REMOTE_WAKEUP : 0);

/* Full speed configuration */
USBD_CONFIGURATION_DEFINE(sample_fs_config, attributes, CONFIG_SAMPLE_USBD_MAX_POWER, &fs_cfg_desc);

/* High speed configuration */
USBD_CONFIGURATION_DEFINE(sample_hs_config, attributes, CONFIG_SAMPLE_USBD_MAX_POWER, &hs_cfg_desc);

/*
 * This does not yet provide valuable information, but rather serves as an
 * example, and will be improved in the future.
 */
static const struct usb_bos_capability_lpm bos_cap_lpm = {
  .bLength            = sizeof(struct usb_bos_capability_lpm),
  .bDescriptorType    = USB_DESC_DEVICE_CAPABILITY,
  .bDevCapabilityType = USB_BOS_CAPABILITY_EXTENSION,
  .bmAttributes       = 0UL,
};

USBD_DESC_BOS_DEFINE(sample_usbext, sizeof(bos_cap_lpm), &bos_cap_lpm);

/* Private macros ----------------------------------------------------------- */
/* Private typedefs --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
static struct usbd_context *usbd_ctx;

/* Private function prototypes ---------------------------------------------- */
static int                  enable_usb_device_next(void);
static void                 hal_usb_init_cb(struct usbd_context *const ctx, const struct usbd_msg *msg);
static inline void          print_baudrate(const struct device *dev);
static void                 sample_fix_code_triple(struct usbd_context *uds_ctx, const enum usbd_speed speed);
static struct usbd_context *sample_usbd_setup_device(usbd_msg_cb_t msg_cb);
static struct usbd_context *sample_usbd_init_device(usbd_msg_cb_t msg_cb);

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

  usbd_ctx = sample_usbd_init_device(hal_usb_init_cb);
  if (usbd_ctx == NULL)
  {
    LOG_ERR("Failed to initialize USB device");
    return -ENODEV;
  }

  if (!usbd_can_detect_vbus(usbd_ctx))
  {
    err = usbd_enable(usbd_ctx);
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

static void sample_fix_code_triple(struct usbd_context *uds_ctx, const enum usbd_speed speed)
{
  /* Always use class code information from Interface Descriptors */
  if (IS_ENABLED(CONFIG_USBD_CDC_ACM_CLASS) || IS_ENABLED(CONFIG_USBD_CDC_ECM_CLASS) ||
      IS_ENABLED(CONFIG_USBD_CDC_NCM_CLASS) || IS_ENABLED(CONFIG_USBD_AUDIO2_CLASS))
  {
    /*
     * Class with multiple interfaces have an Interface
     * Association Descriptor available, use an appropriate triple
     * to indicate it.
     */
    usbd_device_set_code_triple(uds_ctx, speed, USB_BCC_MISCELLANEOUS, 0x02, 0x01);
  }
  else
  {
    usbd_device_set_code_triple(uds_ctx, speed, 0, 0, 0);
  }
}

/**
 * @brief
 *
 * @param[in] msg_cb
 * @return struct usbd_context*
 */
static struct usbd_context *sample_usbd_setup_device(usbd_msg_cb_t msg_cb)
{
  int err;

  err = usbd_add_descriptor(&usbd_dev, &sample_lang);
  if (err)
  {
    LOG_ERR("Failed to initialize language descriptor (%d)", err);
    return NULL;
  }

  err = usbd_add_descriptor(&usbd_dev, &sample_mfr);
  if (err)
  {
    LOG_ERR("Failed to initialize manufacturer descriptor (%d)", err);
    return NULL;
  }

  err = usbd_add_descriptor(&usbd_dev, &sample_product);
  if (err)
  {
    LOG_ERR("Failed to initialize product descriptor (%d)", err);
    return NULL;
  }

  IF_ENABLED(CONFIG_HWINFO, (err = usbd_add_descriptor(&usbd_dev, &sample_sn);))
  if (err)
  {
    LOG_ERR("Failed to initialize SN descriptor (%d)", err);
    return NULL;
  }

  if (usbd_caps_speed(&usbd_dev) == USBD_SPEED_HS)
  {
    err = usbd_add_configuration(&usbd_dev, USBD_SPEED_HS, &sample_hs_config);
    if (err)
    {
      LOG_ERR("Failed to add High-Speed configuration");
      return NULL;
    }

    err = usbd_register_all_classes(&usbd_dev, USBD_SPEED_HS, 1);
    if (err)
    {
      LOG_ERR("Failed to add register classes");
      return NULL;
    }

    sample_fix_code_triple(&usbd_dev, USBD_SPEED_HS);
  }

  /* doc configuration register start */
  err = usbd_add_configuration(&usbd_dev, USBD_SPEED_FS, &sample_fs_config);
  if (err)
  {
    LOG_ERR("Failed to add Full-Speed configuration");
    return NULL;
  }
  /* doc configuration register end */

  /* doc functions register start */
  err = usbd_register_all_classes(&usbd_dev, USBD_SPEED_FS, 1);
  if (err)
  {
    LOG_ERR("Failed to add register classes");
    return NULL;
  }
  /* doc functions register end */

  sample_fix_code_triple(&usbd_dev, USBD_SPEED_FS);

  if (msg_cb != NULL)
  {
    /* doc device init-and-msg start */
    err = usbd_msg_register_cb(&usbd_dev, msg_cb);
    if (err)
    {
      LOG_ERR("Failed to register message callback");
      return NULL;
    }
    /* doc device init-and-msg end */
  }

  if (IS_ENABLED(CONFIG_SAMPLE_USBD_20_EXTENSION_DESC))
  {
    (void)usbd_device_set_bcd_usb(&usbd_dev, USBD_SPEED_FS, 0x0201);
    (void)usbd_device_set_bcd_usb(&usbd_dev, USBD_SPEED_HS, 0x0201);

    err = usbd_add_descriptor(&usbd_dev, &sample_usbext);
    if (err)
    {
      LOG_ERR("Failed to add USB 2.0 Extension Descriptor");
      return NULL;
    }
  }

  ERR_HARDFAULT(1);

  return &usbd_dev;
}

struct usbd_context *sample_usbd_init_device(usbd_msg_cb_t msg_cb)
{
  int err;

  if (sample_usbd_setup_device(msg_cb) == NULL)
  {
    return NULL;
  }

  /* doc device init start */
  err = usbd_init(&usbd_dev);
  if (err)
  {
    LOG_ERR("Failed to initialize device support");
    return NULL;
  }
  /* doc device init end */

  return &usbd_dev;
}

/* End of File -------------------------------------------------------------- */
