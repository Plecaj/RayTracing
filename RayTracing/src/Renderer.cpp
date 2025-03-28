#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::Render(){
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(),  (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // -1 -> 1 scope
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
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

uint32_t Renderer::PerPixel(glm::vec2 coord) {
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	glm::vec3 rayOrigin(0.0f, 0.0f, 3.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);

	float radius = 0.5f;
	glm::vec3 sphereCenter(0.0f, 0.0f, 0.0f);

	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius of sphere
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2 * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Quadriatic formula discriminant - b^2 - 4ac
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant >= 0) {
		float t = (-b - sqrt(discriminant)) / (2.0f * a); // closest intersection
		glm::vec3 hitPoint = rayOrigin + t * rayDirection;
		glm::vec3 normal = glm::normalize(hitPoint - sphereCenter);

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
		float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.1f);

		glm::vec3 baseColor(1.0f, 0.0f, 1.0f); // magenta
		glm::vec3 shadedColor = baseColor * lightIntensity;

		uint8_t r = static_cast<uint8_t>(shadedColor.r * 255.0f);
		uint8_t g = static_cast<uint8_t>(shadedColor.g * 255.0f);
		uint8_t b = static_cast<uint8_t>(shadedColor.b * 255.0f);

		return 0xff000000 | (r << 16) | (g << 8) | b;
	}

	return 0xff000000;
}
