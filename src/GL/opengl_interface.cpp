#include "opengl_interface.hpp"
#include "../tower_sim.hpp"

namespace GL
{

    void handle_error(const std::string &prefix, const GLenum err)
    {
        if (err != GL_NO_ERROR)
        {
            throw std::runtime_error{prefix + std::string{": "} +
                                     std::string{reinterpret_cast<const char *>(gluErrorString(err))}};
        }
    }

    void keyboard(unsigned char key, int, int)
    {
        const auto iter = keystrokes.find(key);
        if (iter != keystrokes.end())
        {
            (iter->second)();
        }
    }

    void toggle_fullscreen()
    {
        if (fullscreen)
        {
            glutPositionWindow(10, 10);
            glutReshapeWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
        }
        else
        {
            glutFullScreen();
        }

        handle_error("Toggle fullscreen");
        fullscreen = !fullscreen;
    }

    void change_zoom(float factor)
    {
        zoom *= factor;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-zoom, zoom, -zoom, zoom, 0.0f, 1.0f); // left, right, bottom, top, near, far
        handle_error("Zoom");
    }
    void change_framerate(int amount)
    {
        if (old_framerate != 0)
        {
            return;
        }
        const int new_ticks = (int)ticks_per_sec + amount;
        if (new_ticks >= 10 && new_ticks <= 120)
        {
            ticks_per_sec = new_ticks;
        }
    }
    void change_framerate_modifier(double delta)
    {
        if (framerate_modifier <= 5 && framerate_modifier > 0)
        {
            framerate_modifier *= delta;
        }
    }

    void reshape_window(int w, int h)
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-zoom, zoom, -zoom, zoom, 0.0f, 1.0f); // left, right, bottom, top, near, far
        handle_error("Cannot reshape window");
    }

    void display()
    {
        // sort the displayable by their z-coordinate
        std::sort(display_queue.begin(), display_queue.end(), disp_z_cmp{});
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-zoom, zoom, -zoom, zoom, 0.0f, 1.0f); // left, right, bottom, top, near, far
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        for (const auto &item : display_queue)
        {
            item->display();
        }
        glDisable(GL_TEXTURE_2D);
        glutSwapBuffers();
    }

    void timer(const int step)
    {

        if (ticks_per_sec != 0)
        {
            if (!is_paused)
            {

                const double dt = framerate_modifier * std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - oldTime).count();
                for (const auto &dynamic : move_queue)
                {
                    dynamic->move(dt);
                }
            }
            glutPostRedisplay();
            oldTime = std::chrono::system_clock::now();
            glutTimerFunc(1000u / ticks_per_sec, timer, step + 1);
        }

        else
        {
            if (!is_paused)
            {
                oldTime = std::chrono::system_clock::now();
            }
            glutTimerFunc(0, timer, step + 1);
        }
    }
    void init_gl(int argc, char **argv, const char *title)
    {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA);
        glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
        glutCreateWindow(title);
        // glutFullScreen();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        // glEnable(GL_DEPTH_TEST);
        // The following two lines enable semi transparent
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glShadeModel(GL_FLAT);

        glutKeyboardFunc(keyboard);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape_window);

        handle_error("Cannot init OpenGL");
    }

    void pause()
    {
        auto new_framerate = old_framerate;
        old_framerate = ticks_per_sec;
        ticks_per_sec = new_framerate;
        oldTime = std::chrono::system_clock::now();
    }

    void loop()
    {
        oldTime = std::chrono::system_clock::now();
        glutTimerFunc(100, timer, 0);
        glutMainLoop();
    }

    void exit_loop()
    {
        glutLeaveMainLoop();
    }

} // namespace GL