// grass.geom
// blender version

#version 400 compatibility

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 80) out;

//------------------------------------------------------------------------------------------------------------------------

uniform sampler2D Texture1;
uniform vec4 Radius1, Radius2;
uniform float Time, windStrength, AmbientMulti, LODdist;

//------------------------------------------------------------------------------------------------------------------------

const float invo = 6.0;                     //needs to match the aboves 'invocations =' - # of instances
const int Lights = 6;                       //number of Lights to add - if it was dynamic, it would be very expensive
const float grassLength = 0.625;			//overall grass height, should be less than 1.0 - decreasing improves performance
const vec2 windDirection = vec2(-1.0,0.5);  //initial direction of the wind - changes
const int windSamples = 3;                  //sine waves to add for wind calcs - 1-4 looks good

//------------------------------------------------------------------------------------------------------------------------

in GroundData {
	vec2 TexCoords;
	vec3 Normal;
} ground[3];

out Grass {
	vec3 LightColor;
	vec2 GrassTexCoords, ColorMulti;
} grass;

flat out int Tex;

//------------------------------------------------------------------------------------------------------------------------

mat3 zRotationMatrix(float angle)   //rotation matrix for Z-axis rotation
{
	float s = sin(angle);
	float c = cos(angle);
	
	mat3 rotate = mat3( c,  -s,  0.0,
						s,   c,  0.0,
					   0.0, 0.0, 1.0);
					   
	return rotate;
}

vec2 getWind(vec2 worldPos, float height)   //sine wave wind function
{
	float windTime = Time+(height*(grassLength*3.0));
	float windDisplacement = cos(0.375*((17.5+worldPos.x)+(17.5+worldPos.y))+(windTime*1.25));

	for (int w = 0; w < windSamples; w++)
	{
		float rAnd = float(w)+1.0-(float(windSamples)/2.0);
		float rCnd = float(w)-1.0+(float(windSamples)/2.0);
		windDisplacement += sin(0.5*((17.5+rAnd+worldPos.x)+(rAnd+worldPos.y))+(windTime*(rAnd*0.1+1.75)));
		windDisplacement -= cos(0.5*((17.5+rCnd+worldPos.x)+(rAnd+worldPos.y))+(windTime*(rCnd*0.1+1.75)));
	}

	vec2 Wind = windStrength*(height*grassLength)*sin((worldPos.xy*normalize(windDirection))+vec2(windTime*0.5))*windDisplacement;

	return Wind;
}

vec3 getPos(vec3 OPos, vec3 Pos1, vec3 Pos2, vec3 Pos3, int iter)   //evenly distributed points per face - used for normal as well
{
	vec3 NPos = OPos;
	NPos = (iter == 1)? OPos*1.5+Pos1+Pos2: NPos;
	NPos = (iter == 2)? OPos*1.5+Pos2+Pos3: NPos;
	NPos = (iter == 3)? OPos*1.5+Pos3+Pos1: NPos;
	NPos = (iter == 4)? (OPos*2.25)+(Pos1*1.25): NPos;
	NPos = (iter == 5)? (OPos*2.25)+(Pos2*1.25): NPos;
	NPos = (iter == 6)? (OPos*2.25)+(Pos3*1.25): NPos;
	NPos = (iter == 7)? OPos+Pos1*2.0: NPos;
	NPos = (iter == 8)? OPos+Pos2*2.0: NPos;
	NPos = (iter == 9)? OPos+Pos3*2.0: NPos;
	
	if (iter != 0)
	{
		NPos /= (iter > 6)? 3.0: 3.5;
	}

	return NPos;
}

//directional lighting
vec3 getDirectLighting(vec3 Normal, int i)
{
	float NdotL = clamp(dot(normalize(Normal), gl_LightSource[i].position.xyz),0.0,0.625);
	vec3 Light = (NdotL > AmbientMulti)? gl_LightSource[i].diffuse.rgb * NdotL: gl_LightSource[i].diffuse.rgb * AmbientMulti;

	return Light;
}

//-----------------------------------            1 / (   1     +   .1  *d +    .01   *d^2)
//point lighting - 1 / (d/r + 1)^2  = same as =  1 / (constant + linear*d + quadratic*d^2)
vec3 getPointLighting(vec3 Normal, int i, vec3 VertPos, float radius)
{
	vec3 PointLight = vec3(0.0);
	vec3 len = (gl_LightSource[i].position.xyz - VertPos);
	float NdotL = clamp(dot(normalize(Normal),normalize(len)), 0.0, 1.0)*0.625;    //*.625 to equalize it with directional lights that are clamped to less than 1

	if (NdotL > 0.0)
	{
		float Dist2Light = length(len);
		float Att = 1.0 / pow((Dist2Light/radius) + 1.0, 2.0);

		PointLight = Att * NdotL * gl_LightSource[i].diffuse.rgb;
	}
	
	return PointLight;
}

