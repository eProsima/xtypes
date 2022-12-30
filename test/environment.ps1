# keep all variables
$old = ls env: 
$oldpath = $old | ? name -eq Path
# load development environment
[xml]$info = & "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\installer\vswhere" `
                -latest -format xml
$pwshmodule = Join-Path $info.instances.instance.installationPath `
              "Common7\Tools\Microsoft.VisualStudio.DevShell.dll" | gi
Import-Module $pwshmodule
Enter-VsDevShell -VsInstanceId $info.instances.instance.instanceId | Out-Null
# keep new variables
$new = ls env: 
$newpath = $new | ? name -eq Path
# output new variables
$cmp = Compare-Object -ReferenceObject $old -DifferenceObject $new
$output = ($cmp.InputObject | % { $_.key + "=" + ($_.value -replace ";","\;") }) -join ";"
$cmppath = Compare-Object -ReferenceObject $oldpath.value.split(";") `
            -DifferenceObject $newpath.value.split(";")
# Split using special char
$output += ">><<"
# output extra Paths
$output += $cmppath.InputObject -join ";"
Write-Output $output
