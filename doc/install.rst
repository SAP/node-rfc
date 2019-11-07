.. _installation:

============
Installation
============

If `SAP NetWeaver RFC SDK <https://support.sap.com/en/product/connectors/nwrfcsdk.html>`_ and `NodeJS <http://nodejs.org/>`_
are already installed on your system, you can install the :mod:`node-rfc` package from npm, or build from source.

You may use the current or any of `LTS node releases  <https://github.com/nodejs/Release>`_ and the latest SAP NW RFC SDK
release (fully backwards compatible).

.. _install-c-connector:

SAP NW RFC SDK Installation
===========================

Information on where to download the SAP NW RFC SDK you may find `here <https://support.sap.com/en/product/connectors/nwrfcsdk.html>`_ .

The NodeJS RFC connector relies on *SAP NW RFC SDK* and must be able to find the library
files at runtime. Therefore, you might either install the *SAP NW RFC SDK*
in the standard library paths of your system or install it in any location and tell the
NodeJS connector where to look.

Here are configuration examples for Windows, Linux and macOS operating systems.

Windows
-------

1. Create the SAP NW RFC SDK home directory, e.g. ``c:\nwrfcsdk``
2. Set SAPNWRFC_HOME environment variable: ``SAPNWRFC_HOME=c:\nwrfcsdk``
3. Unpack the SAP NW RFC SDK archive to it, e.g. ``c:\nwrfcsdk\lib`` shall exist.
4. Include the ``lib`` directory to the library search path on Windows, i.e.
   :ref:`extend<install-problems-envvar-win>` the ``PATH`` environment variable.

Add ``c:\nwrfcsdk\lib`` to PATH.

Linux
-----

1. Create the SAP NW RFC SDK home directory, e.g. ``/usr/local/sap/nwrfcsdk``.
2. Set the SAPNWRFC_HOME environment variable: ``SAPNWRFC_HOME=/usr/local/sap/nwrfcsdk``
3. Unpack the SAP NW RFC SDK archive to it, e.g. ``/usr/local/sap/nwrfcsdk/lib`` shall exist.
4. Include the ``lib`` directory in the library search path:

   * As ``root``, create a file ``/etc/ld.so.conf.d/nwrfcsdk.conf`` and
     enter the following values:

     .. code-block:: sh

        # include nwrfcsdk
        /usr/local/sap/nwrfcsdk/lib

   * As ``root``, run the command ``ldconfig``. To check if libraries are installed:

     .. code-block:: sh

        $ ldconfig -p | grep sap # should show something like:
          libsapucum.so (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libsapucum.so
          libsapnwrfc.so (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libsapnwrfc.so
          libicuuc.so.50 (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libicuuc.so.50
          libicui18n.so.50 (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libicui18n.so.50
          libicudecnumber.so (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libicudecnumber.so
          libicudata.so.50 (libc6,x86-64) => /usr/local/sap/nwrfcsdk/lib/libicudata.so.50
          libgssapi_krb5.so.2 (libc6,x86-64) => /usr/lib/x86_64-linux-gnu/libgssapi_krb5.so.2
          libgssapi.so.3 (libc6,x86-64) => /usr/lib/x86_64-linux-gnu/libgssapi.so.3
        $

macOS
-----

The macOS firewall stealth mode is by default active, blocking the ICMP protocol based network access to Macbook. Applications like
Ping do not work by default (`Can't ping a machine - why? <https://discussions.apple.com/thread/2554739>`_) and the stealth mode
must be disabled:

.. code-block:: sh

    sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setstealthmode off

1. Create the SAP NW RFC SDK home directory ``/usr/local/sap/nwrfcsdk`` (this location is fixed, more info below)
2. Set SAPNWRFC_HOME environment variable: ``SAPNWRFC_HOME=/usr/local/sap/nwrfcsdk``
3. Unpack the SAP NW RFC SDK archive to it, e.g. ``/usr/local/sap/nwrfcsdk/lib`` shall exist.
4. Set the remote paths in SAP NW RFC SDK by running `paths_fix.sh <https://github.com/SAP/PyRFC/blob/master/ci/utils/paths_fix.sh>`_ script.
5. Unzip the `unchar.zip` file attached to `SAP OSS Note 2573953 <https://launchpad.support.sap.com/#/notes/2573953>`_
   and copy the `uchar.h` file to SAP NW RFC SDK include directory:

This default rpath location is ``/usr/local/sap/nwrfcsdk/lib`` is embedded into node-rfc package published on npm.

After moving SAP NW RFC SDK to another location on your system, the rpaths must be adjusted,
in SAP NW RFC SDK and in sapnwrfc.node libraries.

For SAP NW RFC SDK, set the SAPNWRFC_HOME env variable to new SAP NW RFC SDK root directory and re-run the above script.

For node-rfc:

     .. code-block:: sh

        $ npm install node-rfc@next
        $ cd node_modules/node-rfc/lib/binding
        $ install_name_tool -rpath /usr/local/sap/nwrfcsdk/lib <new path> sapnwrfc.node

The v64 suffix is the node abi version for the node release 10 and the suffix for your node release you may find here: https://nodejs.org/en/download/releases.

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

  $ node demo\demo # demo1 ...

In case of issues, check if the SAP NW RFC SDK is properly installed:

.. code-block:: sh

  $ cd $SAPNWRFC_HOME/bin
  $ sudo chmod a+x rfcexec
  $ ./rfcexec # should show something like:
    Error: Not all mandatory parameters specified
      Please start the program in the following way:
      rfcexec -t -a <program ID> -g <gateway host> -x <gateway service>
        -f <file with list of allowed commands> -s <allowed Sys ID>
    The options "-t" (trace), "-f" and "-s" are optional.

The output when SAP NW RFC SDK cannot be found:

.. code-block:: sh

  $ ./rfcexec
  $ ./rfcexec: error while loading shared libraries: libsapnwrfc.so: cannot open shared object file: No such file or directory


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


SAP NW RFC SDK
--------------

1.  ``ImportError: DLL load failed: The specified module could not be found.``

    (Windows)
    This error indicates that the node-rfc connector was not able to find the
    SAP NW RFC SDK libraries on your system. Please check, if the ``$SAPNWRFC_HOME\lib`` directory
    is in your ``PATH`` environment variable.

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

Check `prerequisites <https://github.com/SAP/node-rfc/tree/napi#prerequisites>`_ for your platform and run:

.. code-block:: sh

  npm run prebuild

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

.. hint::

    If you get an error *'sphinx-build' is not recognized as an internal or external command, operable program or batch file*
    on calling ``make html``, install ``sphinx``.

The docu is hosted on GitHub Pages, a propietary solution where a git branch ``gh-pages`` is created
as an orphan and the output of the documentation build process (``_build/html``) is stored in that branch.

GitHub then serves these files under a special ``/pages/`` url.

To update GitHub Pages, copy everyhing under ``_build/html`` and overwrite the existing files in the ``gh-pages`` branch root:

.. code-block:: sh

  rm -Rf ~/tmp/html

  cp -r doc/_build/html ~/tmp/.

  git checkout gh-pages

  rm -Rf *.html *.js *.inv _* doc .buildinfo

  cp -R ~/tmp/html/. .

  touch .nojekyll

