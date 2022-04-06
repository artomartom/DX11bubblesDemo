


 

$in  = "$($PSScriptRoot)//Source//shader//source";
$out   = "$($PSScriptRoot)//Source//Shader";
  
$fin = 'main'


Compile-hlslFile -Name $fin -Config  'Debug' -OutName "Vertex" -OutType 'hpp'-EntryPoint 'vmain'-Profile 'vs_5_0'  -In $in -Out   "$($out  )//Debug"  -VarName "VertexByteCode"
Compile-hlslFile -Name $fin -Config  'Debug' -OutName "Pixel" -OutType 'hpp'-EntryPoint 'pmain'-Profile 'ps_5_0'  -In $in -Out     "$($out  )//Debug"  -VarName "PixelByteCode"

Compile-hlslFile -Name $fin -Config  'Release'  -OutName "Vertex" -OutType 'hpp'-EntryPoint 'vmain'-Profile 'vs_5_0'  -In $in -Out   "$($out  )//Release"  -VarName "VertexByteCode"
Compile-hlslFile -Name $fin -Config  'Release'  -OutName "Pixel" -OutType 'hpp'-EntryPoint 'pmain'-Profile 'ps_5_0'  -In $in -Out  "$($out  )//Release"  -VarName "PixelByteCode"
 