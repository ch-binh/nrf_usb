/**
 ******************************************************************************
 * @file    sys_err.h
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

#ifndef SYS_ERR_H
#define SYS_ERR_H

/* Includes ----------------------------------------------------------------- */
/* Standard libraries */
/* Project-specific headers */
/* Configuration ------------------------------------------------------------ */
/* Public defines ----------------------------------------------------------- */
/* Public macros ------------------------------------------------------------ */

#define ERR_HARDFAULT(reason) sys_err_hardfault_handler(__FILE__, __LINE__, __func__, reason)

/* Helper macros ------------------------------------------------------------ */
/* Public typedefs ---------------------------------------------------------- */
/* Enumerations ------------------------------------------------------------- */
/* Structures --------------------------------------------------------------- */
/* Error codes -------------------------------------------------------------- */
/* Function prototypes ------------------------------------------------------ */

void sys_err_hardfault_handler(const char *file, int line, const char *func, unsigned int reason);

/* Inline functions --------------------------------------------------------- */

#endif /* SYS_ERR_H */

/* End of File -------------------------------------------------------------- */
