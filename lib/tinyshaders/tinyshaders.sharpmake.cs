using Sharpmake;
using System.Collections.Generic;

namespace tinyshaders
{
    [Sharpmake.Generate]
    public class ExampleProject : Project
    {
        public ExampleProject()
        {
            Name = "Example";

            AddTargets(new Target(
                Platform.win64,
                DevEnv.vs2019, 
                Optimization.Debug | Optimization.Release
            ));

            SourceRootPath = @"[project.SharpmakeCsPath]\Example\";

            var excludedFolders = new List<string>();
            excludedFolders.Add("CMake.*");
            SourceFilesExcludeRegex.Add(@"\.*\\(" + string.Join("|", excludedFolders.ToArray()) + @")\\");
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.ProjectFileName = "[project.Name]";
            conf.ProjectPath = @"[project.SharpmakeCsPath]\Example\";
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\Include\");
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\Example\Include\");
            conf.IncludePaths.Add(@"[project.SharpmakeCsPath]\Example\Dependencies\");
            conf.TargetPath = @"[project.SharpmakeCsPath]\bin";
            conf.Options.Add(Sharpmake.Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        }
    }

    [Sharpmake.Generate]
    public class TinyShadersSolution : Sharpmake.Solution
    {
        public TinyShadersSolution()
        {
            Name = "TinyShaders";

            AddTargets(new Target(
                Platform.win64,
                DevEnv.vs2019,
                Optimization.Debug | Optimization.Release
            ));
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.SolutionFileName = "[solution.Name]";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]";
            conf.AddProject<ExampleProject>(target);
        }
 
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            arguments.Generate<TinyShadersSolution>();
        }
    }
}