$output = "all_code.txt"
$exclude = @("\.pio", "\.git", "data", "document", "test")
$include = @("*.cpp", "*.h", "*.ini", "*.json", "*.prettierrc", "*.gitignore")
$enc = [System.Text.Encoding]::UTF8

$writer = [System.IO.StreamWriter]::new($output, $false, $enc)
Get-ChildItem -Recurse -File -Include $include | Where-Object {
    $path = $_.FullName
    -not ($exclude | Where-Object { $path -match $_ })
} | ForEach-Object {
    $writer.WriteLine("=" * 60)
    $writer.WriteLine("=== $($_.FullName) ===")
    $writer.WriteLine("=" * 60)
    [System.IO.File]::ReadAllText($_.FullName, $enc) -split "`n" | ForEach-Object { $writer.WriteLine($_) }
    $writer.WriteLine("")
}
$writer.Close()
Write-Host "完了！UTF-8で出力しました。"