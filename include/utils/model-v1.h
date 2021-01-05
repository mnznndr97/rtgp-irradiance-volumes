#pragma once

using namespace std;

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <utils/mesh-v1.h>

#include <vector>
#include <iostream>

class Model
{
private:
    void LoadModel(const string &path)
    {
        Assimp::Importer importer;
        /* BUG aiProcess_CalcTangentSpace crash if UV not present */
        const aiScene *scene = importer.ReadFile(path,
                                                 aiProcess_Triangulate |               // If object is composed by quads, assimp applies some triangolation
                                                     aiProcess_JoinIdenticalVertices | // Mesh cleaning
                                                     aiProcess_FlipUVs |               // Change internal representation of UV-Space [OpenGL/Directx is TopLeft]
                                                     aiProcess_GenSmoothNormals |      // Check if the meshes comes with vertex normal
                                                     aiProcess_CalcTangentSpace);      // Creates tangent and bi tangent for vertexes
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode)
        {
            cout << "ERROR-ASSIMP: " << importer.GetErrorString() << endl;
            return;
        }
        ProcessNode(scene->mRootNode, scene);
    }

    void ProcessNode(const aiNode *node, const aiScene *scene)
    {
        for (int i = 0; i < node->mNumMeshes; i++)
        {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh));
        }
        for (int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Mesh ProcessMesh(const aiMesh *mesh)
    {
        vector<Vertex> vertices;
        vector<GLuint> indices;

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                cout << "Warning: no tex coords" << endl;
            }

            vertices.push_back(vertex);
        }

        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[i]);
            }
        }

        return Mesh(vertex, indices);
    }

public:
    vector<Mesh> meshes;

    Model(const string &path)
    {
        LoadModel(path);
    }

    void Draw()
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw();
        }
    }

    virtual ~Model()
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Delete();
        }
        meshes.clear();
    }
};
