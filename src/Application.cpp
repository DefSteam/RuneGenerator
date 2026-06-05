#include "Application.hpp"

#include <X11/keysym.h>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <random>

namespace runegenerator {
namespace {
constexpr unsigned int WINDOW_BACKGROUND = 0xCCCAC2;
constexpr unsigned int RUNE_COLOR = 0x222222;
constexpr unsigned int TIP_ALPHA = 0x99;
constexpr int BUTTON_WIDTH = 34;
constexpr int BUTTON_HEIGHT = 14;
constexpr int BUTTON_GAP = 4;
constexpr int BUTTONS_WIDTH = BUTTON_WIDTH * 2 + BUTTON_GAP;
constexpr int BUTTONS_HEIGHT = BUTTON_HEIGHT;

bool chance(double value) {
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator) < value;
}

unsigned int blendOverBackground(unsigned int foreground, unsigned int alpha) {
    const unsigned int bgR = (WINDOW_BACKGROUND >> 16U) & 0xFFU;
    const unsigned int bgG = (WINDOW_BACKGROUND >> 8U) & 0xFFU;
    const unsigned int bgB = WINDOW_BACKGROUND & 0xFFU;
    const unsigned int fgR = (foreground >> 16U) & 0xFFU;
    const unsigned int fgG = (foreground >> 8U) & 0xFFU;
    const unsigned int fgB = foreground & 0xFFU;

    const auto mix = [alpha](unsigned int fg, unsigned int bg) {
        return (fg * alpha + bg * (255U - alpha)) / 255U;
    };

    return (mix(fgR, bgR) << 16U) | (mix(fgG, bgG) << 8U) | mix(fgB, bgB);
}
} // namespace

bool Rect::contains(int px, int py) const {
    return px >= x && py >= y && px < x + w && py < y + h;
}

Application::Application() {
    createWindow();
    switchToAlphabet();
}

Application::~Application() {
    if (gc_ != nullptr) {
        XFreeGC(display_, gc_);
    }
    if (display_ != nullptr && window_ != 0) {
        XDestroyWindow(display_, window_);
    }
    if (display_ != nullptr) {
        XCloseDisplay(display_);
    }
}

int Application::run() {
    XEvent event{};
    while (running_) {
        XNextEvent(display_, &event);
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                    redraw();
                }
                break;
            case ConfigureNotify:
                windowWidth_ = event.xconfigure.width;
                windowHeight_ = event.xconfigure.height;
                updateScale();
                relayout();
                redraw();
                break;
            case ButtonPress:
                if (scene_ != nullptr && event.xbutton.button == Button1) {
                    handlingSceneEvent_ = true;
                    scene_->mouseDown(event.xbutton.x / scale_, event.xbutton.y / scale_);
                    handlingSceneEvent_ = false;
                    dispatchSceneChange();
                    redraw();
                }
                break;
            case KeyPress:
                if (scene_ != nullptr) {
                    const KeySym key = XLookupKeysym(&event.xkey, 0);
                    if (key == XK_Escape) {
                        quit();
                    } else {
                        handlingSceneEvent_ = true;
                        scene_->keyDown(key);
                        handlingSceneEvent_ = false;
                        dispatchSceneChange();
                        redraw();
                    }
                }
                break;
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == wmDelete_) {
                    quit();
                }
                break;
            default:
                break;
        }
    }
    return EXIT_SUCCESS;
}

void Application::switchToAlphabet() {
    if (handlingSceneEvent_) {
        pendingScene_ = PendingScene::Alphabet;
        return;
    }
    scene_ = std::make_unique<AlphabetScene>(*this);
    relayout();
    redraw();
}

void Application::switchToRunes() {
    if (handlingSceneEvent_) {
        pendingScene_ = PendingScene::Runes;
        return;
    }
    scene_ = std::make_unique<RunesScene>(*this);
    relayout();
    redraw();
}

void Application::quit() {
    running_ = false;
}

void Application::drawPixel(int logicalX, int logicalY, unsigned long color) {
    fillRect({logicalX, logicalY, 1, 1}, color);
}

void Application::fillRect(const Rect& rect, unsigned long color) {
    XSetForeground(display_, gc_, color);
    XFillRectangle(display_, window_, gc_, rect.x * scale_, rect.y * scale_, rect.w * scale_, rect.h * scale_);
}

