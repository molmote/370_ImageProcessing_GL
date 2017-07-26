#ifndef H_IMGUI_IMPL
#define H_IMGUI_IMPL

// This class is responsible for managing the actual ImGui instance, including
// initialization, drawing it, cleaning up resources, and handling input
// interactions with FreeGLUT. You shouldn't have to change anything in this
// file or in ImGuiImpl.cpp. Defining new ImGui windows is completely
// independent of any functionality implemented here. This implementation is
// specific to OpenGL 3.3 using FreeGLUT for I/O. This code can easily be
// adapted to other versions of OpenGL or other input libraries.
class ImGuiImpl
{
public:
  struct State;

  static void Initialize(unsigned windowWidth, unsigned windowHeight);
  static void Render();
  static void Cleanup();

  // interface with FreeGLUT; each of these represents a GLUT callback with the
  // prefix 'glut' and suffix 'Func', unless otherwise noted. E.g., Reshape
  // corresponds to glutReshapeFunc.
  static void Reshape(int w, int h);
  static void Entry(int state);
  static void SpecialDown(int key, int x, int y); // glutSpecialFunc
  static void SpecialUp(int key, int x, int y); // glutSpecialUpFunc
  static void KeyboardDown(unsigned char key, int x, int y); // glutKeyboardFunc
  static void KeyboardUp(unsigned char key, int x, int y); // glutKeyboardUpFunc
  static void Mouse(int button, int state, int x, int y);
  static void MouseWheel(int button, int state, int x, int y);
  static void Motion(int x, int y);
  static void PassiveMotion(int x, int y);

private:
  static void SpecialUpDown(int key, int x, int y, bool down);
  static void KeyboardUpDown(unsigned char key, int x, int y, bool down);
};

#endif
