#pragma once

#include "Rune.hpp"

#include <X11/Xlib.h>

#include <memory>
#include <vector>

namespace runegenerator {

class Application;

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] bool contains(int px, int py) const;
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void layout(int logicalWidth, int logicalHeight) = 0;
    virtual void draw(Application& app) = 0;
    virtual void mouseDown(int logicalX, int logicalY) = 0;
    virtual void keyDown(KeySym key) = 0;
};

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();
    void switchToAlphabet();
    void switchToRunes();
    void quit();

    void drawPixel(int logicalX, int logicalY, unsigned long color);
    void fillRect(const Rect& rect, unsigned long color);
    void drawRune(const Rune& rune, int logicalX, int logicalY, int cellSize = 1);
    void drawTableButton(const Rect& rect, bool active);
    void drawAlphabetButton(const Rect& rect, bool active);

    [[nodiscard]] unsigned long rgb(unsigned int red, unsigned int green, unsigned int blue) const;

private:
    void createWindow();
    void updateScale();
    void relayout();
    void redraw();
    void clear();
    void dispatchSceneChange();

    enum class PendingScene {
        NoChange,
        Alphabet,
        Runes
    };

    Display* display_ = nullptr;
    Window window_ = 0;
    GC gc_ = nullptr;
    Atom wmDelete_ = 0;

    int windowWidth_ = 640;
    int windowHeight_ = 640;
    int scale_ = 2;
    int logicalWidth_ = 320;
    int logicalHeight_ = 320;
    bool running_ = true;
    bool handlingSceneEvent_ = false;
    PendingScene pendingScene_ = PendingScene::NoChange;

    std::unique_ptr<Scene> scene_;
};

class RunesScene final : public Scene {
public:
    explicit RunesScene(Application& app);

    void layout(int logicalWidth, int logicalHeight) override;
    void draw(Application& app) override;
    void mouseDown(int logicalX, int logicalY) override;
    void keyDown(KeySym key) override;

private:
    void regenerate();

    Application& app_;
    std::vector<Rune> runes_;
    Rect table_;
    Rect tableButton_;
    Rect alphabetButton_;
};

class AlphabetScene final : public Scene {
public:
    explicit AlphabetScene(Application& app);

    void layout(int logicalWidth, int logicalHeight) override;
    void draw(Application& app) override;
    void mouseDown(int logicalX, int logicalY) override;
    void keyDown(KeySym key) override;

private:
    static constexpr int N = 24;
    static constexpr int COLS = 8;
    static constexpr int GAP = 8;

    void regenerate();
    bool addRune(Rune rune, int position);
    void replaceRune(int index);
    [[nodiscard]] Rect runeRect(int index) const;

    Application& app_;
    std::vector<Rune> alphabet_;
    Rect table_;
    Rect tableButton_;
    Rect alphabetButton_;
};

} // namespace runegenerator
