
local assignmentTitle = "Assignment2"

if _ACTION == "clean" then
  os.rmdir("../bin")
  os.rmdir("./projects")
  os.rmdir("./ipch")
  os.remove("CS300.sln")
  os.remove("CS300.sdf")
  os.remove("CS300.suo")
  os.remove("CS300.v11.suo")
  os.remove("CS300.v12.suo")
  os.exit()
end

solution "CS300"
  configurations { "Debug", "ReleaseSymbols", "Release" }
  project(assignmentTitle)
    targetname(assignmentTitle:lower())
    kind "ConsoleApp"
    language "C++"
    location "projects"
    pchsource "../src/Precompiled.cpp"
    pchheader "Precompiled.h"
    includedirs { "../inc", "../dep" }
    libdirs { "../dep/FreeGLUT", "../dep/GLEW", "../dep/ImGui", "../dep/STB" }
    links { "freeglut", "glew32" }
    files { "../inc/**.h", "../src/**.cpp" }
    configuration "Debug"
      targetdir "../bin/debug"
      defines { "_DEBUG", "ASSET_PATH=\"../../assets/\"" }
      flags { "Symbols" }
      links { "ImGuid" } -- debug version of ImGui
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\debug\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\debug\\" }
    configuration "ReleaseSymbols"
      targetdir "../bin/release"
      defines { "NDEBUG", "ASSET_PATH=\"../../assets/\"" }
      flags { "Optimize", "Symbols" }
      links { "ImGui" } -- release version of ImGui
      linkoptions { "/LTCG" } -- allows whole-program optimization
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\release\\" }
    configuration "Release"
      targetdir "../bin/release"
      defines { "NDEBUG", "ASSET_PATH=\"../../assets/\"" }
      flags { "Optimize" }
      links { "ImGui" } -- release version of ImGui
      linkoptions { "/LTCG" } -- allows whole-program optimization
      postbuildcommands {
        "copy ..\\..\\dep\\GLEW\\glew32.dll ..\\..\\bin\\release\\",
        "copy ..\\..\\dep\\FreeGLUT\\freeglut.dll ..\\..\\bin\\release\\" }
