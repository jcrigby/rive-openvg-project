#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

// Include Rive headers first to avoid X11 name conflicts
#include "rive/file.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/factory.hpp"
#include "utils/no_op_factory.hpp"

// OpenGL and X11 headers (after Rive to avoid None conflict)
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// Undefine X11 macros that conflict with Rive
#ifdef None
#undef None
#endif

// Simple OpenGL renderer for Rive
class SimpleOpenGLRenderer : public rive::Renderer {
private:
    int windowWidth;
    int windowHeight;
    
public:
    SimpleOpenGLRenderer(int width, int height) : windowWidth(width), windowHeight(height) {}
    
    void save() override {
        glPushMatrix();
    }
    
    void restore() override {
        glPopMatrix();
    }
    
    void transform(const rive::Mat2D& transform) override {
        GLfloat matrix[16] = {
            transform[0], transform[1], 0, 0,
            transform[2], transform[3], 0, 0,
            0, 0, 1, 0,
            transform[4], transform[5], 0, 1
        };
        glMultMatrixf(matrix);
    }
    
    void drawPath(rive::RenderPath* path, rive::RenderPaint* paint) override {
        // Draw a visible animated shape
        static float time = 0.0f;
        time += 0.02f;
        
        // Animate color
        float r = 0.5f + 0.5f * sinf(time);
        float g = 0.3f + 0.3f * sinf(time * 1.3f);
        float b = 0.1f + 0.4f * sinf(time * 0.7f);
        
        glColor3f(r, g, b);
        
        // Draw animated rectangle
        float size = 30.0f + 10.0f * sinf(time * 2.0f);
        glBegin(GL_QUADS);
        glVertex2f(-size, -size);
        glVertex2f(size, -size);
        glVertex2f(size, size);
        glVertex2f(-size, size);
        glEnd();
        
        // Draw a circle outline
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 32; i++) {
            float angle = 2.0f * 3.14159f * i / 32.0f;
            float radius = 50.0f;
            glVertex2f(radius * cosf(angle), radius * sinf(angle));
        }
        glEnd();
    }
    
    void clipPath(rive::RenderPath* path) override {
        // Basic clipping implementation
    }
    
    void drawImage(const rive::RenderImage* image, rive::ImageSampler sampler, 
                   rive::BlendMode blendMode, float opacity) override {
        // Image drawing placeholder
    }
    
    void drawImageMesh(const rive::RenderImage* image,
                       rive::ImageSampler sampler,
                       rive::rcp<rive::RenderBuffer> vertices_f32,
                       rive::rcp<rive::RenderBuffer> uvCoords_f32,
                       rive::rcp<rive::RenderBuffer> indices_u16,
                       uint32_t vertexCount,
                       uint32_t indexCount,
                       rive::BlendMode blendMode,
                       float opacity) override {
        // Image mesh drawing placeholder
    }
    
    void setupViewport() {
        glViewport(0, 0, windowWidth, windowHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        // Set up orthographic projection to match artboard
        glOrtho(-100, 100, -100, 100, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Clear background
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void drawTestPattern() {
        // Always draw a test pattern so we know OpenGL is working
        static float testTime = 0.0f;
        testTime += 0.05f;
        
        // Draw a simple animated test pattern
        glColor3f(0.5f + 0.3f * sinf(testTime), 0.3f, 0.7f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0, 20);
        glVertex2f(-20, -20);
        glVertex2f(20, -20);
        glEnd();
        
        // Draw corner indicators to show the viewport
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        glVertex2f(-90, -90);
        glVertex2f(90, -90);
        glVertex2f(90, 90);
        glVertex2f(-90, 90);
        glEnd();
    }
    
    void drawPerformanceHUD(double currentFPS, double avgFrameTime, const std::string& rendererName) {
        // Save current matrix
        glPushMatrix();
        glLoadIdentity();
        
        // Set up 2D overlay projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        
        // Draw semi-transparent background for text readability
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex2f(10, windowHeight - 120);
        glVertex2f(350, windowHeight - 120);
        glVertex2f(350, windowHeight - 10);
        glVertex2f(10, windowHeight - 10);
        glEnd();
        glDisable(GL_BLEND);
        
        // Draw performance bars and indicators
        
        // FPS bar (target 60 FPS)
        float fpsRatio = std::min(currentFPS / 60.0, 2.0); // Cap at 2x for display
        glColor3f(fpsRatio > 0.9f ? 0.0f : 1.0f, fpsRatio > 0.9f ? 1.0f : 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(80, windowHeight - 30);
        glVertex2f(80 + fpsRatio * 150, windowHeight - 30);
        glVertex2f(80 + fpsRatio * 150, windowHeight - 20);
        glVertex2f(80, windowHeight - 20);
        glEnd();
        
        // Frame time indicator (16.67ms for 60fps)
        float frameTimeRatio = std::min(avgFrameTime / 0.0167, 2.0); // 16.67ms target
        glColor3f(frameTimeRatio > 1.1f ? 1.0f : 0.0f, frameTimeRatio > 1.1f ? 0.0f : 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(80, windowHeight - 50);
        glVertex2f(80 + frameTimeRatio * 150, windowHeight - 50);
        glVertex2f(80 + frameTimeRatio * 150, windowHeight - 40);
        glVertex2f(80, windowHeight - 40);
        glEnd();
        
        // Renderer type indicator
        if (rendererName.find("llvmpipe") != std::string::npos) {
            // Software - red square
            glColor3f(1.0f, 0.0f, 0.0f);
        } else {
            // Hardware - green square  
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        glBegin(GL_QUADS);
        glVertex2f(15, windowHeight - 90);
        glVertex2f(35, windowHeight - 90);
        glVertex2f(35, windowHeight - 70);
        glVertex2f(15, windowHeight - 70);
        glEnd();
        
        // Draw text using simple bitmap rendering (draw as lines/shapes)
        glColor3f(1.0f, 1.0f, 1.0f); // White text
        
        // Draw large performance numbers as simple shapes
        drawLargeNumber(245, windowHeight - 30, (int)currentFPS);
        drawLargeNumber(245, windowHeight - 50, (int)(avgFrameTime * 1000)); // ms
        
        // Draw simple text labels
        drawSimpleText(40, windowHeight - 28, "FPS:");
        drawSimpleText(40, windowHeight - 48, "MS:");
        drawSimpleText(40, windowHeight - 78, rendererName.find("llvmpipe") != std::string::npos ? "SOFTWARE" : "HARDWARE");
        
        // Restore matrices
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    
    void drawLargeNumber(float x, float y, int number) {
        // Draw numbers as simple line segments (7-segment display style)
        std::string numStr = std::to_string(number);
        float digitWidth = 15.0f;
        
        for (size_t i = 0; i < numStr.length(); i++) {
            drawDigit(x + i * digitWidth, y, numStr[i] - '0');
        }
    }
    
    void drawDigit(float x, float y, int digit) {
        // Simple 7-segment display rendering
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        
        switch(digit) {
            case 0: // Draw segments: top, top-right, bottom-right, bottom, bottom-left, top-left
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x+10, y); glVertex2f(x+10, y-5); // top-right
                glVertex2f(x+10, y-5); glVertex2f(x+10, y-10); // bottom-right
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                glVertex2f(x, y-5); glVertex2f(x, y-10); // bottom-left
                glVertex2f(x, y); glVertex2f(x, y-5); // top-left
                break;
            case 1: // right side only
                glVertex2f(x+10, y); glVertex2f(x+10, y-10);
                break;
            case 2: // top, top-right, middle, bottom-left, bottom
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x+10, y); glVertex2f(x+10, y-5); // top-right
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                glVertex2f(x, y-5); glVertex2f(x, y-10); // bottom-left
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                break;
            case 3: // top, top-right, middle, bottom-right, bottom
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x+10, y); glVertex2f(x+10, y-5); // top-right
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                glVertex2f(x+10, y-5); glVertex2f(x+10, y-10); // bottom-right
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                break;
            case 4: // top-left, top-right, middle, bottom-right
                glVertex2f(x, y); glVertex2f(x, y-5); // top-left
                glVertex2f(x+10, y); glVertex2f(x+10, y-10); // right side
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                break;
            case 5: // top, top-left, middle, bottom-right, bottom
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x, y); glVertex2f(x, y-5); // top-left
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                glVertex2f(x+10, y-5); glVertex2f(x+10, y-10); // bottom-right
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                break;
            case 6: // top, top-left, middle, bottom-left, bottom, bottom-right
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x, y); glVertex2f(x, y-10); // left side
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                glVertex2f(x+10, y-5); glVertex2f(x+10, y-10); // bottom-right
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                break;
            case 7: // top, top-right, bottom-right
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x+10, y); glVertex2f(x+10, y-10); // right side
                break;
            case 8: // all segments
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x+10, y); glVertex2f(x+10, y-5); // top-right
                glVertex2f(x+10, y-5); glVertex2f(x+10, y-10); // bottom-right
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                glVertex2f(x, y-5); glVertex2f(x, y-10); // bottom-left
                glVertex2f(x, y); glVertex2f(x, y-5); // top-left
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                break;
            case 9: // top, top-left, top-right, middle, bottom-right, bottom
                glVertex2f(x, y); glVertex2f(x+10, y); // top
                glVertex2f(x, y); glVertex2f(x, y-5); // top-left
                glVertex2f(x+10, y); glVertex2f(x+10, y-10); // right side
                glVertex2f(x, y-5); glVertex2f(x+10, y-5); // middle
                glVertex2f(x, y-10); glVertex2f(x+10, y-10); // bottom
                break;
        }
        glEnd();
    }
    
    void drawSimpleText(float x, float y, const std::string& text) {
        // Draw simple text using basic shapes
        glLineWidth(1.0f);
        // For simplicity, just draw a line to indicate text position
        glBegin(GL_LINES);
        glVertex2f(x, y-2);
        glVertex2f(x + text.length() * 6, y-2);
        glEnd();
    }
};

