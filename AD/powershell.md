Powershell b64 encode for the rev shells
```
PS C:\Users\IñakiTornos> $str = 'whoami.exe'
PS C:\Users\IñakiTornos> [System.Convert]::ToBase64String([System.Text.Encoding]::Unicode.GetBytes($str)) | clip
PS C:\Users\IñakiTornos> powershell.exe -enc dwBoAG8AYQBtAGkALgBlAHgAZQA=
azuread\iñakitornos
PS C:\Users\IñakiTornos>
```