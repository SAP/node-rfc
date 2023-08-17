*&---------------------------------------------------------------------*
*& Report ZSERVER_STFC_STRUCT
*&---------------------------------------------------------------------*
*&
*&---------------------------------------------------------------------*
report zserver_stfc_struct.

data lv_echo like sy-lisel.
data lv_resp like sy-lisel.

data ls_struct like  rfctest.
data lt_table like table of rfctest.

data lv_error_message type char512.

ls_struct-rfcint1 = 1.
ls_struct-rfcint2 = 2.
ls_struct-rfcint4 = 4.

insert ls_struct into table lt_table.
call function 'STFC_STRUCTURE' destination 'NWRFC_SERVER_OS'
  exporting
    importstruct          = ls_struct
  importing
    echostruct            = ls_struct
    resptext              = lv_resp
  tables
    rfctable              = lt_table
  exceptions
    communication_failure = 1 message lv_error_message
    system_failure        = 2 message lv_error_message.

if sy-subrc eq 0.
  write: / 'rfcint1:', ls_struct-rfcint1.
  write: / 'rfcint2:', ls_struct-rfcint2.
  write: / 'rfcint4:', ls_struct-rfcint4.
  write: / 'resptext:', lv_resp.
else.
  write:   'subrc  :', sy-subrc.
  write: / 'msgid  :', sy-msgid, sy-msgty, sy-msgno.
  write: / 'msgv1-4:', sy-msgv1, sy-msgv2, sy-msgv3, sy-msgv4.
  write: / 'message:', lv_error_message.
  exit.
endif.