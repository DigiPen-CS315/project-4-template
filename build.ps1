param (
    [string]$configuration = "Debug", 
    [string]$platform = "x64",
    [switch]$run = $false
)

$path = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | select-object -first 1
if ($path) {
  & $path /p:configuration=$configuration /p:platform=$platform  
}

if ($run -eq $true) {
    ./runall.ps1 $configuration $platform 
}
