vs.1.1
#include "macros.vsh"

&AllocateRegister( \$worldPos );
&AllocateRegister( \$worldNormal );
&AllocateRegister( \$projPos );
&SkinPositionAndNormal( $g_numBones, $worldPos, $worldNormal );

&AllocateRegister( \$tmp );
sub $tmp.xyz, $worldPos, c93
mul $tmp.xy, $tmp, c92
add $worldPos.xyz, $tmp, c93
&FreeRegister( \$tmp );

if( $g_numBones != 0 )
{
&Normalize( $worldNormal );
}

dp4 $projPos.x, $worldPos, $cViewProj0
dp4 $projPos.y, $worldPos, $cViewProj1
dp4 $projPos.z, $worldPos, $cViewProj2
dp4 $projPos.w, $worldPos, $cViewProj3
mov oPos, $projPos

&CalcFog( $worldPos, $projPos );
&FreeRegister( \$projPos );

&DoLighting( $g_staticLightType, $g_ambientLightType, $g_localLightType1,
	$g_localLightType2, $worldPos, $worldNormal );
&FreeRegister( \$worldNormal );

dp4 oT0.x, $vTexCoord0, c90
dp4 oT0.y, $vTexCoord0, c91

&AllocateRegister( \$tmp2 );
dp4 $tmp2.x, $vTexCoord0, c90
dp4 $tmp2.y, $vTexCoord0, c91
add oT1.xy, $tmp2, c94
&FreeRegister( \$tmp2 );

mov oT2, $cZero
&FreeRegister( \$worldPos );
