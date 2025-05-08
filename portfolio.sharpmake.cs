using Sharpmake;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace portfolio
{
	public struct shaderDesc_t
	{
		[JsonPropertyName("name")] public string Name { get; set; }
		[JsonPropertyName("type")] public string Type { get; set; }
		[JsonPropertyName("path")] public string Path { get; set; }

		public shaderDesc_t()
		{
			Name = new string("");
			Type = new string("");
			Path = new string("");
		}
	}

	public struct shaderProgramDesc
	{
		[JsonPropertyName("name")] public string Name { get; set; }
		[JsonPropertyName("vertex attributes")] public List<string> Attributes { get; set; }
		[JsonPropertyName("outputs")] public List<string> Outputs { get; set; }
		[JsonPropertyName("shaders")] public List<shaderDesc_t> Shaders { get; set; }

		public shaderProgramDesc()
		{
			Name = new string("");
			Attributes = new List<string>();
			Outputs = new List<string>();
			Shaders = new List<shaderDesc_t>();
		}
	}
	
	/// <summary>
	/// base classes
	/// </summary>
	[Sharpmake.Generate]
	public class scene : Project
	{
		public string CSPath;
		public string commonIncludePath;
		public string projectIncludePath;
		public string resourcePath;
		public string tinywindowPath;
		public string tinyextenderPath;
		public string tinyshadersPath;
		public string tinyclockPath; 
		public string glmPath;
		public string gliPath;
		public string stbPath;
		public string imguiPath;
		public string abseilPath;
		public string cerealPath;
		public string eigenPath;
		public string evePath;
		public string fast_floatPath;
		public string fast_objPath;
		public string RemoteryPath;
		public string flatbuffersPath;
		public string yyJSONPath;

		public string scenePath;

		public scene()
		{
			Name = "Scene";
			Initialize();
		}

		[Configure()]
		public virtual void ConfigureAll(Configuration conf, Target target)
		{
			conf.ProjectFileName = "[project.Name]";
			conf.ProjectPath = CSPath + @"/examples/" + Name;// + @"/generated/";
			conf.TargetPath = CSPath + @"/examples/" + Name + @"/bin";
			conf.IncludePaths.Add(CSPath + @"/include/");
			conf.IncludePaths.Add(CSPath + @"/examples/" + Name + @"/include/");

			conf.IncludePaths.Add(tinywindowPath + @"Include");
			conf.IncludePaths.Add(tinywindowPath + @"Dependencies");
			conf.IncludePaths.Add(tinyextenderPath + @"Include");
			conf.IncludePaths.Add(tinyshadersPath + @"Include");
			conf.IncludePaths.Add(tinyclockPath + @"Include");
			conf.IncludePaths.Add(glmPath);
			conf.IncludePaths.Add(gliPath);
			conf.IncludePaths.Add(stbPath);
			conf.IncludePaths.Add(imguiPath);
			conf.IncludePaths.Add(abseilPath);
			conf.IncludePaths.Add(cerealPath);
			conf.IncludePaths.Add(eigenPath);
			conf.IncludePaths.Add(evePath);
			conf.IncludePaths.Add(fast_floatPath);
			conf.IncludePaths.Add(fast_objPath);
			conf.IncludePaths.Add(RemoteryPath);
			conf.IncludePaths.Add(yyJSONPath);
			conf.IncludePaths.Add(scenePath);

			if (Util.GetExecutingPlatform() == Platform.win64)
			{
				conf.Options.Add(Sharpmake.Options.Vc.General.WindowsTargetPlatformVersion.Latest);
				conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);
				conf.Options.Add(Sharpmake.Options.Vc.General.PlatformToolset.ClangCL);
			}

			else if (Util.GetExecutingPlatform() == Platform.linux)
			{
				conf.Options.Add(Options.Makefile.Compiler.CppLanguageStandard.Cpp2a);
				conf.Options.Add(Options.Makefile.General.PlatformToolset.Clang);
				
				conf.AdditionalLinkerOptions.Add(@"-lGL");
				conf.AdditionalLinkerOptions.Add(@"-lX11");
				conf.AdditionalLinkerOptions.Add(@"-lXinerama");
				conf.AdditionalLinkerOptions.Add(@"-lXrandr");
			}

			//conf.IsFastBuild = true;
		}

		public virtual void AddSources()
		{
			SourceRootPath = @"[project.SharpmakeCsPath]/examples/" + Name + @"/source/";

			string[] imGUiFiles = new string[] 
			{
				imguiPath + @"/imgui.cpp",
				imguiPath + @"/imgui_draw.cpp",
				imguiPath + @"/imgui_demo.cpp",
				imguiPath + @"/imgui_widgets.cpp",
				imguiPath + @"/imgui_tables.cpp",
				imguiPath + @"/imgui_widgets.cpp",
				imguiPath + @"/imGuizmo.cpp"
			};

			string[] yyJsonFiles = new string[]
			{
				yyJSONPath + @"/yyjson.c",
				yyJSONPath + @"/yyjson.h"
			};

			SourceFiles.AddRange(imGUiFiles);
			SourceFiles.AddRange(yyJsonFiles);
			SourceFiles.Add(CSPath + @"/include/Globals.h");
			SourceFiles.Add(scenePath + @"/Scene.h");
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
			LoadShaders(ref ResourceFiles);
		}

		public virtual void Initialize()
		{
			CSPath = @"[project.SharpmakeCsPath]";
			commonIncludePath = CSPath + @"/include/";
			projectIncludePath = CSPath + @"/examples/" + Name + @"/include/";
			resourcePath = CSPath + @"../../assets/";

			string assetsPath = CSPath + @"/lib/";

			tinywindowPath =		assetsPath + @"/tinywindow/";
			tinyextenderPath =		assetsPath + @"/tinyextender/";
			tinyshadersPath =		assetsPath + @"/tinyshaders/";
			tinyclockPath =			assetsPath + @"/tinyclock/";
			glmPath =				assetsPath + @"/glm/";
			gliPath =				assetsPath + @"/gli/";
			stbPath =				assetsPath + @"/stb/";
			imguiPath =				assetsPath + @"/imgui/";
			abseilPath =			assetsPath + @"/abseil-cpp/";
			cerealPath =			assetsPath + @"/cereal/";
			eigenPath =				assetsPath + @"/eigen/";
			evePath =				assetsPath + @"/eve/";
			fast_floatPath =		assetsPath + @"/fast_float/";
			fast_objPath =			assetsPath + @"/fast_obj/";
			RemoteryPath =			assetsPath + @"/Remotery/";
			yyJSONPath =			assetsPath + @"/yyjson/src/";

			scenePath = CSPath + @"/examples/Scene/include";

			if(Util.GetExecutingPlatform() == Platform.linux)
			{
				AddTargets(new Target(
					Platform.linux,
					DevEnv.make,
					Optimization.Debug | Optimization.Release));

			}

			else if(Util.GetExecutingPlatform() == Platform.win64)
			{
				AddTargets(new Target(
					Platform.win64,
					DevEnv.vs2022,
					Optimization.Debug | Optimization.Release));
			}

			AddSources();

			var excludedFolders = new List<string>();
			excludedFolders.Add("CMake.*");
			SourceFilesExcludeRegex.Add(@"/.*//(" + string.Join("|", excludedFolders.ToArray()) + @")//"); //yeah fuck cmake
		}

		public virtual string GenResourcePath()
		{
			return @"assets/shaders/" + "Default" + @".json";
		}

		public void LoadShaders(ref Strings resourceFiles)
		{
			//change this to use JSON instead
			
			
			string resourcePath = GenResourcePath();
			string jsonBuffer = File.ReadAllText(resourcePath);
			
			shaderProgramDesc[] shaderPrograms = JsonSerializer.Deserialize<shaderProgramDesc[]>(jsonBuffer);
			
			//Debug.Print(shaderPrograms.Length);
			Debug.Print("shader program name: " + shaderPrograms[0].Name);
			
			//ok now let's disseminate the data
			for (int iter = 0; iter < shaderPrograms.Length; iter++)
			{
				shaderProgramDesc program = shaderPrograms[iter];
				string programName = program.Name;
				int numVertexAttributes = program.Attributes.Count;
				int numOutputs = program.Outputs.Count;
				int numShaders = program.Shaders.Count;

				for (int shaderIter = 0; shaderIter < numShaders; shaderIter++)
				{
					shaderDesc_t shader = program.Shaders[shaderIter];
					string name = shader.Name;
					string type = shader.Type;
					string path = shader.Path;
					
					string fixedPath = CSPath + @"/resources/" + path.Substring(path.LastIndexOf(@"shaders/"));
					resourceFiles.Add(fixedPath);
				}
				
				resourceFiles.Add(resourcePath);
			}
			
			/*var fileStream = new FileStream(resourcePath, FileMode.Open, FileAccess.Read);
			var reader = new StreamReader(fileStream);
			int numPrograms = int.Parse(reader.ReadLine());
			resourceFiles.Add(resourcePath);

			for (int iter = 0; iter < numPrograms; iter++)
			{
				//we don't need most of this but it's nice to have one day
				string programName = reader.ReadLine();

				int numVertAttributes = int.Parse(reader.ReadLine());
				for (int attribIter = 0; attribIter < numVertAttributes; attribIter++)
				{
					string attrib = reader.ReadLine();
				}

				int numOutputs = int.Parse(reader.ReadLine());
				for (int attribIter = 0; attribIter < numOutputs; attribIter++)
				{
					string output = reader.ReadLine();
				}

				int numShaders = int.Parse(reader.ReadLine());
				for (int shaderIter = 0; shaderIter < numShaders; shaderIter++)
				{
					string name = reader.ReadLine();
					string type = reader.ReadLine();
					string path = reader.ReadLine();

					//take path and rip out everything before shaders/
					string fixedPath = CSPath + @"/resources/" + path.Substring(path.LastIndexOf(@"shaders/"));
					resourceFiles.Add(fixedPath);
					//Console.WriteLine(fixedPath);
				}
				resourceFiles.Add(resourcePath);
				string skipline = reader.ReadLine();
			}*/
		}

		public virtual void GrabHeaders(ref Strings SourceFiles, string folderPath)
		{
			DirectoryInfo d = new DirectoryInfo(folderPath);
			string fixedPath = CSPath + @"/" + folderPath.Substring(folderPath.LastIndexOf(@"../") + 3); //an incredibly ugly hack
			foreach (var file in d.GetFiles("*.h"))
			{
				SourceFiles.Add(fixedPath + file.Name);
			}
		}
	}

	#region scenes
	[Sharpmake.Generate]
	public class textured : scene
	{
		public string texturedScenePath;

		public textured()
		{
			Name = "Textured";
			Initialize();
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			conf.IncludePaths.Add(texturedScenePath);
		}

		public override void AddSources()
		{
			base.AddSources();
			texturedScenePath = CSPath + @"/examples/Textured/include";
			SourceFiles.Add(texturedScenePath + @"/TexturedScene.h");
		}

		public override string GenResourcePath()
		{
			Name = "Textured";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class scene3D : scene
	{
		public string ufbxPath;
		public string scene3DPath;
		public scene3D()
		{
			Name = "3DScene";
			Initialize();
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			
			conf.IncludePaths.Add(ufbxPath);
			conf.IncludePaths.Add(CSPath + @"/examples/" + Name + @"/include/");
			conf.IncludePaths.Add(scene3DPath);// + "Scene3D.h");
		}

		public override void AddSources()
		{
			//Name = "3DScene";
			ufbxPath = CSPath + @"/dependencies/ufbx";
			base.AddSources();
			
			scene3DPath = CSPath + @"/examples/3DScene/include";

			string[] ubfxFiles = new string[] {
				ufbxPath + @"/ufbx.c"
			};

			SourceFiles.AddRange(ubfxFiles);
			SourceFiles.Add(scene3DPath + @"/Scene3D.h");
			SourceFiles.Add(CSPath + @"/include/Model.h");
			//maybe find everything in that folder and add to the string
		}

		public override string GenResourcePath()
		{
			return @"assets/shaders/anim/" + @"AnimTest" + @".txt";
		}
	}

	/// <summary>
	/// abstract classes for convenience
	/// </summary>
	public abstract class texAbstract : textured
	{
		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
		}
	}

	public abstract class sceneAbstract : scene
	{
		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
		}
	}

	public abstract class fb2DAbstract : texAbstract
	{
		public override void AddSources()
		{
			base.AddSources();
			//maybe find everything in that folder and add to the string
			SourceFiles.Add(CSPath + @"/include/Framebuffer.h");
		}
	}

	public abstract class scene3DAbstract : scene3D
	{
		public override void AddSources()
		{
			base.AddSources();
			//maybe find everything in that folder and add to the string
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
		}
	}

	public abstract class texScene3DAbstract : scene3DAbstract
	{
		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/Texture.h");
		}
	}

	public abstract class fb3DAbstract : texScene3DAbstract
	{
		public override void AddSources()
		{
			base.AddSources();
			//maybe find everything in that folder and add to the string
			SourceFiles.Add(CSPath + @"/include/Framebuffer.h");
		}
	}

	/// <summary>
	/// 2D
	/// </summary>
	[Sharpmake.Generate]
	public class computeTest : scene
	{
		public computeTest()
		{
			Name = "ComputeTest";
			Initialize();
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
		}

		public override string GenResourcePath()
		{
			Name = "ComputeTest";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class bindless : texAbstract
	{
		public bindless()
		{
			Name = "Bindless";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Bindless";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class bubble : texAbstract
	{
		public bubble()
		{
			Name = "Bubble";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Bubble";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class cellShading : texAbstract
	{
		public cellShading()
		{
			Name = "CellShading";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "CellShading";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class chromaticAberration : texAbstract
	{
		public chromaticAberration()
		{
			Name = "ChromaticAberration";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "ChromaticAberration";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class contrast : texAbstract
	{
		public contrast()
		{
			Name = "Contrast";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Contrast";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class dilation : texAbstract
	{
		public dilation()
		{
			Name = "Dilation";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Dilation";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class edgeDetection : texAbstract
	{
		public edgeDetection()
		{
			Name = "EdgeDetection";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "EdgeDetection";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class erosion : texAbstract
	{
		public erosion()
		{
			Name = "Erosion";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Erosion";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class heatHaze : fb2DAbstract
	{
		public heatHaze()
		{
			Name = "HeatHaze";
			Initialize();
		}

		public override void AddSources()
		{
			base.AddSources();
			//maybe find everything in that folder and add to the string
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/Bubble.h");
		}

		public override string GenResourcePath()
		{
			Name = "HeatHaze";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class frost : heatHaze
	{
		public frost()
		{
			Name = "Frost";
			Initialize();
		}

		public override void AddSources()
		{
			base.AddSources();
			//maybe find everything in that folder and add to the string
			SourceFiles.Add(CSPath + @"/examples/HeatHaze/include/Bubble.h");
			SourceFiles.Add(CSPath + @"/examples/HeatHaze/include/HeatHaze.h");
		}

		public override string GenResourcePath()
		{
			Name = "Frost";
			return @"assets/shaders/HeatHaze.txt";
		}
	}

	[Sharpmake.Generate]
	public class depthPrePass : fb3DAbstract
	{
		public depthPrePass()
		{
			Name = "DepthPrePass";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "DepthPrePass";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class displacement : scene3DAbstract
	{
		public displacement()
		{
			Name = "Displacement";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Displacement";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class dynamicResolution : fb3DAbstract
	{
		public dynamicResolution()
		{
			Name = "DynamicRes";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "DynamicRes";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class dynamicResolution2 : dynamicResolution
	{
		public dynamicResolution2()
		{
			Name = "DynamicRes2";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "DynamicRes2";
			return @"assets/shaders/TexturedModel.txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/DynamicRes/include/DynamicRes.h");
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			conf.IncludePaths.Add(CSPath + @"/examples/DynamicRes/include/");

		}
	}

	[Sharpmake.Generate]
	public class framebuffers : fb3DAbstract
	{
		public framebuffers()
		{
			Name = "Framebuffers";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Framebuffers";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class FXAA : fb3DAbstract
	{
		public FXAA()
		{
			Name = "FXAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "FXAA";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class gameOfLife : sceneAbstract
	{
		public gameOfLife()
		{
			Name = "GameOfLife";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "GameOfLife";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class gamma : texAbstract
	{
		public gamma()
		{
			Name = "Gamma";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Gamma";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class gaussian : texAbstract
	{
		public gaussian()
		{
			Name = "Gaussian";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Gaussian";
			return @"assets/shaders/" + Name + @".txt";
		}
	}
	
	[Sharpmake.Generate]
	public class gaussianMulti : fb2DAbstract
	{
		public gaussianMulti()
		{
			Name = "GaussianMulti";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "GaussianMulti";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class godRay : fb3DAbstract
	{
		public godRay()
		{
			Name = "GodRay";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "GodRay";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class golCompute : gameOfLife
	{
		public golCompute()
		{
			Name = "GOLCompute";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "GOLCompute";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/GameOfLife/include/GameOfLife.h");
		}
	}

	[Sharpmake.Generate]
	public class heightFog : displacement
	{
		public heightFog()
		{
			Name = "HeightFog";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "HeightFog";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/Displacement/include/Displacement.h");
		}
	}

	[Sharpmake.Generate]
	public class textureSettings : texAbstract
	{
		public textureSettings()
		{
			Name = "TextureSettings";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TextureSettings";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class mipMapping : textureSettings
	{
		public mipMapping()
		{
			Name = "MipMapping";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "MipMapping";
			return @"assets/shaders/" + Name + @".txt";
		}
		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/TextureSettings/include/TextureSettings.h");
		}
	}

	[Sharpmake.Generate]
	public class MSAA : fb3DAbstract
	{
		public MSAA()
		{
			Name = "MSAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "MSAA";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class stencil : fb3DAbstract
	{
		public stencil()
		{
			Name = "Stencil";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Stencil";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class outline : stencil
	{
		public outline()
		{
			Name = "Outline";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Outline";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/Stencil/include/Stencil.h");
		}
	}

	[Sharpmake.Generate]
	public class parallax : texAbstract
	{
		public parallax()
		{
			Name = "Parallax";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Parallax";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class perlinNoise : sceneAbstract
	{
		public perlinNoise()
		{
			Name = "PerlinNoise";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "PerlinNoise";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class perlin3D : perlinNoise
	{
		public perlin3D()
		{
			Name = "Perlin3D";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Perlin3D";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/PerlinNoise/include/PerlinNoise.h");
		}
	}

	[Sharpmake.Generate]
	public class picking : outline
	{
		public picking()
		{
			Name = "Picking";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Picking";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/Outline/include/Outline.h");
			SourceFiles.Add(CSPath + @"/include/Ray.h");
		}
	}

	[Sharpmake.Generate]
	public class pixelize : texAbstract
	{
		public pixelize()
		{
			Name = "Pixelize";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Pixelize";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class radialBlur : texAbstract
	{
		public radialBlur()
		{
			Name = "RadialBlur";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "RadialBlur";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class sepia : texAbstract
	{
		public sepia()
		{
			Name = "Sepia";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Sepia";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class sharpen : texAbstract
	{
		public sharpen()
		{
			Name = "Sharpen";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Sharpen";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class SMAA : fb3DAbstract
	{
		public SMAA()
		{
			Name = "SMAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "SMAA";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class soundTest : sceneAbstract
	{
		string soloudPath;
		public soundTest()
		{
			Name = "SoundTest";
			Initialize();
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			
			conf.IncludePaths.Add(soloudPath + @"include/");

			if (target.Optimization == Optimization.Debug)
			{
				conf.LibraryPaths.Add(soloudPath + @"/lib/");
				conf.LibraryFiles.Add(@"soloud_static");
			}
			else if (target.Optimization == Optimization.Release)
			{
				conf.LibraryPaths.Add(soloudPath + @"/lib/");
				conf.LibraryFiles.Add(@"soloud");
			}
		}

		public override void AddSources()
		{
			Name = "SoundTest";
			base.AddSources();
			soloudPath = CSPath + @"/lib/soloud/";
			string filepath = @"lib/soloud/include/";

			GrabHeaders(ref SourceFiles, filepath);

			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/Compressor.h");
		}
	}

	[Sharpmake.Generate]
	public class speedTree : scene3DAbstract
	{
		public speedTree()
		{
			Name = "SpeedTree";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "SpeedTree";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class SSAA : fb3DAbstract
	{
		public SSAA()
		{
			Name = "SSAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "SSAA";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/HaltonSequence.h");
		}
	}

	[Sharpmake.Generate]
	public class temporalAA : fb3DAbstract
	{
		public temporalAA()
		{
			Name = "TemporalAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TemporalAA";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/HaltonSequence.h");
		}
	}

	[Sharpmake.Generate]
	public class temporalUpsampling : temporalAA
	{
		public temporalUpsampling()
		{
			Name = "TemporalUpsampling";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TemporalUpsampling";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
			SourceFiles.Add(CSPath + @"/examples/TemporalAA/include/TemporalAA.h");
		}
	}

	[Sharpmake.Generate]
	public class texturedModel : texScene3DAbstract
	{
		public texturedModel()
		{
			Name = "TexturedModel";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TexturedModel";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class textureSampling : texAbstract
	{
		public textureSampling()
		{
			Name = "TextureSampling";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TextureSampling";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class TXAA : fb3DAbstract
	{
		public TXAA()
		{
			Name = "TXAA";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "TXAA";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/HaltonSequence.h");
		}
	}

	[Sharpmake.Generate]
	public class profiler : temporalAA
	{
		public string microProfilerPath;
		public profiler()
		{
			Name = "Profiler";
			Initialize();
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);

			conf.IncludePaths.Add(microProfilerPath);
		}

		public override void AddSources()
		{
			Name = "Profiler";
			base.AddSources();
			microProfilerPath = CSPath + @"/lib/microprofile/";
			GrabHeaders(ref SourceFiles, @"lib/microprofile/");


			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/" + Name + @".h");
			SourceFiles.Add(CSPath + @"/examples/TemporalAA/include/TemporalAA.h");
		}

		public override string GenResourcePath()
		{
			Name = "Profiler";
			return @"assets/shaders/TemporalAA.txt";
		}
	}

	[Sharpmake.Generate]
	public class SMAA1xt : SMAA
	{
		public SMAA1xt()
		{
			Name = "SMAA 1xt";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "SMAA 1xt";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/SMAA.h");
		}
	}

	[Sharpmake.Generate]
	public class SMAA2xt : SMAA1xt
	{
		public SMAA2xt()
		{
			Name = "SMAA 2xt";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "SMAA 2xt";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/SMAA.h");
			SourceFiles.Add(CSPath + @"/examples/" + Name + @"/include/SMAA 1xt.h");
		}
	}

	[Sharpmake.Generate]
	public class WDivide : scene3DAbstract
	{
		public WDivide()
		{
			Name = "WDivide";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "WDivide";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class Transforms : scene3DAbstract
	{
		public Transforms()
		{
			Name = "Transforms";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Transforms";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/Transform.h");
		}
	}

	[Sharpmake.Generate]
	public class CheapBlur : texAbstract
	{
		public CheapBlur()
		{
			Name = "CheapBlur";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "CheapBlur";
			return @"assets/shaders/" + Name + @".txt";
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/CheapBlur.h");
		}
	
	}

	[Sharpmake.Generate]
	public class Slipgate : fb3DAbstract
	{
		public Slipgate()
		{
			Name = "Slipgate";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "Slipgate";
			return @"assets/shaders/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class dotProduct : sceneAbstract
	{
		public dotProduct()
		{
			Name = "DotProduct";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "DotProduct";
			return @"assets/shaders/" + Name + @"/" + Name +  @".txt";
		}
	}

	[Sharpmake.Generate]
	public class planeRayCollision : scene3DAbstract
	{
		public planeRayCollision()
		{
			Name = "planeRayCollision";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "planeRayCollision";
			return @"assets/shaders/" + Name + @"/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class BSPLoader : scene3DAbstract
	{
		public BSPLoader()
		{
			Name = "BSPLoader";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "BSPLoader";
			return @"assets/shaders/" + Name + @"/" + Name + @".txt";
		}
	}

	[Sharpmake.Generate]
	public class OAUpsampler : dynamicResolution
	{
		public OAUpsampler()
		{
			Name = "OAUpsampler";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "OAUpsampler";
			return @"assets/shaders/" + Name + @"/" + Name + @".txt";
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			conf.IncludePaths.Add(CSPath + @"/examples/DynamicRes/include");
		}
	}

	[Sharpmake.Generate]
	public class CommandBuffers : textured
	{
		public CommandBuffers()
		{
			Name = "CommandBuffers";
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = "CommandBuffers";
			return @"assets/shaders/" + Name + @"/" + Name + @".txt";
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			conf.IncludePaths.Add(CSPath + @"/examples/CommandBuffers/include");
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/CommandBuffer.h");
		}
	}

	[Sharpmake.Generate]
	public class FeedbackBuffers : texturedModel
	{
		public FeedbackBuffers()
		{
			Name = GetType().Name;
			Initialize();
		}

		public override string GenResourcePath()
		{
			Name = GetType().Name;
			return  @"./examples/" + Name + @"/shaders/" + Name + @".txt";
		}

		public override void ConfigureAll(Configuration conf, Target target)
		{
			base.ConfigureAll(conf, target);
			conf.IncludePaths.Add(CSPath + @"/include/" + Name + @".h");
		}

		public override void AddSources()
		{
			base.AddSources();
			SourceFiles.Add(CSPath + @"/include/FeedbackBuffer.h");
		}
	}
#endregion
	[Sharpmake.Generate]
	public class PortfolioSolution : Sharpmake.Solution
	{
		public PortfolioSolution()
		{
			Name = "Portfolio";

			if(Util.GetExecutingPlatform() == Platform.linux)
			{
				AddTargets(new Target(
					Platform.linux,
					DevEnv.make,
					Optimization.Debug | Optimization.Release
			));

			}

			else if(Util.GetExecutingPlatform() == Platform.win64)
			{
				AddTargets(new Target(
					Platform.win64,
					DevEnv.vs2022,
					Optimization.Debug | Optimization.Release));
			}
		}

		[Configure()]
		public void ConfigureAll(Configuration conf, Target target)
		{
			if(Util.GetExecutingPlatform() == Platform.linux)
			{
				conf.SolutionFileName = "[solution.Name]_[target.Platform]";
			}
			else
			{
				conf.SolutionFileName = "[solution.Name]_[target.Platform]";
			}
			//conf.SolutionFileName = "[solution.Name]_[target.Platform]";
			conf.SolutionPath = @"[solution.SharpmakeCsPath]";

			conf.AddProject<scene>(target);
			/*conf.AddProject<scene3D>(target);
			conf.AddProject<textured>(target);
			conf.AddProject<bindless>(target);
			conf.AddProject<bubble>(target);
			conf.AddProject<cellShading>(target);
			conf.AddProject<chromaticAberration>(target);
			conf.AddProject<computeTest>(target);
			conf.AddProject<contrast>(target);
			conf.AddProject<depthPrePass>(target);
			conf.AddProject<dilation>(target);
			conf.AddProject<displacement>(target);
			conf.AddProject<dynamicResolution>(target);
			conf.AddProject<edgeDetection>(target);
			conf.AddProject<erosion>(target);
			conf.AddProject<framebuffers>(target);
			conf.AddProject<heatHaze>(target);
			conf.AddProject<frost>(target);
			conf.AddProject<FXAA>(target);
			conf.AddProject<gameOfLife>(target);
			conf.AddProject<gamma>(target);
			conf.AddProject<gaussian>(target);
			conf.AddProject<gaussianMulti>(target);
			conf.AddProject<godRay>(target);
			conf.AddProject<golCompute>(target);
			conf.AddProject<heightFog>(target);
			conf.AddProject<textureSettings>(target);
			conf.AddProject<mipMapping>(target);
			conf.AddProject<MSAA>(target);
			conf.AddProject<stencil>(target);
			conf.AddProject<outline>(target);
			conf.AddProject<parallax>(target);
			conf.AddProject<perlinNoise>(target);
			conf.AddProject<perlin3D>(target);
			conf.AddProject<picking>(target);
			conf.AddProject<pixelize>(target);
			conf.AddProject<radialBlur>(target);
			conf.AddProject<sepia>(target);
			conf.AddProject<sharpen>(target);
			conf.AddProject<SMAA>(target);
			conf.AddProject<soundTest>(target);
			conf.AddProject<speedTree>(target);
			conf.AddProject<SSAA>(target);
			conf.AddProject<temporalAA>(target);
			conf.AddProject<temporalUpsampling>(target);
			conf.AddProject<texturedModel>(target);
			conf.AddProject<textureSampling>(target);
			conf.AddProject<TXAA>(target);
			conf.AddProject<profiler>(target);
			conf.AddProject<SMAA1xt>(target);
			conf.AddProject<SMAA2xt>(target);
			conf.AddProject<WDivide>(target);
			conf.AddProject<dynamicResolution2>(target);
			conf.AddProject<Transforms>(target);
			conf.AddProject<CheapBlur>(target);
			conf.AddProject<Slipgate>(target);
			conf.AddProject<dotProduct>(target);
			conf.AddProject<BSPLoader>(target);
			conf.AddProject<OAUpsampler>(target);
			conf.AddProject<CommandBuffers>(target);
			conf.AddProject<FeedbackBuffers>(target);*/
			//conf.AddProject<planeRayCollision>(target);
		}
 
		[Sharpmake.Main]
		public static void SharpmakeMain(Sharpmake.Arguments arguments)
		{
			arguments.Generate<PortfolioSolution>();
		}
	}
}