// Performance metrics
struct PerformanceMetrics {
    double totalTime = 0.0;
    int frameCount = 0;
    double minFrameTime = std::numeric_limits<double>::max();
    double maxFrameTime = 0.0;
    
    void addFrame(double frameTime) {
        totalTime += frameTime;
        frameCount++;
        minFrameTime = std::min(minFrameTime, frameTime);
        maxFrameTime = std::max(maxFrameTime, frameTime);
    }
    
    double getAverageFPS() const {
        return frameCount > 0 ? frameCount / totalTime : 0.0;
    }
    
    void print(const std::string& mode) const {
        std::cout << "\n" << mode << " Performance:" << std::endl;
        std::cout << "  Total frames: " << frameCount << std::endl;
        std::cout << "  Average FPS: " << getAverageFPS() << std::endl;
        std::cout << "  Min frame time: " << minFrameTime * 1000 << " ms" << std::endl;
        std::cout << "  Max frame time: " << maxFrameTime * 1000 << " ms" << std::endl;
        std::cout << "  Average frame time: " << (frameCount > 0 ? (totalTime / frameCount) * 1000 : 0) << " ms" << std::endl;
    }
};

class RiveWindow {
private:
    Display* display;
    Window window;
    GLXContext glContext;
    int windowWidth;
    int windowHeight;
    
public:
    RiveWindow(int width, int height) : windowWidth(width), windowHeight(height) {
        display = XOpenDisplay(nullptr);
        if (!display) {
            throw std::runtime_error("Cannot open X display");
        }
        
        // Get a matching FB config
        static int visual_attribs[] = {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            0
        };
        
        int glx_major, glx_minor;
        if (!glXQueryVersion(display, &glx_major, &glx_minor)) {
            XCloseDisplay(display);
            throw std::runtime_error("GLX not supported");
        }
        
        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
        if (!fbc) {
            XCloseDisplay(display);
            throw std::runtime_error("Failed to retrieve a framebuffer config");
        }
        
        GLXFBConfig bestFbc = fbc[0];
        XFree(fbc);
        
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, bestFbc);
        
