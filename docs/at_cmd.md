/* Document ----------------------------------------------------------- */
/*
=====================================================================
    PARAMETER TYPES
=====================================================================
ASCII printable code: [0x20:0x7E] (space) to (~)

*** Quoted-text: ----------------------------------------------------   A_QUOTE
  Character set:  Any printable character, enclosed between quotes.
                  [0x20:0x7E] (space) to (~)
                  Double quote to express a quote inside quoted-text: ""
  Example:        "smDev@!, abcdef" --> (smDev@!, abcdef)
                  "Hello ""Triet""" --> (Hello "Triet")
                  SM+CMD="smDev@!, abcdef",1
                  SM+CMD="Hello ""Triet""",1

*** Plain-text: -----------------------------------------------------   A_PLAIN
  Character set:  Number, letters:  [0-9], [a-z], [A-Z]
                  Dot:              (.)
                  Dashes:           (-)
                  Underscores:      (_)
    *** HEX-code: ---------------------------------------------------   A_HEX
          Character set:  8-bit value is expressed by 2 ASCII bytes.
                          [0-9], [A-F]
          Example:        2 bytes:  0x7E2A    --> 7E2A
                          3 bytes:  0x0C2D3E  --> 0C2D3E
                          SM+CMD=7E2A,1
                          SM+CMD=0C2D3E,1
    *** Integer number: ---------------------------------------------   A_INTEGER
          Character set:  Number: [0-9]
                          Minus:  (-)
          Example:        SM+CMD=15,-2000
    *** Real number: ------------------------------------------------   A_REAL
          Character set:  Number: [0-9]
                          Minus:  (-)
                          Dot:    (.)
          Example:        SM+CMD=15,-2.123465,1.5

=====================================================================
    SM COMMAND SYNTAX
=====================================================================
*** Common ----------------------------------------------------------
    + <CMD>:        Plain-text. 1 to 32 characters
    + [=]:          Could be omitted if there is no parameter/item

*** Request: --------------------------------------------------------
  SM+<CMD>[=<PARAM_1>[,<PARM_2>[,<PARAM_3>[,...]]]][?]
    + ?: READ syntax
         WRITE/EXECUTE syntax
    + Examples:
      SM+COMMAND
      SM+COMMAND?
      SM+COMMAND=PARAM1,PARAM2,...
      SM+COMMAND=PARAM1?
      SM+COMMAND=PARAM1,PARAM2?

*** Response OK: ----------------------------------------------------
  OK+<CMD>[=<RES_1>[,<RES_2>[,<RES_3>[,...]]]]
    + Examples:
      OK+COMMAND
      OK+COMMAND=RES_1,RES_2,...

*** Response Error: -------------------------------------------------
  * Syntax Error/Unrecognized command:
    ER+SYNTAX

  * Normal Error:
    ER+<CMD>[=<ER_CODE>[,<ER_MESSAGE>]]
      + <ER_CODE>:    Integer
      + <ER_MESSAGE>: Quoted-text

      + Examples:
        ER+COMMAND
        ER+COMMAND=1
        ER+COMMAND=1,"INVALID PARAMETER"

*** Notification: ---------------------------------------------------
  NT+<CMD>[=<NT_1>[,<NT_2>[,<NT_3>[,...]]]]
    + Example:
      NT+UIB=NO_NETWORK

*** Examples: -------------------------------------------------------
*/