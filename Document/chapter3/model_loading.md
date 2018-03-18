模型加载
===============================
Assimp
-----------------------
目前我们一直使用的箱子朋友，是我们自己定义好的顶点、法线和纹理坐标。这些可能不能满足我们后面的开发需求。我们需要更加专业的3D艺术家在Blender、3DS Max或者Maya这样专业的工具中制作出来的经典模型加入我们的OpenGL程序中。

这些所谓的**3D建模工具(3D Modeling Tool)**可以让艺术家创建复杂的形状，并使用一种叫做**UV映射(uv-mapping)**的手段来应用贴图。这些工具将会在导出模型文件的时候自动生成所有的顶点坐标、顶点法线以及纹理坐标。我们需要了解这些模型文件，并将其加入我们的程序中。

我们的工作就是解析这些到处的模型文件以及提取所有有用的信息，将它们存储为OpenGL能够理解的格式。一个很常见的问题是，模型的文件格式有很多种，每一种都会以它们自己的方式来倒出模型数据。然而，我们很幸运的有一个库专门处理这个问题。

**模型加载库**

一个非常流行的模型导入库是**Assimp**，它是**Open Asset Import Library(开放的资产导入库)**的缩写。Assimp能够导入很多种不同的模型文件格式，它会将所有的模型数据加载至Assimp的通用数据结构中。当Assimp加载完模型之后，就能够从Assimp的数据结构中提取出所需的所有数据。由于Assimp的数据结构保持不变，不论导入的是什么种类的文件格式，它都能够将我们从这些不同的文件格式中抽象出来，用同一种方式访问所需的数据。

当使用Assimp导入一个模型的时候，它通常会将整个模型加载进一个**场景(Scene)**对象，它会包含导入的模型/场景中的所有数据。Assimp会将场景载入为一系列的节点(Node)，每个节点包含了场景对象中所储存数据的索引，每个节点都可以有任意数量的字节点。Assimp数据结构的(简化)模型如下:

![Assimp模型加载结构图](https://github.com/whynotAC/Learn_Opengl/blob/master/Document/chapter3/assimp_struct.jpg)

* 材质和网格(`Mesh`)一样，所有的场景/模型数据都包含在`Scene`对象中。`Scene`对象也包含了场景根节点的引用。
* 场景的`Root node`(根节点)可能包含子节点(和其他的节点一样)，它会有一系列指向场景对象中`mMeshes`数组中储存的网格数据的索引。`Scene`下的`mMeshes`数组储存了真正的`Mesh`对象，节点中的`mMeshes`数组保存的只是场景中网格数组的索引。
* 一个`Mesh`对象本身包含了渲染所需要的所有相关数据，像是顶点位置、法向量、纹理坐标、面(`Face`)和物体的材质。
* 一个网格包含了多个面。`Face`代表的是物体的渲染图元(`Primitive`)(三角形、方形、点)。一个面包含了组成图元的顶点的索引。由于顶点和索引是分开的，使用一个索引缓冲来渲染是非常简单的。
* 最后，一个网格在包含了一个`Material`对象，它包含了一些函数能让我们获取物体的材质属性。

所以，需要做的第一件事是将一个物体加载到`Scene`对象中，遍历节点，获取对应的`Mesh`对象(需要递归搜索每个节点的子节点)，并处理每个`Mesh`对象来获取顶点数据、索引以及它的材质属性。最终的结果是一系列的网格数据，会将它们包含在一个`Model`对象中。

>网格：当使用建模工具对物体建模的时候，艺术家通常不会用单个形状创建出整个模型。通常每个模型都由几个字模型/形状组合而成。组合模型的每个单独的形状就叫做一个**网格(Mesh)**。一个网格在OpenGL中绘制物体所需的最小单位(顶点数据、索引和材质属性)。一个模型(通常)会包括多个网格。

如果想要绘制一个模型，不需要将整个模型渲染为一个整体，只需要渲染组成模型的每个独立的网格就可以了。

网格
----------------------
通过使用Assimp，可以加载不同的模型到程序中，但是载入后它们都被储存为Assimp的数据结构。最终仍要将这些数据转换为OpenGL能够理解的格式，这样才能渲染物体。网格(Mesh)代表的是单个的可绘制实体，现在先来定义一个网格类。

**一个网格应该至少需要一系列的顶点，每个顶点包含了一个位置向量、一个法向量和一个纹理坐标向量。一个网格还应该包含用于索引绘制的索引以及纹理形式的材质数据。**

既然有了一个网格类的最低需求，可以在OpenGL中定义一个顶点:

		struct Vertex {
			glm::vec3 Position;
			glm::vec3 Noraml;
			glm::vec2 TexCoords;
		};

将所有需要的向量储存到一个叫做`Vertex`的结构体中，可以用它来索引每个顶点属性。除了`Vertex`结构体之外，还需要将纹理数据整理到一个`Texture`结构体中。

		struct Texture {
			unsigned int id;
			string type;
		};

知道了顶点和纹理的实现，可以开始定义网格类的结构了:

		class Mesh {
		public:
			// 网格数据
			vector<Vertex> vertices;
			vector<unsigned int> indices;
			vector<Texture> textures;
			// 函数
			Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
			void Draw(Shader shader);
		private:
			// 渲染数据
			unsigned int VAO, VBO, EBO;
			// 函数
			void setupMesh();
		};
		
**初始化**

由于有了构造器，现在有一大列的网格数据用于渲染。在此之前还必须配置正确的缓冲，并通过顶点属性指针定义顶点着色器的布局。利用C++结构体中内存布局是连续的属性和结构体的另外一个`offsetof(s, m)`预处理指令来快速的完成数据缓冲区的定义。这对于结构体的扩展带来了极大地方便。

**渲染**

在真正渲染这个网格之前，需要在调用`glDrawElements`函数之前先绑定相应的纹理。然而，实际上有些困难，应为一开始并不知道这个网格有多少纹理、纹理是什么类型的。所以如何在着色器中设置纹理单元和采样器呢？

为了解决这个问题，需要设定一个命名标准:每个漫反射纹理被命名为`texture_diffuseN`，每个镜面光纹理应该被命名为`texture_specularN`，其中`N`的范围是1到纹理采样器最大允许的数字。例如对某个网格有3个漫反射纹理，2个镜面光纹理，它们的纹理采样器应该之后被调用:

	uniform sampler2D texture_diffuse1;
	uniform sampler2D texture_diffuse2;
	uniform sampler2D texture_diffuse3;
	uniform sampler2D texture_specular1;
	uniform sampler2D texture_specular2;
	
根据这个标准，可以在着色器中定义任意需要数量的纹理采样器。

模型
---------------------------
本节会使用Assimp来加载模型，并将它转换(Translate)至多个在上节创建的`Mesh`对象。`Model`类的结构体如下:

	class Model {
		public:
			Model (char *path) {
				loadModel(path);
			}
			void Draw(Shader shader);
		private:
			// 模型数据
			vector<Mesh> meshes;
			string directory;
			// 函数
			void loadModel(string path);
			void processNode(aiNode *node, const aiScene *scene);
			Mesh processMesh(aiMesh *mesh, const aiScene *scene);
			vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
	};
	
`Model`类包含了一个`Mesh`对象的vector，用于存放所需要绘制的数据。

**导入3D模型到OpenGL**

使用assimp来导入数据，需要添加的头文件如下:
	
		#include <assimp/Importer.hpp>
		#include <assimp/scene.h>
		#include <assimp/postprocess.h>
		
使用Assimp来加载模型至Assimp的一个叫做`scene`的数据结构中。一旦有了这个场景对象，就能访问到加载模型中所有所需的数据。

Assimp的优势在于它抽象掉了加载不同文件格式的所有技术细节，只需要一行代码就能完成所有的工作:

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	
`importer.ReadFile`第二个参数是一些**后期处理(Post-processing)**的选项。除了加载文件之外，Assimp允许设定一些选项来强制它对导入的数据做一些额外的计算或操作。通过设定`aiPrcoess_Triangulate`，来告诉Assimp如果模型不是(全部)由三角形组成，它需要将模型所有的图元形状变换为三角形。`aiProcess_FlipUVs`将在处理的时候翻转y轴的纹理坐标。其它一些比较有用的选项如下:

* aiProcess_GenNormals:如果模型不包含法向量的话，就为每个顶点创建法线。
* aiProcess_SplitLargeMeshes:将比较大的网格分割成更小的子网格，如果渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
* aiProcess_OptimizeMeshes:和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化。

在Assimp加载完成后，将第一节点(根节点)传入递归的`processNode`函数。然后`processNode`递归处理所有节点。

	void processNode(aiNode *node, const aiScene *scene) {
		// 处理节点所有的网格
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene);
		}
		// 接下来对它的子节点重复这一过程
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

