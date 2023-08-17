*&---------------------------------------------------------------------*
*& Report ZSERVERTEST_C1
*&---------------------------------------------------------------------*
*&
*&---------------------------------------------------------------------*
report zserver_stfc_connection.

data lv_echo like sy-lisel.
data lv_resp like sy-lisel.

data ls_struct like  rfctest.
data lt_table like table of rfctest.

data lv_error_message type char512.

call function 'STFC_CONNECTION' destination 'NWRFC_SERVER_OS'
  exporting
    requtext              = '4'
  importing
    echotext              = lv_echo
    resptext              = lv_resp
  exceptions
    communication_failure = 1 message lv_error_message
    system_failure        = 2 message lv_error_message.

if sy-subrc eq 0.
  write: 'result'.
  write: / lv_echo.
  write: / lv_resp.
else.
  write:   'subrc  :', sy-subrc.
  write: / 'msgid  :', sy-msgid, sy-msgty, sy-msgno.
  write: / 'msgv1-4:', sy-msgv1, sy-msgv2, sy-msgv3, sy-msgv4.
  write: / 'message:', lv_error_message.
  exit.
endif.