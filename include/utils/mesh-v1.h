#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

using namespace std;

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class Mesh
{
private:
    vector<Vertex> _vertices;
    vector<GLuint> _indices;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    void SetupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // VBO is and ARRAy buffer
        glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // VBO is and ARRAy buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), &_indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // Creation of attributes named 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);

        glEnableVertexAttribArray(1); // Creation of attributes named 0
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, Normal));
        
        glEnableVertexAttribArray(2); // Creation of attributes named 0
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3); // Creation of attributes named 0
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4); // Creation of attributes named 0
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }

public:
    Mesh(const vector<Vertex> &vertices, const vector<GLuint> &indices)
    {
        _vertices = vertices;
        _indices = indices;

        SetupMesh();
    }

    void Draw()
    {
        glBindVertexArray(VAO);
        // Geometry stage on gpu
        glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Delete()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};