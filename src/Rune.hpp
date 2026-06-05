#pragma once

#include <array>
#include <random>

namespace runegenerator {

class Rune {
public:
    static constexpr int WIDTH = 5;
    static constexpr int HEIGHT = 7;

    Rune();

    [[nodiscard]] bool pixel(int x, int y) const;
    [[nodiscard]] bool isTip(int x, int y) const;
    [[nodiscard]] int diff(const Rune& another) const;
    [[nodiscard]] Rune mirrorX() const;
    [[nodiscard]] Rune mirrorY() const;
    [[nodiscard]] Rune rotate180() const;
    [[nodiscard]] bool isSymmetric();

    bool hSym = false;
    bool vSym = false;

private:
    using Bitmap = std::array<std::array<bool, WIDTH>, HEIGHT>;

    explicit Rune(const Bitmap& bitmap);

    void random1();
    void random2();
    [[nodiscard]] int getWeight() const;
    [[nodiscard]] bool isConnected(int weight);
    [[nodiscard]] int countNeighbours(int x, int y, int radius = 1) const;
    [[nodiscard]] static bool coin(double chance = 0.5);

    Bitmap bitmap_{};
    int dotx_ = -1;
    int doty_ = -1;

    static std::mt19937& rng();
};

} // namespace runegenerator
