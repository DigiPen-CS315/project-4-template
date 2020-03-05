param (
    [string]$configuration = "Debug", 
    [string]$platform = "x64"
)

if ($platform -eq "x64") {
    $platform = "x64/"
}
else {
    $platform = ""
}

$exePath = "$platform$configuration"
Write-Host $exePath

for ($i=0; $i -le 11; $i++) {
    Write-Host $exePath/project2.exe $i
    & ./$exePath/project2.exe $i
}
