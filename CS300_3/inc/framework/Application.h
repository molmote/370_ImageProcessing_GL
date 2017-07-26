#ifndef H_APPLICATION
#define H_APPLICATION

// An application represents a complete state needed to use ImGui or render
// using OpenGL. Due to limitations with FreeGLUT, only one Application
// instance may exist per process.
class Application
{
public:
  typedef void(*CallbackFunction)(Application *application, void *userData);

  Application(std::string const &title, unsigned width, unsigned height);
  inline std::string const &GetTitle() const { return title_; }
  inline unsigned GetWindowWidth() const { return windowWidth_; }
  inline unsigned GetWindowHeight() const { return windowHeight_; }

  // Creates a new GL 3.3 context using FreeGLUT and GLEW. Creates and opens
  // the window for the application. Verifies the context has been successfully
  // created.
  void Initialize(int argc, char *argv[]);

  // Initializes callbacks for input for GLUT, initializes ImGui, and begins
  // the main process loop. The initCallback is called exactly once before the
  // update loop begins. The deinitCallback is called exactly once after the
  // update loop is no longer executing (such as when the user clicks the exit
  // button). The update loop is continuously called as long as the application
  // is not running. You can use 'Application::close' to exit the application
  // at any time. It properly handles calling the deinitCallback in that case,
  // so you may clean up OpenGL objects and any other memory used. This
  // function blocks until Close is called.
  void Run(CallbackFunction initCallback, CallbackFunction updateCallback,
    CallbackFunction cleanupCallback, void *initUserData = NULL,
    void *updateUserData = NULL, void *cleanupUserData = NULL);

  // Safely closes the application, leading to the cleanupCallback passed to
  // Update to be called and, eventually, thread control to be restored to the
  // caller of Run.
  void Close();

private:
  struct ApplicationImpl;
  friend struct ApplicationImpl;

  std::string title_;
  unsigned windowWidth_, windowHeight_, windowHandle_;
  CallbackFunction updateCallback_, cleanupCallback_;
  void *updateCallbackData_, *cleanupCallbackData_;
};

#endif
