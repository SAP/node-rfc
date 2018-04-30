.. _intro:

============
Introduction
============


The NodeJS connector (a synonym for the :mod:`node-rfc` package) wraps the existing *SAP NW RFC Library*,
often colloquially called *SAP C connector* or *SAP NW RFC SDK*. To start using :mod:`node-rfc`
and similar connectors effectively, we highly recommend reading a series of insightful articles
about RFC communication and *SAP NW RFC Library*, published in the SAP Professional Journal (SPJ),
in 2009, by Ulrich Schmidt and Guangwei Li: *Improve communication between your C/C++ applications
and SAP systems with SAP NetWeaver RFC SDK*
`Part 1: RFC client programs <http://www.sdn.sap.com/irj/sdn/index?overridelayout=true&rid=/library/uuid/302f1a30-f4cb-2c10-e985-c8a280a96e43>`_,
`Part 2: RFC server programs <http://www.sdn.sap.com/irj/scn/go/portal/prtroot/docs/library/uuid/b02b0719-4ccc-2c10-71ab-fe31483e466f>`_,
`Part 3: Advanced topics <http://www.sdn.sap.com/irj/sdn/go/portal/prtroot/docs/library/uuid/5070f62a-6acd-2c10-8cb5-858ef06adbb9>`_.

The lecture of these articles and `NW RFC SDK Guide (SAP Help) <http://help.sap.com/saphelp_nw73ehp1/helpdata/en/48/a88c805134307de10000000a42189b/content.htm?frameset=/en/48/a994a77e28674be10000000a421937/frameset.htm>`_
are recommended as an introduction into RFC communication and programming, while :mod:`node-rfc` documentation is
focused merely on technical aspects of :mod:`node-rfc` API.


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
