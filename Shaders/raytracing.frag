#version 430
#define EPSILON 0.001
#define BIG 1000000.0

struct Camera
{
	vec3 position;
	vec3 view;
	vec3 up;
	vec3 side;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
};


struct Sphere
{
	vec3 center;
	float radius;
	vec3 color;
	int material_idx;
};

struct Material
{
	float ambient;
	float diffuse;
	float specular;
	float specular_power;
};

struct Intersection
{
	float time;
	vec3 point;
	vec3 normal;
	vec3 color;
	int material_idx;
};

bool IntersectSphere(Sphere sphere, Ray ray, out float time)
{
	ray.origin -= sphere.center;
	float B = dot(ray.direction, ray.origin);
	float C = dot(ray.origin, ray.origin) - sphere.radius * sphere.radius;
	float D = B * B - C;
	if (D > 0.0)
	{
		D = sqrt(D);
		float t1 = -B - D;
		float t2 = -B + D;
		float min_t = min(t1, t2);
		float max_t = max(t1, t2);
		if (max_t < 0)
		return false;
		if (min_t < 0)
		{
			time = max_t;
			return true;
		}
		time = min_t;
		return true;
	}
	return false;
}

// ---------- in, out, uniform vars ---------- //

in vec3 interpolated_vertex;
out vec4 frag_color;

uniform Camera camera;
uniform vec2 scale;
uniform vec3 light_pos;

// ---------- scene ---------- //

//Sphere sphere = {vec3(-1.0, -1.0, -2.0), 2, 1};
layout(std430, binding = 0) buffer SphereBuffer
{
	Sphere sphere_data[];
};

Material material = {0.4, 0.9, 5, 512.0};

// ---------- functions ---------- //

bool Intersect(Ray ray, float start, float final, inout Intersection intersect)
{
	bool result = false;
	float time = start;
	intersect.time = final;
	for(int i = 0; i < sphere_data.length(); i++)
	{
		if (IntersectSphere(sphere_data[i], ray, time) && time < intersect.time)
		{
			intersect.time = time;
			intersect.point = ray.origin + ray.direction * time;
			intersect.normal = normalize(intersect.point - sphere_data[i].center);
			intersect.color = sphere_data[i].color;
			intersect.material_idx = sphere_data[i].material_idx;
			result = true;
		}
	}
	return result;
}

Ray GenerateRay(Camera camera)
{
	vec2 coords = interpolated_vertex.xy * normalize(scale);
	vec3 direction = camera.view + camera.side * coords.x + camera.up * coords.y;
	return Ray(camera.position, normalize(direction));
}

vec3 Phong(Intersection intersect, vec3 pos_light, float shadow)
{
	vec3 light = normalize(pos_light - intersect.point);
	float diffuse = max(dot(light, intersect.normal), 0.0);
	vec3 view = normalize(camera.position - intersect.point);
	vec3 reflected = reflect(-light, -intersect.normal);
	float specular = pow(max(dot(view, reflected), 0.0), material.specular_power);
	return material.ambient * intersect.color +
		shadow *
		(material.diffuse * diffuse * intersect.color +
		material.specular * specular * vec3(0, 1, 1));
}

float Shadow(vec3 pos_light, vec3 point)
{
	float light = 1.0; // Point is lighted
	vec3 direction = normalize(pos_light - point);
	Ray shadow_ray = Ray(point + direction * EPSILON, direction);
	Intersection intersect;
	intersect.time = distance(pos_light, point);
	if (Intersect(shadow_ray, 0, intersect.time, intersect))
	{
		// this light source is invisible in the intercection point
		light = 0.0;
	}
	return light;
}

vec4 Raytrace(Ray primary_ray)
{
	vec4 resultColor = vec4(0, 0, 0, 0);
	Ray ray = primary_ray;
	Intersection intersect;
	intersect.time = BIG;
	float start = 0;
	float final = BIG;
	if (Intersect(ray, start, final, intersect))
	{
		float shadowing = Shadow(light_pos, intersect.point);
		resultColor += vec4(Phong(intersect, light_pos, shadowing), 0);
	}
	return resultColor;
}

// ---------- main ---------- //

void main ( void )
{
	// ---------- trace ---------- //
	Ray ray = GenerateRay(camera);
	frag_color = Raytrace(ray);
	//frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}