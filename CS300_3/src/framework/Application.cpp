#include "Precompiled.h"
#include "framework/Application.h"
#include "framework/Debug.h"
#include "framework/ImGuiImpl.h"

// declarations of helpers
namespace
{
  static Application *instance = NULL;
}

// These callbacks are wrapped in this struct so they may have private scope
// access to an instance of Application.
struct Application::ApplicationImpl
{
  static void OnInitialize();
  static void OnCleanup();
  static void OnIdle();
  static void OnDraw();
  static void OnWindowResize(int width, int height);
  static void OnKey(unsigned char key, int x, int y);
  static void OnMouse(int button, int state, int x, int y);
  static void OnMouseDrag(int x, int y);
};

Application::Application(std::string const &title, unsigned width,
  unsigned height)
  : title_(title), windowWidth_(width), windowHeight_(height), windowHandle_(0),
  updateCallback_(NULL), cleanupCallback_(NULL), updateCallbackData_(NULL),
  cleanupCallbackData_(NULL)
{
}

void Application::Initialize(int argc, char *argv[])
{
  Assert(!instance, "Error: cannot have more than one Application instance.");
  instance = this;

  // initialize GLUT & OpenGL 3.3-specific stuff
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
  // glutInitContextVersion(3, 3); // this doesn't work with my intel driver
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
  glutInitWindowSize(windowWidth_, windowHeight_);

  // safely close GLUT: http://stackoverflow.com/questions/5033832
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

  // center window
  int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
  int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
  glutInitWindowPosition((screenWidth / 2) - (windowWidth_ / 2),
    (screenHeight / 2) - (windowHeight_ / 2));

  // open the window
  windowHandle_ = glutCreateWindow(title_.c_str());

  // link OpenGL using GLEW
  glewExperimental = GL_TRUE;
  CheckGlew(glewInit());
  Assert(GLEW_VERSION_3_3, "OpenGL 3.3 not supported.");
}

void Application::Run(CallbackFunction initCallback,
  CallbackFunction updateCallback, CallbackFunction cleanupCallback,
  void *initUserData/* = NULL*/, void *updateUserData/* = NULL*/,
  void *cleanupUserData/* = NULL*/)
{
  // initialize GLUT callbacks; each of these functions will be called, in turn,
  // based on OS events fired for each action; see for more information:
  //   http://freeglut.sourceforge.net/docs/api.php#WindowCallback
  glutDisplayFunc(ApplicationImpl::OnDraw);
  glutReshapeFunc(ApplicationImpl::OnWindowResize);
  glutEntryFunc(ImGuiImpl::Entry);
  glutSpecialFunc(ImGuiImpl::SpecialDown);
  glutSpecialUpFunc(ImGuiImpl::SpecialUp);
  glutKeyboardFunc(ApplicationImpl::OnKey);
  glutKeyboardUpFunc(ImGuiImpl::KeyboardUp);
  glutMouseFunc(ApplicationImpl::OnMouse);
  glutMouseWheelFunc(ImGuiImpl::MouseWheel);
  glutMotionFunc(ApplicationImpl::OnMouseDrag);
  glutPassiveMotionFunc(ImGuiImpl::PassiveMotion);
  glutIdleFunc(ApplicationImpl::OnIdle);

  // initialize ImGui (see ImGuiImpl.cpp for more)
  ImGuiImpl::Initialize(windowWidth_, windowHeight_);

  // initialize application
  updateCallback_ = updateCallback;
  updateCallbackData_ = updateUserData;
  cleanupCallback_ = cleanupCallback;
  cleanupCallbackData_ = cleanupUserData;
  ApplicationImpl::OnInitialize();
  if (initCallback)
    initCallback(this, initUserData);

  // begin executing using GLUT
  glutMainLoop();

  // finished; return execution to the caller
}

void Application::Close()
{
  // call cleanup callback before terminating
  if (cleanupCallback_)
    cleanupCallback_(this, cleanupCallbackData_);

  // cleanup the application and ImGui
  ApplicationImpl::OnCleanup();
  ImGuiImpl::Cleanup();

  // safely close GLUT: http://stackoverflow.com/questions/5033832
  glutLeaveMainLoop();
}

// implementation of helpers

void Application::ApplicationImpl::OnInitialize()
{
  std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << std::endl << std::endl;
}

void Application::ApplicationImpl::OnCleanup()
{
}

void Application::ApplicationImpl::OnDraw()
{
  // prepare application to be able to create ImGui windows
  ImGui::NewFrame();

  // allow client code to draw stuff with OpenGL
  instance->updateCallback_(instance, instance->updateCallbackData_);

  // ensure no OpenGL errors have occurred during this frame
#ifdef _DEBUG
  //CheckGL();
#endif

  // draw ImGui
  ImGuiImpl::Render();

  // finish rendering; sync with graphics card and allow it to catch up
  glutSwapBuffers();
}

void Application::ApplicationImpl::OnWindowResize(int width, int height)
{
  // ensure OpenGL viewport matches the window size (NDC->screen conversion)
  glViewport(0, 0, width, height);

  // ensure ImGui is aware of the new window dimensions
  ImGuiImpl::Reshape(width, height);

  // update the dimensions as far as the application knows; this allows the
  // projection matrix being used to project to the correct dimensions
  instance->windowWidth_ = width;
  instance->windowHeight_ = height;
}

void Application::ApplicationImpl::OnKey(unsigned char key, int x, int y)
{
  if (key == 27) // escape key
    instance->Close();
  ImGuiImpl::KeyboardDown(key, x, y);
}

void Application::ApplicationImpl::OnMouse(int button, int state, int x, int y)
{
  ImGuiImpl::Mouse(button, state, x, y);
}

void Application::ApplicationImpl::OnMouseDrag(int x, int y)
{
  ImGuiImpl::Motion(x, y);
}

void Application::ApplicationImpl::OnIdle()
{
  // continue to display as fast as possible
  glutPostRedisplay();
}
