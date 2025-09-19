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
#include <zephyr/logging/log.h>

#include "at_cmd.h"
#include "at_handler.h"

/* Private includes --------------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */

#define TAG "ATCMD"
LOG_MODULE_REGISTER(ATCMD, LOG_LEVEL_INF);

#define AT_INTERNAL_BUFFER_SIZE (256)

/* Private macros ----------------------------------------------------------- */
#define AT_HANDLER_NAME(id) AT_HDL_##id

/* Private typedefs --------------------------------------------------------- */
/* Public variables --------------------------------------------------------- */
at_hdl_t g_at_hdl = {
  .state = AT_IDLE,
  .cmd = {
      .id  = AT_UNKNOWN,
      .seq = 0,
      .ret = 0,
      .time = 0,
  },
};

/* Private variables -------------------------------------------------------- */

/* AT Table =============================================================== { */
#define AT_CMD(id, label, prop) [id] = { label, #prop, AT_HDL_##id }

static const at_cmd_t at_tbl[AT_NUM_CMD] = {
  AT_CMD(AT_ID, "ID", "R"),
  AT_CMD(AT_VER, "VER", "R"),
  AT_CMD(AT_TEST, "TEST", "R"),
};

#undef AT_CMD

/* AT Table =============================================================== } */

/* Private function prototypes ---------------------------------------------- */
static at_cmd_id_e at_cmd_parser(at_hdl_t *h, uint8_t *msg, uint8_t len);

/* Exported functions ------------------------------------------------------- */

void at_init(void)
{
}

void at_deinit(void)
{
}

/**
 * @brief
 *
 * @note      This function assumes that application recognizes this as a AT command (starts with "AT")
 *
 * @param[in] data
 * @param[in] len
 */
void at_process(at_hdl_t *h, uint8_t *data, uint8_t len)
{
  at_cmd_id_e cmd_id;

  /* Processing input */
  cmd_id = at_cmd_parser(h, data, len);

  /* Call callback upon getting message */

  if (cmd_id < AT_NUM_CMD)
  {
    at_tbl[cmd_id].cb(h);
  }
  else
  {
    memcpy(h->cmd.buf.out, "AT+SYNTAX\r\n", strlen("AT+SYNTAX\r\n"));
  }

  LOG_INF("AT command processed, response: %s", h->cmd.buf.out);
}

#warning "update this function"
/**
 * @brief
 *
 * @param[in] h
 * @param[in] buf
 * @param[in] len
 */
void at_get_resp(at_hdl_t *h, char *buf, uint16_t *len)
{
  if (h->cmd.buf.out[0] == 0)
  {
    *len = 0;
    return;
  }

  *len = strlen(h->cmd.buf.out);
  memcpy(buf, h->cmd.buf.out, *len);
}

/*
 * Private function definitions
 * =============================================================================
 */

/**
 * @brief Parse AT command
 *
 * @note  Th
 *
 * @param[in] msg
 * @param[in] len
 * @return at_cmd_id_e
 */
static at_cmd_id_e at_cmd_parser(at_hdl_t *h, uint8_t *msg, uint8_t len)
{
  at_cmd_id_e cmd_id = AT_UNKNOWN;

  /* Input validation */
  memset(h->cmd.buf.in, 0, AT_INTERNAL_BUFFER_SIZE);
  memcpy(h->cmd.buf.in, msg, len);
  LOG_INF("AT process called: %s (%d bytes)\n", h->cmd.buf.in, len);

  /* Get cmd */
  for (int i = 0; i < AT_NUM_CMD; i++)
  {
    if (strncmp((char *)h->cmd.buf.in + 3, at_tbl[i].label, strlen(at_tbl[i].label)) == 0)
    {
      cmd_id = (at_cmd_id_e)i;
      break;
    }
  }

  /* Get message */
  return cmd_id;
}

/* End of File -------------------------------------------------------------- */
