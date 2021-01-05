#pragma once

#include <std_include.h>
#include <GpuResource.hpp>

/// <summary>
/// Functor for the allocation and deallocation for a shader
/// </summary>
template <GLenum ShaderType>
struct ShaderAllocator
{
public:
	/// <returns>New buffer id </returns>
	GLuint CreateResource()
	{
		return glCreateShader(ShaderType);
	}

	void DestroyResource(GLuint shader)
	{
		glDeleteShader(shader);
	}
};

class SharedShader : public GpuResource<GLuint, ShaderAllocator<GL_FRAGMENT_SHADER>>
{

private:
	//////////////////////////////////////////

	// Check compilation and linking errors
	static void checkCompileErrors(GLuint shader, string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << endl;
			}
		}
	}

public:
	SharedShader(const std::string& file)
	{
		// Step 1: we retrieve shaders source code from provided filepaths
		string mappingCode;
		ifstream mappingFile;

		// ensure ifstream objects can throw exceptions:
		mappingFile.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			// Open files
			mappingFile.open(file);
			stringstream mappingStream;
			// Read file's buffer contents into streams
			mappingStream << mappingFile.rdbuf();
			// close file handlers
			mappingFile.close();
			// Convert stream into string
			mappingCode = mappingStream.str();
		}
		catch (ifstream::failure e)
		{
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
		}

		// Convert strings to char pointers
		const GLchar* fMappingCode = mappingCode.c_str();


		// Mapping Shader (shared code)
		glShaderSource(Resource(), 1, &fMappingCode, NULL);
		glCompileShader(Resource());
		// check compilation errors
		checkCompileErrors(Resource(), "SHARED_FRAGMENT");
	}
};