//------------------------------------------------------------------------------------------------------------------------

void main()
{
	//UV center -----------------------
	vec2 TCoords1 = ground[0].TexCoords;
	vec2 TCoords2 = ground[1].TexCoords;
	vec2 TCoords3 = ground[2].TexCoords;
	
	vec2 PosCoords = (TCoords1+TCoords2+TCoords3) / 3.0;
	//----------------------------------------------------

	//first cull check
	float Height = texture2D(Texture1, PosCoords).r;//*0.5+0.5;
	if (Height >= 0.25)
	{
		Tex = 1;										  //used to determine texture
		float LodF = 1.0;                                 //initial LOD fade factor - 0.0 to test LOD
		float inst = float(gl_InvocationID)-(invo/2.0);   //used to randomize the instances
		float instCheck = float(gl_InvocationID);

		mat4 viewProjectionMatrix = gl_ModelViewProjectionMatrix;
		mat4 ModelViewMatrix = gl_ModelViewMatrix;

		//face center------------------------
		vec3 Vert1 = gl_in[0].gl_Position.xyz;
		vec3 Vert2 = gl_in[1].gl_Position.xyz;
		vec3 Vert3 = gl_in[2].gl_Position.xyz;

		vec3 CenterPos = (Vert1+Vert2+Vert3) / 3.0;   //Center of the triangle - copy for later
		vec3 Pos = CenterPos;                         //Center of the triangle
		//-----------------------------------

		//------ original face normal
        vec3 Norm1 = ground[0].Normal;
        vec3 Norm2 = ground[1].Normal;
        vec3 Norm3 = ground[2].Normal;

		vec3 NormC = (ground[0].Normal+ground[1].Normal+ground[2].Normal) / 3.0;  //copy for later
		vec3 Norm = NormC;

        //New UV coords array------------------------------------------------------
        //tested - cheaper to calculate new uv coords here and store it in an array
        //for Pos & Norm this method is more expensive
        vec2 NCoords[10];
    	NCoords[0] = PosCoords;
    	NCoords[1] = (PosCoords*1.5+TCoords1+TCoords2)/3.5;
    	NCoords[2] = (PosCoords*1.5+TCoords2+TCoords3)/3.5;
    	NCoords[3] = (PosCoords*1.5+TCoords3+TCoords1)/3.5;
    	NCoords[4] = ((PosCoords*2.25)+(TCoords1*1.25))/3.5;
    	NCoords[5] = ((PosCoords*2.25)+(TCoords2*1.25))/3.5;
    	NCoords[6] = ((PosCoords*2.25)+(TCoords3*1.25))/3.5;
    	NCoords[7] = (TCoords1*2.0+PosCoords)/3.0;
    	NCoords[8] = (TCoords2*2.0+PosCoords)/3.0;
    	NCoords[9] = (TCoords3*2.0+PosCoords)/3.0;
        //-------------------------------------------------------------------------

		//randomize the height a little
		float GrassMulti = (sin((cos((Pos.x+Pos.y)+sin(Pos.x*Pos.y))*0.1)+(float(gl_InvocationID)/(invo))*0.5+0.5)*0.5+0.5);

		//LOD factores ----------------------------------------------------------------------------------
		float LODi = LODdist;                 //first LOD distance - might can make this a property later
		float LODi2x = LODi*2.0;			  //second LOD distance
		float Trans = -LODi/3.0;			  //distance to transition LOD levels

		LODi += inst*(-Trans/(invo/2.0));	  //layer LOD levels with instances to "blend the line"
		LODi2x += inst*(-Trans/(invo/2.0));   //layer LOD levels with instances to "blend the line"

		float VertDist = (viewProjectionMatrix*vec4(Pos, 1.0)).z;
		//-----------------------------------------------------------------------------------------------

		//------ LOD - # of grass objects to spawn per-face
		int Density = (VertDist <= LODi)? 10: 7;
		Density = (VertDist >= LODi2x)? 4: Density;
		//Density = 10;   //used for testing disables LOD - need to comment out LOD transition if used

		inst *= invo;

		//first pass to randomize the position using face center as seed
		float SinX = inst+sin(inst*(Pos.x/(Pos.y*0.1592)));
		float SinY = inst+cos(inst*(Pos.y/(Pos.x*0.1592)));

		int ck1 = int(invo*0.8);	//texture 2 check
		int ck2 = int(invo*0.45);   //texture 3 check
		int ck3 = int(invo*0.9);	//texture 3 check

		vec3 LightCol = (Lights != 0)? vec3(AmbientMulti): vec3(1.0+AmbientMulti);

		//getting ready to draw objects
		for (int o = 0; o < Density; o++)
		{
			//prep second cull check with more finalized positions - resolution = (face/10)
            PosCoords = NCoords[o];
			Height = texture2D(Texture1, PosCoords).r;

			//determine the textures
			Tex = (gl_InvocationID == ck1 && o == 5)? 2: Tex;
			Tex = ((gl_InvocationID == ck2 || gl_InvocationID == ck3) && o == 6)? 3: Tex;

			//get a more finalized normal using copy made earlier
			Norm = getPos(NormC, Norm1, Norm2, Norm3, o);

			float FinalGrassSize = ((Norm.z+2.0)/3.0)*grassLength*GrassMulti;

			Norm = normalize(gl_NormalMatrix * Norm);

			//different height for different grass types
			if (Tex != 1)
			{
				FinalGrassSize *= (Tex == 2)? 1.5: 1.25;
			}

			//height map for grass
			FinalGrassSize *= (Height*0.75+0.25);

			//becomes stecil map - last cull check
			if (FinalGrassSize >= 0.3)
			{
				float instO = (float(o)-5.0)/10.0;

				//Fading in/out the LOD levels--------------------
				if (o >= 3 && o < 7 && VertDist >= (LODi2x+Trans))
				{
					LodF = clamp((VertDist-LODi2x)/Trans,0.0,1.0);     //transitioning - smooths the LOD transition

				} else
				if (o >= 7 && VertDist >= (LODi+Trans))
				{
					LodF = clamp((VertDist-LODi)/Trans,0.0,1.0);       //transitioning - smooths the LOD transition

				}

				float GrassBottom = FinalGrassSize*0.75;
				float GrassTop = FinalGrassSize*1.125;
				//------------------------------------------------

				//finalizing the position---------------------------
				Pos = getPos(CenterPos, Vert1, Vert2, Vert3, o);
				Pos.xy += vec2(cos(Pos.x*SinX), sin(Pos.y*SinY));       //offest by a random amount
				//--------------------------------------------------

				//randomizing the color - used in fragment shader
				float rCol = 0.2*sin((instO+inst)*(Pos.x+Pos.y));
				rCol += 1.0;

				grass.ColorMulti = vec2((rCol), LodF);

				//randomizing the angle for rotation
				float RandomAngle = sin(((Pos.x*Pos.y)/(Pos.x+Pos.y))*(inst+instO));

				//add the wind
				vec3 Wind = Pos+vec3(getWind(Pos.xy, FinalGrassSize), 0.0)+vec3(0.0,0.0,FinalGrassSize);

				//------------------------------ LIGHTING ------------------------------\\

				grass.LightColor = LightCol;

				vec3 Pos4L = vec3(0.0);
				Pos4L = (ModelViewMatrix * vec4((Wind+Pos)/2.0,1.0)).xyz;  //center of grass object
				float lightRadius[8];

				int NumOfLights = clamp(Lights, 0, 8);

				for (int l = 0; l < NumOfLights; l++)
				{
					lightRadius[l] = (l < 4)? Radius1[l]: Radius2[l-4];

					if (gl_LightSource[l].position.w == 0)
					{
						//Directional Light
						grass.LightColor += getDirectLighting(Norm, l);

					} else
					{
						//Point Light
						grass.LightColor += getPointLighting(Norm, l, Pos4L, lightRadius[l]);

					}
				}

				//------------------------------ LIGHTING ------------------------------\\

				//draw grass objects
				for (int g = 0; g < 4; g++)
				{
					//prep random rotation
					RandomAngle += (g != 0)? 2.0943951: 0.0;	//2.0943951 = radians(120) = 120 degrees
					mat3 rotMat = zRotationMatrix(RandomAngle);

					//bottom verts
					gl_Position = viewProjectionMatrix * vec4((rotMat*vec3(GrassBottom,0.0,0.0))+Pos, 1.0);
					grass.GrassTexCoords = vec2(float(g), 0.0);
					EmitVertex();

					//top verts
					gl_Position = viewProjectionMatrix * vec4((rotMat*vec3(GrassTop,0.0,0.0))+Wind, 1.0);
					grass.GrassTexCoords = vec2(float(g), 1.0);
					EmitVertex();

				}
			EndPrimitive();
			}
		}
	}
}