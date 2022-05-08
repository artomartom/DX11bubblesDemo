
[CmdletBinding()]
param (
    [ValidateNotNullOrEmpty()]
    [string]
    $Config
)
  
begin {}
  
process {
    Import-Module 'F:\Dev\Projects\PowerShell\DXUtil.psm1';
    $in = "$($PSScriptRoot)//Source//shader//source";

     
    switch ($Config) {
        'Debug' { $out = "$($PSScriptRoot)//Source//Shader//Debug" }
        'Release' { $out = "$($PSScriptRoot)//Source//Shader//Release" }
        Default { $Config = 'Debug'; $out = "$($PSScriptRoot)//Source//Shader//Debug" }
    }
      
    $fin = 'main'
    
     
    $array = @(
        #__ OutName __________ EntryPoint _________ VarName _____________ Profile  
        @(  'VSmain'         , 'mainVertex'     , 'VSByteCode'         , 'vs_5_0'  ),
        @(  'PSmain'         , 'mainPixel'      , 'PSByteCode'         , 'ps_5_0'  ),
        @(  'CSmain'         , 'mainCompute'    , 'CSByteCode'         , 'cs_5_0'  ),
        @(  'CSCircle'       , 'CircleCompute'  , 'CSCircleByteCode'   , 'cs_5_0'  ) 
    );
    
    foreach ($each in $array) {
        Invoke-fxcCompiler  -Name $fin -Config $Config  -OutName $each[0] -OutType 'hpp'  -EntryPoint $each[1] -VarName $each[2]  -Profile $each[3]  -In $in -Out  $out ;
        if ($LASTEXITCODE -ne 0 ) {
            return $LASTEXITCODE;
        };
    }
    Remove-Module 'DXUtil'   ;
}
  
end {}

 

