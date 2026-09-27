param([Parameter(ValueFromRemainingArguments=$true)][string[]]$BuildArgs)
$info = [System.Diagnostics.ProcessStartInfo]::new()
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $installation) { throw 'Visual Studio with C++ build tools was not found.' }
$info.FileName=Join-Path $installation 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$info.UseShellExecute=$false
$clean=@{}
foreach($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) { $clean[$entry.Key.ToUpperInvariant()]=$entry.Value }
if (-not $clean.ContainsKey('CMAKE_GENERATOR')) { $clean.Remove('CMAKE_GENERATOR_PLATFORM') }
$info.Environment.Clear()
foreach($key in $clean.Keys) { $info.Environment[$key]=$clean[$key] }
foreach($arg in $BuildArgs) { $info.ArgumentList.Add($arg) }
$proc=[System.Diagnostics.Process]::Start($info)
$proc.WaitForExit()
exit $proc.ExitCode