        XSetWindowAttributes swa;
        swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
        swa.background_pixmap = 0L;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask | KeyPressMask;
        
        window = XCreateWindow(display, RootWindow(display, vi->screen),
                              0, 0, windowWidth, windowHeight, 0, vi->depth, InputOutput,
                              vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
        
        if (!window) {
            XFree(vi);
            XCloseDisplay(display);
            throw std::runtime_error("Failed to create window");
        }
        
        XStoreName(display, window, "Rive Animation");
        XMapWindow(display, window);
        
        glContext = glXCreateContext(display, vi, nullptr, GL_TRUE);
        XFree(vi);
        
        if (!glContext) {
            XDestroyWindow(display, window);
            XCloseDisplay(display);
            throw std::runtime_error("Failed to create OpenGL context");
        }
        
        glXMakeCurrent(display, window, glContext);
        
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    }
    
    ~RiveWindow() {
        glXMakeCurrent(display, 0L, nullptr);
        glXDestroyContext(display, glContext);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
    
    void swapBuffers() {
        glXSwapBuffers(display, window);
    }
    
    bool checkEvents() {
        XEvent xev;
        while (XPending(display)) {
            XNextEvent(display, &xev);
            if (xev.type == KeyPress) {
                return false; // Exit on any key press
            }
        }
        return true;
    }
    
    int getWidth() const { return windowWidth; }
    int getHeight() const { return windowHeight; }
};

int main(int argc, char* argv[]) {
    std::string riveFile = "fire_button.riv";
    if (argc > 1) {
        riveFile = argv[1];
    }
    
    std::cout << "Rive Visual Test" << std::endl;
    std::cout << "Loading: " << riveFile << std::endl;
    std::cout << "Press any key to exit" << std::endl;
    
    try {
        // Create window
        RiveWindow window(800, 600);
        
        // Load the .riv file
        std::ifstream file(riveFile, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open Rive file: " << riveFile << std::endl;
            return -1;
        }
        
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> bytes(length);
        file.read(reinterpret_cast<char*>(bytes.data()), length);
        file.close();
        
        // Import the Rive file
        rive::NoOpFactory factory;
        auto riveFilePtr = rive::File::import(rive::Span<const uint8_t>(bytes.data(), bytes.size()), &factory);
        if (!riveFilePtr) {
            std::cerr << "Failed to import Rive file" << std::endl;
            return -1;
        }
        
        // Get the artboard
        auto artboard = riveFilePtr->artboardDefault();
        if (!artboard) {
            std::cerr << "No artboard found in Rive file" << std::endl;
            return -1;
        }
        
        std::cout << "Artboard loaded: " << artboard->name() << std::endl;
        std::cout << "Dimensions: " << artboard->width() << " x " << artboard->height() << std::endl;
        std::cout << "Animation count: " << artboard->animationCount() << std::endl;
        
        // Get the first animation
        std::unique_ptr<rive::LinearAnimationInstance> animation;
        if (artboard->animationCount() > 0) {
            animation = artboard->animationAt(0);
            animation->time(0);
            animation->apply();
            std::cout << "Animation loaded: " << artboard->animation(0)->name() << std::endl;
            std::cout << "Duration: " << animation->durationSeconds() << " seconds" << std::endl;
        }
        
        // Create renderer
        SimpleOpenGLRenderer renderer(window.getWidth(), window.getHeight());
        
        // Animation loop
        PerformanceMetrics metrics;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Run for 3 seconds for benchmarking, or until user input
        bool benchmark_mode = (argc > 2 && std::string(argv[2]) == "--benchmark");
        auto benchmark_duration = std::chrono::seconds(3);
        
        // Real-time FPS calculation
        std::string rendererName = (const char*)glGetString(GL_RENDERER);
        double currentFPS = 0.0;
        int fpsFrameCount = 0;
        auto fpsStartTime = startTime;
        
        while (window.checkEvents() && 
               (!benchmark_mode || (std::chrono::high_resolution_clock::now() - startTime) < benchmark_duration)) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            // Update animation
            if (animation) {
                animation->advance(1.0 / 60.0); // Advance by 1/60th of a second
                animation->apply();
            }
            
            // Render
            renderer.setupViewport();
            
            // Always draw test pattern first
            renderer.drawTestPattern();
            
            // Apply artboard transform to center it
            renderer.save();
            renderer.transform(rive::Mat2D::fromScale(1.0f, 1.0f)); // Keep original scale
            
            // Draw the artboard
            artboard->draw(&renderer);
            
            renderer.restore();
            
            auto frameEnd = std::chrono::high_resolution_clock::now();
            double frameTime = std::chrono::duration<double>(frameEnd - frameStart).count();
            
            // Calculate real-time FPS every 30 frames
            fpsFrameCount++;
            if (fpsFrameCount >= 30) {
                auto fpsCurrentTime = std::chrono::high_resolution_clock::now();
                double fpsDuration = std::chrono::duration<double>(fpsCurrentTime - fpsStartTime).count();
                currentFPS = fpsFrameCount / fpsDuration;
                fpsFrameCount = 0;
                fpsStartTime = fpsCurrentTime;
                
                // Print to console every 30 frames so we can see the numbers
                std::cout << "LIVE: " << rendererName.substr(0, 20) << " | FPS: " << (int)currentFPS 
                         << " | Frame Time: " << (int)(frameTime * 1000) << "ms" << std::endl;
            }
            metrics.addFrame(frameTime);
            
            // Draw performance HUD on top
            renderer.drawPerformanceHUD(currentFPS, frameTime, rendererName);
            
            // Swap buffers
            window.swapBuffers();
            
            // Target 60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // Print final performance results with renderer information
        std::cout << "\n=== FINAL PERFORMANCE RESULTS ===" << std::endl;
        std::cout << "Renderer: " << rendererName << std::endl;
        std::cout << "Renderer Type: " << (rendererName.find("llvmpipe") != std::string::npos ? "SOFTWARE (CPU)" : "HARDWARE (GPU)") << std::endl;
        std::cout << "Final Real-time FPS: " << (int)currentFPS << std::endl;
        std::cout << "Average Frame Time: " << (metrics.frameCount > 0 ? (metrics.totalTime / metrics.frameCount) * 1000 : 0) << " ms" << std::endl;
        std::cout << "=================================" << std::endl;
        
        metrics.print("OpenGL Renderer");
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "\nWindow closed!" << std::endl;
    return 0;
}