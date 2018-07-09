#version 330
// IN
in vec3 w_normal;
in vec3 w_position;

// OUT
out vec4 FragColor;

// UNIFORMS
uniform float roughness;

uniform float intensity;
uniform vec3  dcolor;
uniform vec3  scolor;

const int MAX_POINTS = 20;
uniform int n_points;
uniform vec3 points_arr[MAX_POINTS];

uniform vec3 camera_position;

uniform sampler2D ltc_1;
uniform sampler2D ltc_2;

const float LUT_SIZE  = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;

// Linearly Transformed Cosines
///////////////////////////////

vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
    return IntegrateEdgeVec(v1, v2).z;
}

vec3 LTC_Evaluate(
    vec3 N, vec3 V, vec3 P, mat3 Minv)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[MAX_POINTS];
    for (int i = 0; i < n_points; ++i) {
      L[i] = Minv * (points_arr[i] - P);
    }

    // integrate
    float sum = 0.0;

    vec3 dir = points_arr[0].xyz - P;

    vec3 lightNormal = cross(points_arr[1] - points_arr[0], points_arr[2] - points_arr[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    for (int i = 0; i < n_points; ++i) {
      L[i] = normalize(L[i]);
    }

    vec3 vsum = vec3(0.0);

    for (int i = 0; i < n_points-1; ++i) {
      vsum += IntegrateEdgeVec(L[i], L[i+1]);
    }
    vsum += IntegrateEdgeVec(L[n_points-1], L[0]);

    float len = length(vsum);
    float z = vsum.z/len;

    if (behind)
      z = -z;

    vec2 uv = vec2(z*0.5 + 0.5, len);
    uv = uv*LUT_SCALE + LUT_BIAS;

    float scale = texture(ltc_2, uv).w;

    sum = len*scale;

    if (behind)
      sum = 0.0;

    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

// Misc. helpers
////////////////

float saturate(float v)
{
    return clamp(v, 0.0, 1.0);
}

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float gamma = 2.2;
vec3 ToLinear(vec3 v) { return PowVec3(v, gamma); }

void main()
{
    vec3 lcol = vec3(intensity);
    vec3 dcol = ToLinear(dcolor);
    vec3 scol = ToLinear(scolor);

    vec3 col = vec3(0);

		vec3 pos = w_position;

		vec3 N = normalize(w_normal);
		vec3 V = normalize(camera_position - w_position);

		float ndotv = saturate(dot(N, V));
		vec2 uv = vec2(roughness, sqrt(1.0 - ndotv));
		uv = uv*LUT_SCALE + LUT_BIAS;

		vec4 t1 = texture(ltc_1, uv);
		vec4 t2 = texture(ltc_2, uv);

		mat3 Minv = mat3(
				vec3(t1.x, 0, t1.y),
				vec3(  0,  1,    0),
				vec3(t1.z, 0, t1.w)
				);

		vec3 spec = LTC_Evaluate(N, V, pos, Minv);
		// BRDF shadowing and Fresnel
		spec *= scol*t2.x + (1.0 - scol)*t2.y;

		vec3 diff = LTC_Evaluate(N, V, pos, mat3(1));

		col = lcol*(spec + dcol*diff);

    FragColor = vec4(col, 1.0);
}
