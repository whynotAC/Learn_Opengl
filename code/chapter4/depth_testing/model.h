//
//  model.h
//  hello_window
//
//  Created by 李文龙 on 2018/3/15.
//  Copyright © 2018年 liwenlong. All rights reserved.
//

#ifndef model_h
#define model_h

#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "mesh.h"
#include "shader.h"
#include "stb_image.h"

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

unsigned int TextureFromFile(char const *path, const string &directory, bool gamma = false);

class Model {
public:
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gamaCorrection;
    
    Model(string const &path, bool gamma = false) : gamaCorrection(gamma) {
        loadModel(path);
    }
    
    void Draw(Shader shader) {
        for (unsigned int index = 0; index < meshes.size(); index++) {
            meshes[index].Draw(shader);
        }
    }
    
private:
    void loadModel(string const &path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        
        processNode(scene->mRootNode, scene);
    }
    
    void processNode(aiNode *node, const aiScene *scene) {
        for (unsigned int index = 0; index < node->mNumMeshes; index++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[index]];
            meshes.push_back(prcessMesh(mesh, scene));
        }
        
        for (unsigned int index = 0; index < node->mNumChildren; index++) {
            processNode(node->mChildren[index], scene);
        }
    }
    
    Mesh prcessMesh(aiMesh *mesh, const aiScene *scene) {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        
        // walk through each of the mesh's vertices
        for (unsigned int index = 0; index < mesh->mNumVertices; index++) {
            Vertex vertex;
            glm::vec3 vector;
            // positions
            vector.x = mesh->mVertices[index].x;
            vector.y = mesh->mVertices[index].y;
            vector.z = mesh->mVertices[index].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[index].x;
            vector.y = mesh->mNormals[index].y;
            vector.z = mesh->mNormals[index].z;
            vertex.Normal = vector;
            // texture coordinates
            if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][index].x;
                vec.y = mesh->mTextureCoords[0][index].y;
                vertex.TexCoords = vec;
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            // tangent 切线
            vector.x = mesh->mTangents[index].x;
            vector.y = mesh->mTangents[index].y;
            vector.z = mesh->mTangents[index].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[index].x;
            vector.y = mesh->mBitangents[index].y;
            vector.z = mesh->mBitangents[index].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int index = 0; index < mesh->mNumFaces; index++) {
            aiFace face = mesh->mFaces[index];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int innerIndex = 0; innerIndex < face.mNumIndices; innerIndex++) {
                indices.push_back(face.mIndices[innerIndex]);
            }
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders.Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes;
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        
        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }
    
    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int index = 0; index < mat->GetTextureCount(type); index++) {
            aiString str;
            mat->GetTexture(type, index, &str);
            // check if texture was loaded before and if so, continue to next iteration; skip loading a new texture
            bool skip = false;
            for (unsigned int innerIndex = 0; innerIndex < textures_loaded.size(); innerIndex++) {
                if (std::strcmp(textures_loaded[innerIndex].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[innerIndex]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one.
                    break;
                }
            }
            if (!skip) {
                // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // store it as texture loaded for entrie model, to ensure we won't unnecessery load duplicate textures.
            }
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path" << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}

#endif /* model_h */
