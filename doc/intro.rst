.. _intro:

============
Introduction
============


The NodeJS connector (a synonym for the :mod:`node-rfc` package) wraps the existing *SAP NW RFC SDK Library*,
often colloquially called *SAP C connector*. To start using :mod:`node-rfc` and similar connectors effectively,
we highly recommend the `SAP NW RFC SDK documentation and programming guide <https://support.sap.com/en/product/connectors/nwrfcsdk.html>`_
and the series of three insightful articles about RFC communication and SAP NW RFC Library,
published in the SAP Professional Journal (SPJ), by Ulrich Schmidt and Guangwei Li:

`Part 1: RFC client programs <https://wiki.scn.sap.com/wiki/x/zz27Gg>`_,
`Part 2: RFC server programs <https://wiki.scn.sap.com/wiki/x/9z27Gg>`_,
`Part 3: Advanced topics <https://wiki.scn.sap.com/wiki/x/FD67Gg>`_.

The :mod:`node-rfc` documentation here is focused merely on technical aspects of :mod:`node-rfc` API.


Example usage
=============

In order to call remote enabled ABAP function module, we need to create a client with
valid logon credentials, connect to the nw system and then we can invoke a remote enabled
ABAP function module from node.

.. code-block:: js

  var rfc = require('node-rfc');

  // create new client
  var client = new rfc.Client({'user': 'demo', 'passwd': 'welcome', 'ashost': '10.0.0.1', 'sysnr': '00', 'client': '001'});

  client.connect(function(err) {  // and connect
    if (err) {  // check for login/connection errors
      return console.error('could not connect to server', err);
    }

    // invoke remote enabled ABAP function module
    client.invoke('STFC_CONNECTION', { REQUTEXT: 'Hello SAP!' }, function(err, res) {
        if (err) {  // check for errors (e.g. wrong parameters)
          return console.error('Error invoking STFC_CONNECTION:', err);
        }
        // work with result;  should be something like:
        // { ECHOTEXT: 'Hello SAP!',
        //   RESPTEXT: 'SAP R/3 Rel. 702   Sysid: E1Q      Date: 20140613   Time: 142530   Logon_Data: 001/DEMO/E',
        //   REQUTEXT: 'Hello SAP!' }
        console.log('Result STFC_CONNECTION:', res);
    }):
  });

Finally, the connection is closed automatically when the instance is deleted by the garbage collector
or by explicitly calling the :meth:`Client.close()` method on the client instance.


Data types
==========
A remote function call executes ABAP code, which works with parameters
that have an ABAP data type. Hence, when you look at the metadata description
you will find ABAP data types for the parameters.

The Javascipt connector does not provide ABAP data types to be instantiated and
used within Javascript code, but rather converts between ABAP data types and Javascript
built-in types.

.. Resources:
  http://help.sap.com/saphelp_nw04/helpdata/en/fc/eb2fd9358411d1829f0000e829fbfe/content.htm
  http://msdn.microsoft.com/en-us/library/cc185537%28v=bts.10%29.aspx

================= ========== ========================================== =========== ============== =====================================================================
Type Category     ABAP       Meaning                                    RFC         Javascript     Remark
================= ========== ========================================== =========== ============== =====================================================================
numeric           I          Integer (whole number)                     INT         Integer        Internal 1 and 2 byte integers (INT1, INT2) are also mapped to int
numeric           F          Floating point number                      FLOAT       Number
numeric           P          Packed number / BCD number                 BCD         see remark     Input as Number, string or decimal (decimal.js ...), output as string
character         C          Text field (alphanumeric characters)       CHAR        string
character         D          Date field (Format: YYYYMMDD)              DATE        string
character         T          Time field (Format: HHMMSS)                TIME        string
character         N          Numeric text field (numeric characters)    NUM         string
hexadecimal       X          Hexadecimal field                          BYTE        string
variable length   STRING     Dynamic length string                      STRING      string
variable length   XSTRING    Dynamic length hexadecimal string          BYTE        string
================= ========== ========================================== =========== ============== =====================================================================

Further `details on predefined ABAP types`_ are available online.

.. _details on predefined ABAP types: http://help.sap.com/saphelp_nw04/helpdata/en/fc/eb2fd9358411d1829f0000e829fbfe/content.htm

The Javascript representation of a parameter is a simple key-value pair, where
the key is the name of the parameter and the value is the value of the parameter
in the corresponding Javascript type.
Beside the mentioned types, there are tables and structures:

* A structure is represented in Javascript by an object, with the
  structure fields' names as dictionary keys.
* A table is represented in Javascript by a list of objects.
