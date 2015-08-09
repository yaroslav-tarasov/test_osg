// grass.frag
// blender version

#version 150

uniform float AmbientMulti, LightMulti, Intensity;
uniform sampler2D Texture2, Texture3, Texture4;

in Grass {
	vec3 LightColor;
	vec2 GrassTexCoords, ColorMulti;
} grss;

flat in int Tex;

out vec4 FragmentColor;

void main()
{
	float NearLum;
	float FarLum;

	vec4 color;

	//textures & factor for brightening with distance
	if (Tex == 1)
	{
		NearLum = 1.0;
		FarLum = 2.0;
		color = texture2D(Texture2, grss.GrassTexCoords);
	}
	else
	{
		NearLum = 0.75;
		FarLum = (Tex == 3)? 0.75: 1.5;
		color = (Tex == 3)? texture2D(Texture4, grss.GrassTexCoords): texture2D(Texture3, grss.GrassTexCoords);
	}
	
	//brighten with distance
	float Dist = gl_FragCoord.z / gl_FragCoord.w;	   //get the distance from camera
	color.rgb *= mix(NearLum, FarLum, clamp(Dist/25.0,0.0,1.5));

	//random coloring
	vec3 RandomColor = (grss.ColorMulti.x > 1.19)? vec3(0.15,0.125,0.1): vec3(0.0);    //Needs moved to the geo shader - unfortunately exceeds max output components there
	RandomColor = (grss.ColorMulti.x < 0.81)? vec3(0.1,0.04,0.04): RandomColor;        //Needs moved to the geo shader - unfortunately exceeds max output components there

	//assemble coloring and lighting
	color.rgb += RandomColor;
    color.rgb *= grss.ColorMulti.x*(AmbientMulti+1.0);
	color.rgb *= clamp(grss.LightColor*LightMulti, vec3(0.0), vec3(LightMulti*1.25));
	color.rgb *= (grss.GrassTexCoords.y*0.6+1.2)*(Intensity);

	//assemble alpha and LOD fade factoring
	color.a = color.a*grss.ColorMulti.y*1.375;

	vec4 FinalColor = color;

	FragmentColor = FinalColor;
}