# 参考: https://qiita.com/A-Kira/items/b637bb286bdd8002f08b
# 使用方法
# > .\createFiles.ps1 {フォルダパス} {数量} {容量}
Param(
    [Parameter(Mandatory=$true)][string]$path,
    [Parameter(Mandatory=$true)][int]$count,
    [Parameter(Mandatory=$true)][int]$size
)

# ファイル作成または上書き
for ($i = 0; $i -lt $count; $i++) {
    $filePath = $path + "\testFile" + $i
    if (Test-Path $filePath) {
        Remove-Item $filePath;
    }
    fsutil file createnew $filePath $size
}
