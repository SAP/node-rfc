SAP NW RFC Library supports plain and secure connection with following authentication methods:

-   Plain with user / password
-   SNC with user PSE
-   SNC with client system PSE and user X509

NW ABAP servers support in addition:

-   SAP logon tickets
-   Security Assertion Markup Language (SAML)

Assuming you are familiar with abovementioned concepts and have ABAP backend system configured for SNC communication, here you may find connection strings examples, for testing plain and secure RFC connections, with various authentication methods.

## Plain with user credentials

The simplest and the least secure form of the user authentication:

```javascript
const abapConnection = {
    user: "demo",
    passwd: "welcome",
    ashost: "10.68.110.51",
    sysnr: "00",
    client: "620",
    lang: "EN",
};
```

## SNC with user PSE

User PSE is used for opening the SNC connection and the same PSE is used for the authentication (logon) in NW ABAP backend.

Generally not recomended, see SAP Note [1028503 - SNC-secured RFC connection: Logon ticket is ignored](https://launchpad.support.sap.com/#/notes/1028503)

```javascript
const abapConnection = {
    snc_lib: "C:\\Program Files\\SAP\\FrontEnd\\SecureLogin\\libsapcrypto.dll",
    snc_partnername: "p/secude:CN=QM7, O=SAP-AG, C=DE",
    ashost: "ldciqm7.wdf.sap.corp",
    sysnr: "20",
    client: "715",
};
```

In this example the `SNC_LIB` key contains the path to security library (SAP cryptographic library or 3rd party product).

Alternatively, the path can be set as `SNC_LIB` environment variable, in which case it does not have to be provided as a connection parameter.

**Prerequisites**

-   SAP Single Sign On must be configured on a client and the user must be logged in on a client.
-   SNC name must be configured for the ABAP user in NW ABAP system, using transaction SU01:
    ![](assets/SU01-SNC.png)

## SNC with client system PSE and User X509

The client system PSE is used for opening SNC connection and user X509 certificate is forwarded to ABAP backend system, for authentication and logon.

Connection parameters are the same as in a previous example, with user X509 certificate added:

```javascript
const abapConnection = {
    snc_lib: "C:\\Program Files\\SAP\\FrontEnd\\SecureLogin\\libsapcrypto.dll",
    snc_partnername: "p/secude:CN=QM7, O=SAP-AG, C=DE",
    x509cert: "MIIDJjCCAtCgAwIBAgIBNzA ... NgalgcTJf3iUjZ1e5Iv5PLKO",
    ashost: "ldciqm7.wdf.sap.corp",
    sysnr: "20",
    client: "715",
};
```

**Prerequisites**

-   The user does not have to be logged into the client system, neither the Single Sign On must be configured on a client
-   The trusted relationship must be established between the ABAP backend system and the client system.
-   The client system must be registered in the NW ABAP backend Access Control List (ACL), using transaction SNC0
-   Keystores are generated on a client system, using SAP cryptography tool SAPGENPSE and the environment variable SECUDIR points to the folder with generated keystores
    ![](assets/SNC0-1.png)
-   User X509 certificate must be mapped to ABAP NW backend user, using transaction EXTID_DN
    ![](assets/EXTID_DN-1.png)

    ![](assets/EXTID_DN-2.png)
