# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @file environment.ps1
#

Param(
[Parameter(Mandatory=$true, HelpMessage = 'CMAKE_GENERATOR_PLATFORM')]
[String]$Platform,
[Parameter(Mandatory=$true, HelpMessage = 'CMAKE_GENERATOR_TOOLSET')]
[ValidateScript({ $_ -match 'host=(?<arch>[^,]*)'})]
[String]$Toolset
)

# keep all variables
$old = ls env: 
# load development environment
[xml]$info = & "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\installer\vswhere" `
                -latest -format xml
$pwshmodule = Join-Path $info.instances.instance.installationPath `
              "Common7\Tools\Microsoft.VisualStudio.DevShell.dll" | gi
Import-Module $pwshmodule

$Toolset -match 'host=(?<arch>[^,]*)' | Out-Null
$toolsetplatform = $Matches.arch

Enter-VsDevShell -VsInstanceId $info.instances.instance.instanceId `
                 -DevCmdArguments "/arch=$Platform /host_arch=$toolsetplatform" | Out-Null
# keep new variables
$new = ls env: 
# compare old and new environment
$raw = Compare-Object -ReferenceObject $old -DifferenceObject $new -Property key, value

# Split collections
$PathVars, $Vars = $raw | Group-Object {$_.key -match "Path|INCLUDE|LIB"} | Sort-Object Name -Descending

$PathVars = $PathVars.Group | Group-Object key

# New variables
$Vars = $Vars.Group
$output = $Vars | % { '{0}={1}' -f $_.key, $_.value } |
               Add-Member -NotePropertyMembers @{PropertyName="ENVIRONMENT"} -PassThru

# New Paths
$NewPaths = ($PathVars | ? Count -eq 1).Group
$NewPaths | % { 
    $name = $_.key
    $values = $_.value -split ";"
    $output += '{0}={1}' -f $name, $values[0] |
                  Add-Member -NotePropertyMembers @{PropertyName="ENVIRONMENT"} -PassThru
    $output += $values | select -skip 1 |
                  % {'{0}=path_list_append:{1}' -f $name, $_} |
                  Add-Member -NotePropertyMembers @{PropertyName="ENVIRONMENT_MODIFICATION"} -PassThru
}

# Paths to update
$UpdatePaths = ($PathVars | ? Count -eq 2).Name

$UpdatePaths | % {
        $name = $_
        $filt = {$_.key -eq $name}
        $cmp = Compare-Object -ReferenceObject $old.Where($filt).value.split(";") `
                              -DifferenceObject $new.Where($filt).value.split(";")
        $output += $cmp.InputObject |  % {'{0}=path_list_append:{1}' -f $name, $_} |
                      Add-Member -NotePropertyMembers @{PropertyName="ENVIRONMENT_MODIFICATION"} -PassThru
}

## Saving space using lists is not an option because add_custom_command() forces ; expansion
#$p_env, $p_envmod = $output | Group-Object -Property PropertyName |
#                              Sort-Object -Property Name
# $output = @() + $p_env.Name + $p_env.Group + $p_envmod.Name + $p_envmod.Group -join ";"

$output = ($output | Group-Object -Property PropertyName |
                     % { $name = $_.Name; $_.Group | % { $name; $_ } }) -join ";"

# Return a single string properly formatted for cmake taste
$output.replace("\","/")