void Application::drawRune(const Rune& rune, int logicalX, int logicalY, int cellSize) {
    const unsigned long solid = rgb(0x22, 0x22, 0x22);
    const unsigned int tipRgb = blendOverBackground(RUNE_COLOR, TIP_ALPHA);
    const unsigned long tip = rgb((tipRgb >> 16U) & 0xFFU, (tipRgb >> 8U) & 0xFFU, tipRgb & 0xFFU);

    for (int y = 0; y < Rune::HEIGHT; ++y) {
        for (int x = 0; x < Rune::WIDTH; ++x) {
            if (rune.pixel(x, y)) {
                fillRect({logicalX + x * cellSize, logicalY + y * cellSize, cellSize, cellSize}, rune.isTip(x, y) ? tip : solid);
            }
        }
    }
}

void Application::drawTableButton(const Rect& rect, bool active) {
    const unsigned long fill = active ? rgb(0xEE, 0xEC, 0xE4) : rgb(0xDE, 0xDC, 0xD4);
    const unsigned long stroke = rgb(0x22, 0x22, 0x22);
    fillRect(rect, fill);

    // Table icon: ten tiny runes arranged as a grid.
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            fillRect({rect.x + 4 + col * 6, rect.y + 3 + row * 5, 3, 2}, stroke);
        }
    }
}

void Application::drawAlphabetButton(const Rect& rect, bool active) {
    const unsigned long fill = active ? rgb(0xEE, 0xEC, 0xE4) : rgb(0xDE, 0xDC, 0xD4);
    const unsigned long stroke = rgb(0x22, 0x22, 0x22);
    fillRect(rect, fill);

    // Alphabet icon: three rows of glyph slots.
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 8; ++col) {
            const int x = rect.x + 3 + col * 4;
            const int y = rect.y + 2 + row * 4;
            fillRect({x, y, 1, 3}, stroke);
            if ((row + col) % 2 == 0) {
                fillRect({x + 1, y + 1, 1, 1}, stroke);
            }
        }
    }
}

unsigned long Application::rgb(unsigned int red, unsigned int green, unsigned int blue) const {
    return (red << 16U) | (green << 8U) | blue;
}

void Application::createWindow() {
    display_ = XOpenDisplay(nullptr);
    if (display_ == nullptr) {
        throw std::runtime_error("Unable to open an X11 display. Make sure DISPLAY is set.");
    }

    const int screen = DefaultScreen(display_);
    window_ = XCreateSimpleWindow(display_, RootWindow(display_, screen), 0, 0, windowWidth_, windowHeight_, 0,
                                  BlackPixel(display_, screen), rgb(0xCC, 0xCA, 0xC2));
    XStoreName(display_, window_, "RuneGenerator");
    XSelectInput(display_, window_, ExposureMask | StructureNotifyMask | ButtonPressMask | KeyPressMask);

    wmDelete_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDelete_, 1);

    gc_ = XCreateGC(display_, window_, 0, nullptr);
    XMapWindow(display_, window_);
    updateScale();
}

void Application::updateScale() {
    const int fit = std::min(windowWidth_ / 104, windowHeight_ / 143);
    scale_ = std::max(1, (fit / 2) * 2);
    logicalWidth_ = std::max(1, windowWidth_ / scale_);
    logicalHeight_ = std::max(1, windowHeight_ / scale_);
}

void Application::relayout() {
    if (scene_ != nullptr) {
        scene_->layout(logicalWidth_, logicalHeight_);
    }
}

void Application::redraw() {
    if (display_ == nullptr || window_ == 0 || scene_ == nullptr) {
        return;
    }
    clear();
    scene_->draw(*this);
    XFlush(display_);
}

void Application::clear() {
    fillRect({0, 0, logicalWidth_, logicalHeight_}, rgb(0xCC, 0xCA, 0xC2));
}

void Application::dispatchSceneChange() {
    const PendingScene next = pendingScene_;
    pendingScene_ = PendingScene::NoChange;
    switch (next) {
        case PendingScene::Alphabet:
            switchToAlphabet();
            break;
        case PendingScene::Runes:
            switchToRunes();
            break;
        case PendingScene::NoChange:
            break;
    }
}

RunesScene::RunesScene(Application& app) : app_(app) {
    regenerate();
}

void RunesScene::layout(int logicalWidth, int logicalHeight) {
    table_.w = 10 * Rune::WIDTH + 9 * 6;
    table_.h = 10 * Rune::HEIGHT + 9 * 6;
    const int totalHeight = table_.h + 5 + BUTTONS_HEIGHT;
    table_.x = (logicalWidth - table_.w) / 2;
    table_.y = (logicalHeight - totalHeight) / 2;

    const int buttonsX = (logicalWidth - BUTTONS_WIDTH) / 2;
    const int buttonsY = table_.y + table_.h + 5;
    tableButton_ = {buttonsX, buttonsY, BUTTON_WIDTH, BUTTON_HEIGHT};
    alphabetButton_ = {buttonsX + BUTTON_WIDTH + BUTTON_GAP, buttonsY, BUTTON_WIDTH, BUTTON_HEIGHT};
}

