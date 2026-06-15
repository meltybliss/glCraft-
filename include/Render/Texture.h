#pragma once

#include <string>

class Texture {
public:
	Texture() = default;
	explicit Texture(const std::string& path);

	bool LoadFromFile(const std::string& path);
	void Bind(unsigned int unit = 0) const;
	void Destroy();

private:

	unsigned int m_id = 0;
	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;

};