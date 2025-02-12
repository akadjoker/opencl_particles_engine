#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>



enum class VertexType
{
    POSITION = 1,
    NORMAL,
    COLOR,
    TANGENT,
    BINORMAL,
    BLENDWEIGHTS,
    BLENDINDICES,
    TEXCOORD0,
    TEXCOORD1,
    TEXCOORD2,
    TEXCOORD3,
    TEXCOORD4,
    TEXCOORD5,
    TEXCOORD6,
    TEXCOORD7,
    VT_COUNT
};

class VertexFormat {
public:
    class Element {
    public:
        VertexType usage;
        unsigned int size;
        Element();
        Element(VertexType usage, unsigned int size);
        bool operator==(const Element &e) const;
        bool operator!=(const Element &e) const;
    };

    VertexFormat() { _vertexSize = 0; };

    VertexFormat(const Element *elements, unsigned int elementCount);
    ~VertexFormat();

    const Element &getElement(unsigned int index) const;
    unsigned int getElementCount() const;
    unsigned int getVertexSize() const;
    bool operator==(const VertexFormat &f) const;
    bool operator!=(const VertexFormat &f) const;

    VertexFormat(const VertexFormat &other)
        : _elements(other._elements), _vertexSize(other._vertexSize)
    {}

    VertexFormat &operator=(const VertexFormat &other)
    {
        if (this != &other)
        {
            _elements = other._elements;
            _vertexSize = other._vertexSize;
        }
        return *this;
    }

    VertexFormat(VertexFormat &&other) noexcept
        : _elements(std::move(other._elements)), _vertexSize(other._vertexSize)
    {
        other._vertexSize = 0;
    }

    VertexFormat &operator=(VertexFormat &&other) noexcept
    {
        if (this != &other)
        {
            _elements = std::move(other._elements);
            _vertexSize = other._vertexSize;
            other._vertexSize = 0;
        }
        return *this;
    }

    void addElement(VertexType usage, unsigned int size)
    {
        Element element(usage, size);
        _elements.push_back(element);
        _vertexSize += size * sizeof(float);
    }

    size_t getStrideInBytes() const { return _vertexSize; }
    size_t getOffsetOf(VertexType usage) const;

    bool isValid() const { return !_elements.empty(); }

private:
    std::vector<Element> _elements;
    unsigned int _vertexSize;
};


class MeshBuffer 
{
public:
    MeshBuffer(bool dynamic = false);
    ~MeshBuffer();

    void createVertexBuffer(const VertexFormat &vertexFormat,unsigned int vertexCount);
    void createIndexBuffer(unsigned int indexCount);

    void setVertexData(const void *vertexData);
    void setIndexData(const void *indexData);

    void render(int mode, int count);
    void release();

    unsigned int getVAO() const { return m_vao; }
    unsigned int getVBO() const { return m_vbo; }
    unsigned int getIBO() const { return m_ibo; }

private:
    MeshBuffer(const MeshBuffer &) = delete;
    MeshBuffer &operator=(const MeshBuffer &) = delete;
    MeshBuffer(MeshBuffer &&) = delete;
    MeshBuffer &operator=(MeshBuffer &&) = delete;

    int m_numVertices = 0;
    int m_numIndices = 0;

    bool m_useIndices = false;
    bool m_dynamic = false;
    unsigned int m_ibo = 0;
    unsigned int m_vbo = 0;
    unsigned int m_vao = 0;
    VertexFormat m_vertexFormat;
};