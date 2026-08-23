#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class TextureObject {
protected:
    GLuint m_id = 0;

public:
    TextureObject() = default;
    ~TextureObject() {
        Destroy();
    };

    TextureObject(const TextureObject&) = delete;
    TextureObject& operator=(const TextureObject&) = delete;

    TextureObject(TextureObject&& other) noexcept;
    TextureObject& operator=(TextureObject&& other) noexcept;

    GLuint GetID() const { return m_id; }
    void Destroy() {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
            m_id = 0;
        }
    }

};