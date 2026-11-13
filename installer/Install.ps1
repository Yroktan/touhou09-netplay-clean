param([string]$GameDirectory,[string]$RelayHost,[int]$RelayPort=0)
$ErrorActionPreference='Stop'
$releaseRoot=Split-Path -Parent $PSScriptRoot
if(-not $GameDirectory) {
 Add-Type -AssemblyName System.Windows.Forms
 $picker=New-Object System.Windows.Forms.FolderBrowserDialog
 $picker.Description='Select your Touhou 09 version 1.50a game folder'
 try {
  if($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK){exit 0}
  $GameDirectory=$picker.SelectedPath
 } finally {$picker.Dispose()}
}
$gamePath=(Resolve-Path -LiteralPath $GameDirectory).Path
$required=@{
 'th09.dat'='FC810CBD80BD05BA9B24AEDFEEFAD03250128E83'
 'thbgm.dat'='B0D04325D7AB1D9C0CD4ED94B008058C08926277'
}
foreach($name in $required.Keys) {
 $file=Join-Path $gamePath $name
 if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw "Missing game file: $name"}
 if((Get-FileHash -LiteralPath $file -Algorithm SHA1).Hash -ne $required[$name]) {
  throw "Unsupported game data in $name. Use compatible Touhou 09 1.50a files; Steam is not required."
 }
}
$binary=Join-Path $releaseRoot 'mod\th09-netplay.exe'
if(-not (Test-Path -LiteralPath $binary)){$binary=Join-Path $releaseRoot 'game\th09-netplay.exe'}
if(-not (Test-Path -LiteralPath $binary)){throw 'Build the game first or use the release archive.'}
$values=@{}
$envPath=Join-Path $gamePath '.env'
$sourceEnv=Join-Path $releaseRoot '.env'
if(Test-Path -LiteralPath $envPath){$sourceEnv=$envPath}
if(Test-Path -LiteralPath $sourceEnv) {
 foreach($line in (Get-Content -LiteralPath $sourceEnv -Encoding UTF8)) {
  if($line -match '^\s*(TH09_RELAY_HOST|TH09_RELAY_PORT)\s*=\s*(.*?)\s*$') {
   $key=$matches[1];$value=$matches[2]
   if($value.Length -ge 2 -and (($value.StartsWith('"') -and $value.EndsWith('"')) -or ($value.StartsWith("'") -and $value.EndsWith("'")))) {
    $value=$value.Substring(1,$value.Length-2)
   }
   $values[$key]=$value
  }
 }
}
if(-not $RelayHost){$RelayHost=$values['TH09_RELAY_HOST']}
if(-not $RelayHost){$RelayHost=Read-Host 'Relay hostname or IPv4 (provided by the server owner)'}
$RelayHost=$RelayHost.Trim()
if([Uri]::CheckHostName($RelayHost) -notin @([UriHostNameType]::Dns,[UriHostNameType]::IPv4) -or $RelayHost -match '[^\x21-\x7e]') {
 throw 'Use an ASCII hostname or an IPv4 address, without a protocol or port.'
}
if($RelayPort -eq 0) {
 if($values['TH09_RELAY_PORT']) {
  if($values['TH09_RELAY_PORT'] -notmatch '^\d+$'){throw 'Invalid TH09_RELAY_PORT'}
  $RelayPort=[int]$values['TH09_RELAY_PORT']
 } else {$RelayPort=10900}
}
if($RelayPort -lt 1 -or $RelayPort -gt 65535){throw 'Relay port must be 1-65535.'}
$destination=Join-Path $gamePath 'th09-netplay.exe'
if(Test-Path -LiteralPath $destination) {
 if((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $binary -Algorithm SHA256).Hash) {
  Copy-Item -LiteralPath $destination -Destination ($destination+'.backup-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
 }
}
if([IO.Path]::GetFullPath($binary) -ne [IO.Path]::GetFullPath($destination)) {
 Copy-Item -LiteralPath $binary -Destination $destination -Force
}
$configuration="TH09_RELAY_HOST=$RelayHost`r`nTH09_RELAY_PORT=$RelayPort`r`n"
if(Test-Path -LiteralPath $envPath) {
 $existing=[IO.File]::ReadAllText($envPath)
 if($existing -ne $configuration) {
  Copy-Item -LiteralPath $envPath -Destination ($envPath+'.backup-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
 }
}
[IO.File]::WriteAllText($envPath,$configuration,(New-Object Text.UTF8Encoding($false)))
Write-Host "Installed: $destination"
Write-Host 'Connection settings are in .env beside the EXE. Keep this file private.'
Write-Host 'Run th09-netplay.exe and select NETPLAY.'
