.. _installation:

============
Installation
============

:mod:`node-rfc` is a wrapper for the *SAP NetWeaver RFC Library* and you need to obtain and install it first.

If `NodeJS <http://nodejs.org/>`_ is not already installed on your system, you need to download and install it as well.


.. _install-c-connector:

SAP NW RFC Library Installation
===============================

The entry page for *SAP NetWeaver RFC library* is SAP Service Marketplace (SMP), http://service.sap.com/rfc-library,
with detailed instructions how to
`download <http://service.sap.com/sap/support/notes/1025361>`_,
`use <https://websmp103.sap-ag.de/~sapidb/011000358700000869672007.pdf>`_ and
`compile <https://websmp103.sap-ag.de/sap/support/notes/1056696>`_.

Basically, you should search for ``SAP NW RFC SDK 7.20``, in ``Software Downloads`` of SAP Software Download Center
on `SMP Support Portal <http://service.sap.com/support>`_, download SAP NW RFC Library adequate for your platform
and unpack using ``SAPCAR`` archive utility.

.. figure:: _static/SMP-SAPNWRFCSDK.png
  :align: center

``SAPCAR`` can be donwloaded from the SMP as well and you should search for ``SAPCAR 7.20``

.. figure:: _static/SMP-SAPCAR.png
  :align: center

.. _install-combination:

Which SAP NW RFC Library version is relevant for your platform? Here are platform/Python combinations tested so far:

========== ============================== =========================
Platform     NetWeaver RFC Library (SMP)       Filename (SMP)
========== ============================== =========================
Windows    *Windows on x64 64bit*         ``NWRFC_20-20004568.SAR``
Linux      *Linux on x86_64 64bit*        ``NWRFC_20-20004565.SAR``
========== ============================== =========================

.. note::
   * *SAP NW RFC Library* is fully backwards compatible and it is reccomended using
     the newest version also for older backend system releases

   * SMP search terms and filenames given here will not be regularly updated,
     you should always search  for current version or filename in ``Software Downloads``.

   * The server functionality is currently not working under Windows 32bit

.. _SAP Note 1025361: http://service.sap.com/sap/support/notes/1025361
.. _download location: http://www.service.sap.com/~form/handler?_APP=00200682500000001943&_EVENT=DISPHIER&HEADER=N&FUNCTIONBAR=N&EVENT=TREE&TMPL=01200314690200010197&V=MAINT


The NodeJS connector relies on *SAP NW RFC Library* and must be able to find library
files at runtime. Therefore, you might either install the *SAP NW RFC Library*
in the standard library paths of your system or install it in any location and tell the
NodeJS connector where to look.

Here are configuration examples for Windows and Linux operating systems.


Windows
-------

1. Create an directory, e.g. ``c:\nwrfcsdk``.
2. Unpack the SAR archive to it, e.g. ``c:\nwrfcsdk\lib`` shall exist.
3. Include the ``lib`` directory to the library search path on Windows, i.e.
   :ref:`extend<install-problems-envvar-win>` the ``PATH`` environment variable.

Add ``c:\nwrfcsdk\lib`` to PATH

Linux
-----

1. Create the directory, e.g. ``/usr/local/sap/nwrfcsdk``.
2. Unpack the SAR archive to it, e.g. ``/usr/local/sap/nwrfcsdk/lib`` shall exist.
3. Include the ``lib`` directory in the library search path:

   * As ``root``, create a file ``/etc/ld.so.conf.d/nwrfcsdk.conf`` and
     enter the following values:

     .. code-block:: sh

        # include nwrfcsdk
        /usr/local/sap/nwrfcsdk/lib

   * As ``root``, run the command ``ldconfig``.


.. _install-node-connector:

node-rfc Module Installation
============================

If not already installed, you need to install ``node`` and ``npm`` first.

Set NODE_PATH to wherever your node modules are installed, like

``set NODE_PATH=C:\Users\BSrdjan\AppData\Roaming\npm\node_modules``

Install the node-rfc module from npm

.. code-block:: sh

    npm install node-rfc


Test
----

Check if the module can be found and loaded:

.. code-block:: js

  var r = require ('node-rfc)

Call remote enabled function modules in NW backend system (maintain your test system parameters first):

.. code-block:: sh

  node demo\demo
  node demo\demo1, 2 ...


Problems
========

Behind a Proxy
--------------

If you are within an internal network that accesses the internet through
an HTTP(S) proxy, some of the shell commands will fail with urlopen errors, etc.

Assuming that your HTTP(S) proxy could be accessed via ``http://proxy:8080``, on Windows
you can communicate this proxy to your shell via::

    SET HTTP_PROXY=http://proxy:8080
    SET HTTPS_PROXY=http://proxy:8080

or permanently set environment variables.


SAP NW RFC Library Installation
-------------------------------

1.  ``ImportError: DLL load failed: The specified module could not be found.``

    (Windows)
    This error indicates that the Python connector was not able to find the
    C connector on your system. Please check, if the ``lib`` directory of the
    C connector is in your ``PATH`` environment variable.

2. ``ImportError: DLL load failed: %1 is not a valid Win32 application.``

   (Windows)
   This error occurs when SAP NW RFC Library 64bit version is installed on a system with 32bit version Python.

Environment variables
---------------------

.. _install-problems-envvar-win:

Windows
'''''''
The environment variable may be set within a command prompt via the ``set``
command, e.g.

* ``set PATH=%PATH%;C:\nwrfcsdk\lib`` (extend PATH with the C connector lib)
* ``set HTTPS_PROXY=proxy:8080`` (setting an proxy for HTTPS communication)

When the command prompt is closed, the environment variable is reset. To achieve
a persistent change of the environment variable, do the following (Windows 7):

1. Open the Start Menu and type ``environment`` into the search box.
2. A window opens in which the user variables are displayed in the upper part
   and the system variables in the lower part. You may select and edit
   the desired variable.
3. The modified variables are used when a *new* command prompt is opened.


.. _build:

Building from Source
====================

Windows
-------

Toolchain used for building on Windows is VS2013 Professional Update 2, with native tools command prompts for 32 and 64 bit architectures:

.. code-block:: sh

  node-gyp clean
  node-gyp configure --msvs_version=2013
  node-gyp build

Linux
-----

To compile on Linux, run

.. code-block:: sh

  node-gyp configure build

For unit tests run

.. code-block:: sh

  npm test

For more test examples, see files in demo folder.

.. _makethedoc:

Building the Documentation
--------------------------

Change into the ``doc`` directory and type:

  .. code-block:: sh

     make clean
     make html

The result is found in ``_build/html`` and for other options call ``make``.

* If you get an error *'sphinx-build' is not recognized as an internal or external command, operable program or batch file* on calling ``make html``, install ``sphinx``

The docu is hosted on GitHub Pages, a propietary solution where a git branch ``gh-pages`` is created 
as an orphan and the output of the documentation build process (``_build/html``) is stored in that branch. 

GitHub then serves these files under a special ``/pages/`` url.

To update GitHub Pages, copy everyhing under ``_build/html`` and overwrite the existing files in the ``gh-pages`` branch root:

.. code-block:: sh

  rm -Rf ~/tmp/html

  cp doc/_build/html ~/tmp/.

  git checkout gh-pages

  rm -Rf *.html *.js *.inv _* doc .buildinfo

  cp -R ~/tmp/html/. .

  touch .nojekyll

