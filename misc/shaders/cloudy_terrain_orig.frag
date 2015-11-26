// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    float a = texture2D( iChannel0, x.xy/256.0 + (p.z+0.0)*120.7123 ).x;
    float b = texture2D( iChannel0, x.xy/256.0 + (p.z+1.0)*120.7123 ).x;
	return mix( a, b, f.z );
}


const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}

float envelope( vec3 p )
{
	float isLake = 1.0-smoothstep( 0.62, 0.72, texture2D( iChannel0, 0.001*p.zx,-100.0).x );
	return 0.1 + isLake*0.9*texture2D( iChannel1, 0.01*p.xz ).x;
}

float mapTerrain( in vec3 pos )
{
	return pos.y - envelope(pos);
}

float raymarchTerrain( in vec3 ro, in vec3 rd )
{
	float maxd = 50.0;
	float precis = 0.001;
    float h = 1.0;
    float t = 0.0;
    for( int i=0; i<80; i++ )
    {
        if( abs(h)<precis||t>maxd ) break;
        t += h;
	    h = mapTerrain( ro+rd*t );
    }

    if( t>maxd ) t=-1.0;
    return t;
}

vec3 lig = normalize( vec3(0.7,0.4,0.2) );

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3(0.02,0.0,0.0);
	return normalize( vec3(
           mapTerrain(pos+eps.xyy) - mapTerrain(pos-eps.xyy),
           0.5*2.0*eps.x,
           mapTerrain(pos+eps.yyx) - mapTerrain(pos-eps.yyx) ) );

}

vec4 mapTrees( in vec3 pos, in vec3 rd )
{
    vec3  col = vec3(0.0);	
	float den = 1.0;

	float kklake = texture2D( iChannel0, 0.001*pos.zx,-100.0).x;
	float isLake = smoothstep( 0.7, 0.71, kklake );
	
	if( pos.y>1.0 || pos.y<0.0 ) 
	{
		den = 0.0;
	}
	else
	{
		
		float h = pos.y;
		float e = envelope( pos );
		float r = clamp(h/e,0.0,1.0);
		
        den = smoothstep( r, 1.0, texture2D(iChannel0, pos.xz*0.15, -100.0).x );
        
		den *= 1.0-0.95*clamp( (r-0.75)/(1.0-0.75) ,0.0,1.0);
		
        float id = texture2D( iChannel0, pos.xz,-100.0).x;
        float oc = pow( r, 2.0 );

		vec3  nor = calcNormal( pos );
		vec3  dif = vec3(1.0)*clamp( dot( nor, lig ), 0.0, 1.0 );
		float amb = 0.5 + 0.5*nor.y;
		
		float w = (2.8-pos.y)/lig.y;
		float c = fbm( (pos+w*lig)*0.35 );
		c = smoothstep( 0.38, 0.6, c );
		dif *= pow( vec3(c), vec3(0.8, 1.0, 1.5 ) );
			
		vec3  brdf = 1.7*vec3(1.5,1.0,0.8)*dif*(0.1+0.9*oc) + 1.3*amb*vec3(0.1,0.15,0.2)*oc;

		vec3 mate = 0.6*vec3(0.5,0.5,0.1);
		mate += 0.3*texture2D( iChannel1, 0.1*pos.xz ).zyx;
		
		col = brdf * mate;

		den *= 1.0-isLake;
	}

	return vec4( col, den );
}


