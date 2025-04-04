#include "Renderer.h"

#include "Walnut/Random.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::Render(const Scene& scene, const Camera& camera){
	const glm::vec3& rayOrigin = camera.GetPosition();

	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

void Renderer::OnResize(uint32_t width, uint32_t height){
	if (m_FinalImage) {
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) 
			return;

		m_FinalImage->Resize(width, height);
	}
	else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}


glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray) {
	// =======================================================================================
	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance
	// =======================================================================================

	if (scene.Spheres.size() == 0)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const Sphere* closestSphere = nullptr;
	float hitDistance = FLT_MAX;

	for (const Sphere& sphere : scene.Spheres) {
		glm::vec3 origin = ray.Origin - sphere.Position;

		// (bx^2 + by^2 + bz^2) --> a
		float a = glm::dot(ray.Direction, ray.Direction);

		//(2(axbx + ayby + azbz)) --> b
		float b = 2 * glm::dot(origin, ray.Direction);

		//(ax^2 + ay^2 + az^2 - r^2) --> c
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// quadriatic formula discriminant --> b^2 - 4ac
		float discriminant = b * b - 4.0f * a * c;

		if (discriminant < 0)
			continue;

		// quadriatic formula --> (-b +- sqrt(discriminant)) / 2a
		float closestT = (-b - sqrt(discriminant)) / (2.0f * a);

		if (closestT < 0.0f)
			continue;

		if (closestT < hitDistance) {
			closestSphere = &sphere;
			hitDistance = closestT;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::vec3 origin = ray.Origin - closestSphere->Position;
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f);

	glm::vec3 shadedColor = closestSphere->Albedo * lightIntensity;

	return glm::vec4(shadedColor, 1.0f);
}
