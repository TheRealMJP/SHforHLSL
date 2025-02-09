dxc -enable-16bit-types -WX -HV 2021 /T cs_6_8 /E CompileTest CompileTest.hlsl
dxc -enable-16bit-types -WX -HV 2021 /T cs_6_8 /E CompileTest CompileTest_Lite.hlsl
fxc /T cs_5_0 /E CompileTest CompileTest_Lite.hlsl