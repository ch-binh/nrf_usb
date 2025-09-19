/**
 ******************************************************************************
 * @file    at_cmd.h
 * @brief   [Short module description, e.g.] Interface for Template Math Module
 * @details This file provides declarations for configuration macros, types,
 *          and public API for the Template module. Designed for embedded use.
 *
 * @date    2025/07/04
 * @author  [Your Name]
 * @version 1.0.0
 * @license MIT
 ******************************************************************************
 * @attention
 * This is a template header file. Replace placeholder content before use.
 ******************************************************************************
 */

#ifndef __AT_HANDLER_H
#define __AT_HANDLER_H

/* Includes ----------------------------------------------------------------- */
/* Standard libraries */
/* Project-specific headers */
#include "at_cmd.h"

/* Configuration ------------------------------------------------------------ */
/* Public defines ----------------------------------------------------------- */
/* Public macros ------------------------------------------------------------ */

/* Helper macros ------------------------------------------------------------ */
#define AT_HDL_FUNC(id) void AT_HDL_##id(at_hdl_t *h)

/* Public typedefs ---------------------------------------------------------- */
/* Enumerations ------------------------------------------------------------- */
/* Structures --------------------------------------------------------------- */
/* Error codes -------------------------------------------------------------- */
/* Function prototypes ------------------------------------------------------ */

AT_HDL_FUNC(AT_ID);
AT_HDL_FUNC(AT_VER);
AT_HDL_FUNC(AT_TEST);

/* Inline functions --------------------------------------------------------- */

#endif /* __AT_CMD_H */

/* End of File -------------------------------------------------------------- */
