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

/* Private function prototypes ---------------------------------------------- */
static void                 hal_usb_init_cb(struct usbd_context *const ctx, const struct usbd_msg *msg);
static inline void          print_baudrate(const struct device *dev);
static void                 sample_fix_code_triple(struct usbd_context *uds_ctx, const enum usbd_speed speed);
static struct usbd_context *hal_usbd_setup_device(usbd_msg_cb_t msg_cb);

/* Exported functions ------------------------------------------------------- */
/* Private function definitions --------------------------------------------- */

/**
 * @brief  This function setups and initializes the USB device support.
 *
 * @return int
 */
int hal_usbd_init(void)
{
  int err = 0;

  if (hal_usbd_setup_device(hal_usb_init_cb) == NULL)
  {
    return -ENODEV;
  }

  err = usbd_init(&usbd_dev);
  if (err)
  {
    LOG_ERR("Failed to initialize device support");
    return -ENODEV;
  }

  err = hal_usbd_enable();
  if (err)
  {
    return err;
  }

  return err;
}

/**
 * @brief
 *
 * @return int
 */
int hal_usbd_enable(void)
{
  int err;

  if (!usbd_can_detect_vbus(&usbd_dev))
  {
    err = usbd_enable(&usbd_dev);
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
 */
int hal_usbd_disable(void)
{
  int err;

  err = usbd_disable(&usbd_dev);
  if (err)
  {
    LOG_ERR("Failed to disable device support");
    return err;
  }

  LOG_INF("USB device support disabled");

  return 0;
}

/**
 * @brief
 *
 */
void hal_usbd_deinit(void)
{
  (void)hal_usbd_disable();
  usbd_shutdown(&usbd_dev);
  LOG_INF("USB device support deinitialized");
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
static struct usbd_context *hal_usbd_setup_device(usbd_msg_cb_t msg_cb)
{
  int err;

  /* Add descriptors */
  struct usbd_desc_node *descs[] = {
    &sample_lang,
    &sample_mfr,
    &sample_product,
#ifdef CONFIG_HWINFO
    &sample_sn,
#endif
  };

  for (size_t i = 0; i < ARRAY_SIZE(descs); i++)
  {
    err = usbd_add_descriptor(&usbd_dev, descs[i]);
    if (err)
    {
      LOG_ERR("Failed to initialize descriptor %zu (%d)", i, err);
      return NULL;
    }
  }

  /* Add configurations */

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

  err = usbd_add_configuration(&usbd_dev, USBD_SPEED_FS, &sample_fs_config);
  if (err)
  {
    LOG_ERR("Failed to add Full-Speed configuration");
    return NULL;
  }

  err = usbd_register_all_classes(&usbd_dev, USBD_SPEED_FS, 1);
  if (err)
  {
    LOG_ERR("Failed to add register classes");
    return NULL;
  }

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

  return &usbd_dev;
}

/* End of File -------------------------------------------------------------- */