vec4 raymarchTrees( in vec3 ro, in vec3 rd, float tmax, vec3 bgcol )
{
	vec4 sum = vec4(0.0);
    float t = tmax;
	for( int i=0; i<512; i++ )
	{
		vec3 pos = ro + t*rd;
		if( sum.a>0.99 || pos.y<0.0  || t>20.0 ) break;
		
		vec4 col = mapTrees( pos, rd );

		col.xyz = mix( col.xyz, bgcol, 1.0-exp(-0.0018*t*t) );
        
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);	
		
		t += 0.0035*t;
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

vec4 mapClouds( in vec3 p )
{
	float d = 1.0-0.3*abs(2.8 - p.y);
	d -= 1.6 * fbm( p*0.35 );

	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	res.xyz = mix( 0.8*vec3(1.0,0.95,0.8), 0.2*vec3(0.6,0.6,0.6), res.x );
	res.xyz *= 0.65;
	
	return res;
}

float kk = 0.0;

vec4 raymarchClouds( in vec3 ro, in vec3 rd, in vec3 bcol, float tmax )
{
	vec4 sum = vec4(0, 0, 0, 0);

	float sun = clamp( dot(rd,lig), 0.0, 1.0 );
	float t = 0.0;
	for(int i=0; i<64; i++)
	{
		if( sum.w>0.99 || t>tmax ) break;
		vec3 pos = ro + t*rd;
		vec4 col = mapClouds( pos );

		float dt = max(0.1,0.05*t);
		float h = (2.8-pos.y)/lig.y;
		float c = fbm( (pos + lig*h)*0.35 );
		//kk += 0.05*dt*(smoothstep( 0.38, 0.6, c ))*(1.0-col.a);
		kk += 0.02*(smoothstep( 0.38, 0.6, c ))*(1.0-col.a)*(1.0-smoothstep(2.75,2.8,pos.y));
	
		
		col.xyz *= vec3(0.4,0.52,0.6);
		
        col.xyz += vec3(1.0,0.7,0.4)*0.4*pow( sun, 6.0 )*(1.0-col.w);
		
		col.xyz = mix( col.xyz, bcol, 1.0-exp(-0.0018*t*t) );
		
		col.a *= 0.5;
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);	

		t += dt;//max(0.1,0.05*t);
	}
kk = clamp( kk, 0.0, 1.0 );
	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

vec3 path( float time )
{
	return vec3( 32.0*cos(0.2+0.75*.1*time*1.5), 1.2, 32.0*sin(0.1+0.75*0.11*time*1.5) );
}

mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 q = fragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;
	
	float time = 23.5+iGlobalTime;
	
    // camera	
	vec3 ro = path( time );
	vec3 ta = path( time+1.0 );
	ta.y *= 0.2;
	float roll = 0.3*cos(0.07*time);
	// camera tx
    mat3 cam = setCamera( ro, ta, roll );

    // ray direction
    vec3 rd = normalize( cam * vec3(p.xy,1.75) );

    // sky	 
	vec3 col = vec3(0.84,0.95,1.0)*0.77 - rd.y*0.6;
	col *= 0.75;
	float sun = clamp( dot(rd,lig), 0.0, 1.0 );
    col += vec3(1.0,0.7,0.3)*0.3*pow( sun, 6.0 );
	vec3 bcol = col;

    // lakes
    float gt = (0.0-ro.y)/rd.y;
    if( gt>0.0 )
    {
        vec3 pos = ro + rd*gt;

		vec3 nor = vec3(0.0,1.0,0.0);
	    nor.xz  = 0.10*(-1.0 + 2.0*texture2D( iChannel3, 1.5*pos.xz ).xz);
	    nor.xz += 0.15*(-1.0 + 2.0*texture2D( iChannel3, 3.2*pos.xz ).xz);
	    nor.xz += 0.20*(-1.0 + 2.0*texture2D( iChannel3, 6.0*pos.xz ).xz);
		nor = normalize(nor);

		vec3 ref = reflect( rd, nor );
	    vec3 sref = reflect( rd, vec3(0.0,1.0,0.0) );
		float sunr = clamp( dot(ref,lig), 0.0, 1.0 );

	    float kklake = texture2D( iChannel0, 0.001*pos.zx,-100.0).x;
		col = vec3(0.1,0.1,0.0);
        vec3 lcol = vec3(0.2,0.5,0.7);
		col = mix( lcol, 1.1*vec3(0.2,0.6,0.7), 1.0-smoothstep(0.7,0.81,kklake) );
		
		col *= 0.12;

	    float fre = 1.0 - max(sref.y,0.0);
		col += 0.8*vec3(1.0,0.9,0.8)*pow( sunr, 64.0 )*pow(fre,1.0);
		col += 0.5*vec3(1.0,0.9,0.8)*pow( fre, 10.0 );

		float h = (2.8-pos.y)/lig.y;
        float c = fbm( (pos+h*lig)*0.35 );
		col *= 0.4 + 0.6*smoothstep( 0.38, 0.6, c );

	    col *= smoothstep(0.7,0.701,kklake);

	    col.xyz = mix( col.xyz, bcol, 1.0-exp(-0.0018*gt*gt) );
    }


    // terrain	
	float t = raymarchTerrain(ro, rd);
    if( t>0.0 )
	{
        // trees		
        vec4 res = raymarchTrees( ro, rd, t, bcol );
	    col = mix( col, res.xyz, res.w );
	}

	// sun glow
    col += vec3(1.0,0.5,0.2)*0.35*pow( sun, 3.0 );

    // clouds	
	{
	if( t<0.0 ) t=600.0;
    vec4 res = raymarchClouds( ro, rd, bcol, t );
	col = mix( col, res.xyz, res.w );
	}

	//col += 0.2*vec3(1.0,0.8,0.7)*(kk);
	//col = vec3(smoothstep(0.3,0.4,kk));
	col += (1.0-0.8*col)*kk*kk*kk*0.3*vec3(1.0,0.8,0.7);
	col = clamp( col, 0.0, 1.0 );

	
    // gamma	
	col = pow( col, vec3(0.45) );

    // contrast, desat, tint and vignetting	
	col = col*0.1 + 0.9*col*col*(3.0-2.0*col);
	col = mix( col, vec3(col.x+col.y+col.z)*0.33, 0.2 );
	col *= vec3(1.06,1.05,1.0);
	col *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );

	
	fragColor = vec4( col, 1.0 );
}