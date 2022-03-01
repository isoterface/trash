# 参考: https://tsuredurediary.com/archives/windows-dummy-file.html
Param (
    [Parameter(Mandatory=$true)]
    [string]
    $FolderPath,
    [Parameter(Mandatory=$true)]
    [ValidateRange(1,100)]
    [int]
    $FileCount,
    [Parameter(Mandatory=$true)]
    [ValidateRange(1,107374182400)]
    [long]
    $FileSize
)

#----- define functions
function CreateRandomDataDummyFile([string]$fpath, [long]$fsize) {

    if((Split-Path $fpath -IsAbsolute) -eq $false){
        $LocationPath = Get-Location
        $fpath = Join-Path $LocationPath $fpath
    }
    
    Add-type -AssemblyName System.Web
    
    [long]$WriteSize = 0

    $file = New-Object System.IO.StreamWriter($fpath, $false)
    while($WriteSize -lt $fsize){
        $String = [System.Web.Security.Membership]::GeneratePassword(128, 1)
        $file.Write($String)
        $WriteSize = $WriteSize + 128
    }

    $file.Close()
}

#----- main
$ErrorActionPreference = 'Stop'

for ($i = 0; $i -lt $FileCount; $i++) {
    $filePath = $FolderPath + "\testFile" + $i
    if (Test-Path $filePath) {
        Remove-Item $filePath;
    }
    $msg = "CreateFile: " + $filePath
    Write-Host $msg
    CreateRandomDataDummyFile $filePath $FileSize
}
