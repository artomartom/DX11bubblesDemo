


Copy-Item -Path "$($PSScriptRoot)\Source\Assets" `
-Destination "$($PSScriptRoot)\Build\x64\Debug" -Recurse  -ErrorAction SilentlyContinue
 


$in  = "$($PSScriptRoot)//Source//shader//source";
$out = "$($PSScriptRoot)//build//x64";
$fin = 'main'


Compile-hlslFile -Name $fin -Config  'Debug' -OutName "Vertex" -OutType 'so'-EntryPoint 'vmain'-Profile 'vs_5_0'  -In $in -Out  "$($out )//Debug"
Compile-hlslFile -Name $fin -Config  'Debug' -OutName "Pixel" -OutType 'so'-EntryPoint 'pmain'-Profile 'ps_5_0'  -In $in -Out  "$($out )//Debug"

Compile-hlslFile -Name $fin -Config  'Release'  -OutName "Vertex" -OutType 'so'-EntryPoint 'vmain'-Profile 'vs_5_0'  -In $in -Out  "$($out )//Release"
Compile-hlslFile -Name $fin -Config  'Release'  -OutName "Pixel" -OutType 'so'-EntryPoint 'pmain'-Profile 'ps_5_0'  -In $in -Out  "$($out )//Release"
 