**从Assimp到网格**

将一个`aiMesh`对象转化为自己的网格对抗不是那么困难。要做的只是访问网格的相关属性并将它们储存到自己的对象中。`processMesh`函数的大体结构如下:

	Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
		
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			// 处理顶点位置、法线和纹理坐标
			...
			vertices.push_back(vertex);
		}
		// 处理索引
		...
		// 处理材质
		if (mesh->mMaterialIndex >= 0) {
			...
		}
		
		return Mesh(vertices, indices, textures);
	}

处理网格的过程主要有三部分:获取所有的顶点数据，获取它们的网格索引，并获取相关的材质数据。

>Assimp允许一个模型在一个顶点上有最多8个不同的纹理坐标。

**索引**

Assimp的接口定义了每个网格都有一个面(`Face`)数组，每个面代表了一个图元，在例子中它总是三角形。一个面包含了多个索引，它们定义了在每个图元中，应该绘制哪个顶点，并以什么顺序进行绘制，所以需要遍历所有的面，并储存了面的索引到`indices`这个vector中就可以了。

**材质**

和节点一样，一个网格只包含了一个指向材质对象的索引。如果想要获取网格真正的材质，还需要索引场景的`mMaterials`数组。网格材质索引位于它的`mMaterialIndex`属性中，同样可以用它来检测一个网格是否包含有材质。

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	
首先从场景的`mMaterials`数组中获取`aiMaterial`对象。接下来希望加载网格的漫反射或镜面光贴图。一个材质对象的内部对每种纹理类型都存储了一个纹理位置数组。不同的纹理类型都以`aiTextureType_`为前缀。

`loadMaterialTextures`函数遍历了给定纹理类型的所有纹理位置，获取了纹理的文件位置，并加载生成纹理，将信息储存在一个`Vertex`结构体中。其代码如下:

	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			Texture texture;
			texture.id = TextureFromFile(str.C_str(), directory);
			texture.type = typename;
			texture.path = str;
			textures.push_back(texture);
		}
		return textures;
	}

首先通过`GetTextureCount`函数检查储存在材质种纹理的数量，这个函数需要一个纹理类型。会使用`GetTexture`获取每个纹理的文件位置，它会将结果储存在一个`aiString`中。接下来是用另外一个叫做`TextureFromFile`的工具函数，它将会加载一个纹理并返回该纹理的ID。


**重大优化**

大多数场景会在多个网格中重用部分纹理，所以会对模型的代码进行调整，将所有加载过的纹理全局储存，每当想加载一个纹理的时候，首先去检查它有没有被加载过。如果有的话，会直接使用那个纹理，并跳过整个加载流程，来省下很多处理能力。


		