void RunesScene::draw(Application& app) {
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            app.drawRune(runes_[row * 10 + col], table_.x + col * (Rune::WIDTH + 6), table_.y + row * (Rune::HEIGHT + 6));
        }
    }
    app.drawTableButton(tableButton_, true);
    app.drawAlphabetButton(alphabetButton_, false);
}

void RunesScene::mouseDown(int logicalX, int logicalY) {
    if (tableButton_.contains(logicalX, logicalY)) {
        app_.switchToRunes();
    } else if (alphabetButton_.contains(logicalX, logicalY)) {
        app_.switchToAlphabet();
    }
}

void RunesScene::keyDown(KeySym key) {
    if (key == XK_Return || key == XK_KP_Enter) {
        app_.switchToRunes();
    }
}

void RunesScene::regenerate() {
    runes_.clear();
    runes_.reserve(100);
    for (int i = 0; i < 100; ++i) {
        runes_.emplace_back();
    }
}

AlphabetScene::AlphabetScene(Application& app) : app_(app) {
    regenerate();
}

void AlphabetScene::layout(int logicalWidth, int logicalHeight) {
    table_.w = COLS * Rune::WIDTH + (COLS - 1) * GAP;
    table_.h = 3 * Rune::HEIGHT + 2 * GAP;
    const int totalHeight = table_.h + 10 + BUTTONS_HEIGHT;
    table_.x = (logicalWidth - table_.w) / 2;
    table_.y = (logicalHeight - totalHeight) / 2;

    const int buttonsX = (logicalWidth - BUTTONS_WIDTH) / 2;
    const int buttonsY = table_.y + table_.h + 10;
    tableButton_ = {buttonsX, buttonsY, BUTTON_WIDTH, BUTTON_HEIGHT};
    alphabetButton_ = {buttonsX + BUTTON_WIDTH + BUTTON_GAP, buttonsY, BUTTON_WIDTH, BUTTON_HEIGHT};
}

void AlphabetScene::draw(Application& app) {
    for (int i = 0; i < static_cast<int>(alphabet_.size()); ++i) {
        const Rect rect = runeRect(i);
        app.drawRune(alphabet_[i], rect.x, rect.y);
    }
    app.drawTableButton(tableButton_, false);
    app.drawAlphabetButton(alphabetButton_, true);
}

void AlphabetScene::mouseDown(int logicalX, int logicalY) {
    if (tableButton_.contains(logicalX, logicalY)) {
        app_.switchToRunes();
        return;
    }
    if (alphabetButton_.contains(logicalX, logicalY)) {
        app_.switchToAlphabet();
        return;
    }

    for (int i = 0; i < static_cast<int>(alphabet_.size()); ++i) {
        if (runeRect(i).contains(logicalX, logicalY)) {
            replaceRune(i);
            return;
        }
    }
}

void AlphabetScene::keyDown(KeySym key) {
    if (key == XK_Return || key == XK_KP_Enter) {
        app_.switchToAlphabet();
    }
}

void AlphabetScene::regenerate() {
    alphabet_.clear();
    alphabet_.reserve(N);
    for (int i = 0; i < N; ++i) {
        Rune rune;
        while (!addRune(rune, i)) {
            rune = Rune();
        }
    }
}

bool AlphabetScene::addRune(Rune rune, int position) {
    if (!rune.isSymmetric() && chance(0.9)) {
        return false;
    }

    const Rune mirrX = rune.mirrorX();
    const Rune mirrY = rune.mirrorY();
    const Rune rotated = rune.rotate180();

    for (const Rune& existing : alphabet_) {
        if (rune.diff(existing) < 5) {
            return false;
        }

        if (!rune.hSym && existing.diff(mirrX) < 3) {
            return false;
        }

        if (!rune.vSym && existing.diff(mirrY) < 3) {
            return false;
        }

        if (!rune.vSym && !rune.hSym && existing.diff(rotated) < 1) {
            return false;
        }
    }

    alphabet_.insert(alphabet_.begin() + position, rune);
    return true;
}

void AlphabetScene::replaceRune(int index) {
    alphabet_.erase(alphabet_.begin() + index);

    Rune rune;
    while (!addRune(rune, index)) {
        rune = Rune();
    }
}

Rect AlphabetScene::runeRect(int index) const {
    return {table_.x + (index % COLS) * (Rune::WIDTH + GAP), table_.y + (index / COLS) * (Rune::HEIGHT + GAP), Rune::WIDTH, Rune::HEIGHT};
}

} // namespace runegenerator
