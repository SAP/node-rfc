# Installation

- **[SAP NWRFC SDK installation](#sap-nwrfc-sdk-installation)**
  - [Windows](#windows)
  - [Linux](#linux)
  - [macOS](#macos)
- **[node-rfc installation](#node-rfc-installation)**
- **[Troubleshooting](#troubleshooting)**
  - [Verify SAP NWRFC SDK release](#verify-sap-nwrfc-sdk-release)
  - [Verify node-rfc installation](#verify-node-rfc-installation)
  - [Verify node-rfc environment](#verify-node-rfc-environment)
- **[Publish release](#publish-release) (maintainers)**

## SAP NWRFC SDK Installation

First check if your host system platform is supported and install prerequisites, if needed: [SAP/node-rfc#requirements](https://github.com/SAP/node-rfc#requirements)

Information on where to download the SAP NWRFC SDK you may find at: https://support.sap.com/en/product/connectors/nwrfcsdk.html

The Node.js RFC connector relies on SAP NWRFC SDK and must be able to find the library
files at runtime. Therefore, you might either install the SAP NWRFC SDK
in the standard library paths of your system or install it in any location and tell the
Node.js connector where to look.

Here are configuration examples for Windows, Linux and macOS operating systems.

### Windows

1. Create the SAP NWRFC SDK home directory, e.g. `c:\nwrfcsdk`
2. Set SAPNWRFC_HOME environment variable: `SAPNWRFC_HOME=c:\nwrfcsdk`
3. Unpack the SAP NW RFC SDK archive to it, e.g. `c:\nwrfcsdk\lib` shall exist.
4. Include the `lib` directory to the library search path on Windows, i.e. add to `PATH` environment variable:

```shell
PS C:\> $env:Path += ";$env:SAPNWRFC_HOME\lib"
```

### Linux

1. Create the SAP NW RFC SDK home directory, e.g. `/usr/local/sap/nwrfcsdk`.
2. Set the SAPNWRFC_HOME environment variable: `SAPNWRFC_HOME=/usr/local/sap/nwrfcsdk`
3. Unpack the SAP NW RFC SDK archive to it, e.g. `/usr/local/sap/nwrfcsdk/lib` shall exist.
4. Include the `lib` directory in the library search path:

   - As `root`, create a file `/etc/ld.so.conf.d/nwrfcsdk.conf`

        **nwrfcsdk.conf**

        ```conf
        # include nwrfcsdk
        /usr/local/sap/nwrfcsdk/lib
        ```

   - As `root`, run the command `ldconfig` and check if libraries are installed:

        ```shell
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
        ```

### macOS

:exclamation: Stay on SAP NWRFC SDK <= 7.55 until [#143](https://github.com/SAP/node-rfc/issues/143) closed.

The macOS firewall stealth mode is by default active, blocking the ICMP protocol based network access to Macbook. Applications like Ping do not work by default (Can’t ping a machine - why?) and the stealth mode must be disabled:

```shell
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setstealthmode off
```

1. Create the SAP NWRFC SDK home directory /usr/local/sap/nwrfcsdk (this location is fixed, more info below)
2. Set SAPNWRFC_HOME environment variable: SAPNWRFC_HOME=/usr/local/sap/nwrfcsdk
3. Unpack the SAP NWRFC SDK archive to it, e.g. /usr/local/sap/nwrfcsdk/lib shall exist.

Set the remote paths in SAP NW RFC SDK by running [paths_fix.sh](https://github.com/SAP/PyRFC/blob/master/ci/utils/paths_fix.sh) script.

**Custom SAP NWRFC SDK location**

The default rpath `/usr/local/sap/nwrfcsdk/lib` is embedded into `node-rfc` package published on npm.

After moving SAP NWRFC SDK to different location on your system, the rpaths must be adjusted, in SAP NWRFC SDK and in `sapnwrfc.node` binary lib:

1. Set the SAPNWRFC_HOME env variable to new SAP NWRFC SDK home directory and re-run the above script
2. Fix `sapnwrfc.node`

    ```shell
    install_name_tool -rpath /usr/local/sap/nwrfcsdk/lib /usr/local/sap/nwrfcsdk_new/lib sapnwrfc.node
    ```

## node-rfc installation

The `node-rfc` package can be installed from npm, or built from source: [Installation](/README.md#installation).

## Troubleshooting

`ImportError: DLL load failed: The specified module could not be found`

(Windows) This error indicates that the node-rfc connector was not able to find the SAP NWRFC SDK libraries on your system.

Verify:

1. `SAPNWRFC_HOME\lib` directory is in your PATH environment variable
2. SAP NWRFC SDK for your platform is installed, not Linux SDK for example on Windows system
3. SAP NWRFC SDK 32bit is installed when runnung 32bit Node.js, not 64bit SDK

`ImportError: DLL load failed: %1 is not a valid Win32 application`

(Windows) This error occurs when SAP NWRFC Library 64bit version is installed on a system with 32bit version Node.js.

## Verify SAP NWRFC SDK release

**Windows**

```shell
@echo off
findstr Patch %SAPNWRFC_HOME%\lib\sapnwrfc.dll
```

**Linux**

```shell
echo `strings $SAPNWRFC_HOME/lib/libsapnwrfc.so | grep "Patch Level"`
```

**macOS**

```shell
echo `strings $SAPNWRFC_HOME/lib/libsapnwrfc.dylib | grep "Patch Level"`
```

## Verify SAP NW RFC SDK installation

To verify SAP NW RFC SDK installation run the `startrfc -v` from SAP NW RFC SDK `bin` folder. The output should be like:

```shell
cd $SAPNWRFC_HOME/bin
./startrfc -v
RfcLogWrite: could not open /usr/local/sap/nwrfcsdk/./dev_rfc_17149.log. Trace directory set to ./NW RFC Library Version: 750 Patch Level 12
Compiler Version:
Version not available.
Startrfc Version: 2018-08-15
```

The output like below, shows that either 32 bit NWRFC SDK is installed on 64 bit system, or NWRFC SDK for another platform is installed:

```shell
./rfcexec
./rfcexec: error while loading shared libraries: libsapnwrfc.so:
cannot open shared object file: No such file or directory
```

## Verify node-rfc installation

```shell
node -p "require('node-rfc')" # should end w/o errors
```

## Verify node-rfc environment

You may use the `npx envinfo`:

```shell
npx envinfo --system --binaries
npx: installed 1 in 1.399s

  System:
    OS: macOS 10.15.7
    CPU: (8) x64 Intel(R) Core(TM) i7-7820HQ CPU @ 2.90GHz
    Memory: 1.27 GB / 16.00 GB
    Shell: 5.8 - /usr/local/bin/zsh
  Binaries:
    Node: 14.13.1 - ~/.Node.js/nvm/versions/node/v14.13.1/bin/node
    npm: 6.14.8 - ~/.Node.js/nvm/versions/node/v14.13.1/bin/npm
```

and/or noderfc `environment` object:

```shell
node -p "require('node-rfc')".environment
{
  platform: { name: 'darwin', arch: 'x64', release: '19.5.0' },
  env: { SAPNWRFC_HOME: '/usr/local/sap/nwrfcsdk', RFC_INI: '' },
  noderfc: {
    release: '1.2.0',
    nwrfcsdk: { major: 7500, minor: 0, patchLevel: 5 }
  },
  versions: {
    node: '14.4.0',
    v8: '8.1.307.31-node.33',
    uv: '1.37.0',
    zlib: '1.2.11',
    brotli: '1.0.7',
    ares: '1.16.0',
    modules: '83',
    nghttp2: '1.41.0',
    napi: '6',
    llhttp: '2.0.4',
    openssl: '1.1.1g',
    cldr: '37.0',
    icu: '67.1',
    tz: '2019c',
    unicode: '13.0'
  }
}
```

## Publish release

```shell
# push to github
git tag -a v1.0.3 e585cb2 -m "v1.0.3"
git push origin --tags
# for each platform
git pull
npm run addon
prebuild -r napi -u $PAT --verbose
# push to npm
npm publish
```
