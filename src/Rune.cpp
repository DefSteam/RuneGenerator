#include "Rune.hpp"

#include <algorithm>
#include <functional>
#include <vector>

namespace runegenerator {
namespace {
constexpr int MIN_WEIGHT = 11;
constexpr int MAX_WEIGHT = 21;
}

Rune::Rune() {
    int weight = 0;
    do {
        bitmap_ = {};

        if (coin(5.0 / 7.0)) {
            random1();
        } else {
            random2();
        }

        weight = getWeight();
    } while (weight < MIN_WEIGHT || weight > MAX_WEIGHT || !isConnected(weight));
}

Rune::Rune(const Bitmap& bitmap) : bitmap_(bitmap) {}

bool Rune::pixel(int x, int y) const {
    return bitmap_[y][x];
}

bool Rune::isTip(int x, int y) const {
    return countNeighbours(x, y) == 1;
}

void Rune::random1() {
    bitmap_[0][0] = coin(4.0 / 5.0);
    bitmap_[0][2] = coin(4.0 / 5.0);
    bitmap_[0][4] = coin(4.0 / 5.0);

    bitmap_[3][0] = coin(4.0 / 5.0);
    bitmap_[3][2] = coin(4.0 / 5.0);
    bitmap_[3][4] = coin(4.0 / 5.0);

    bitmap_[6][0] = coin(4.0 / 5.0);
    bitmap_[6][2] = coin(4.0 / 5.0);
    bitmap_[6][4] = coin(2.0 / 5.0);

    bitmap_[0][1] = bitmap_[0][0] && bitmap_[0][2] && coin(3.0 / 4.0);
    bitmap_[0][3] = bitmap_[0][2] && bitmap_[0][4] && coin(3.0 / 4.0);

    bitmap_[1][0] = bitmap_[2][0] = bitmap_[0][0] && bitmap_[3][0] && coin(2.0 / 3.0);
    bitmap_[1][2] = bitmap_[2][2] = bitmap_[0][2] && bitmap_[3][2] && coin(1.0 / 4.0);
    bitmap_[1][4] = bitmap_[2][4] = bitmap_[0][4] && bitmap_[3][4] && coin(4.0 / 5.0);

    bitmap_[3][1] = bitmap_[3][0] && bitmap_[3][2] && coin(3.0 / 4.0);
    bitmap_[3][3] = bitmap_[3][2] && bitmap_[3][4] && coin(2.0 / 5.0);

    bitmap_[4][0] = bitmap_[5][0] = bitmap_[3][0] && bitmap_[6][0] && coin(3.0 / 4.0);
    bitmap_[4][2] = bitmap_[5][2] = bitmap_[3][2] && bitmap_[6][2] && coin(2.0 / 4.0);
    bitmap_[4][4] = bitmap_[5][4] = bitmap_[3][4] && bitmap_[6][4] && coin(3.0 / 4.0);

    bitmap_[6][1] = bitmap_[6][0] && bitmap_[6][2] && coin(4.0 / 5.0);
    bitmap_[6][3] = bitmap_[6][2] && bitmap_[6][4] && coin(4.0 / 5.0);

    if (bitmap_[4][2] && (bitmap_[3][1] != bitmap_[3][3])) {
        bitmap_[5][2] = false;
    }
}

void Rune::random2() {
    bitmap_[0][0] = coin(1.0 / 2.0);
    bitmap_[0][2] = coin(1.0 / 2.0);
    bitmap_[0][4] = coin(1.0 / 2.0);

    bitmap_[2][0] = coin(3.0 / 4.0);
    bitmap_[2][2] = coin(3.0 / 4.0);
    bitmap_[2][4] = coin(3.0 / 4.0);

    bitmap_[4][0] = coin(1.0 / 2.0);
    bitmap_[4][2] = coin(1.0 / 2.0);
    bitmap_[4][4] = coin(1.0 / 2.0);

    bitmap_[6][0] = coin(1.0 / 2.0);
    bitmap_[6][2] = coin(1.0 / 2.0);
    bitmap_[6][4] = coin(1.0 / 2.0);

    bitmap_[0][1] = bitmap_[0][0] && bitmap_[0][2] && coin(1.0 / 4.0);
    bitmap_[0][3] = bitmap_[0][2] && bitmap_[0][4] && coin(1.0 / 4.0);

    bitmap_[1][0] = bitmap_[0][0] && bitmap_[2][0] && coin(3.0 / 4.0);
    bitmap_[1][2] = bitmap_[0][2] && bitmap_[2][2] && coin(3.0 / 4.0);
    bitmap_[1][4] = bitmap_[0][4] && bitmap_[2][4] && coin(3.0 / 4.0);

    bitmap_[2][1] = bitmap_[2][0] && bitmap_[2][2] && coin(3.0 / 4.0);
    bitmap_[2][3] = bitmap_[2][2] && bitmap_[2][4] && coin(3.0 / 4.0);

    bitmap_[3][0] = bitmap_[2][0] && bitmap_[4][0] && coin(3.0 / 4.0);
    bitmap_[3][2] = bitmap_[2][2] && bitmap_[4][2] && coin(1.0 / 2.0);
    bitmap_[3][4] = bitmap_[2][4] && bitmap_[4][4] && coin(3.0 / 4.0);

    bitmap_[4][1] = bitmap_[4][2];
    bitmap_[4][3] = bitmap_[4][2];

    bitmap_[5][0] = bitmap_[4][0] && bitmap_[6][0] && coin(3.0 / 4.0);
    bitmap_[5][2] = bitmap_[4][2] && bitmap_[6][2] && coin(1.0 / 2.0);
    bitmap_[5][4] = bitmap_[4][4] && bitmap_[6][4] && coin(3.0 / 4.0);

    bitmap_[6][1] = bitmap_[6][0] && bitmap_[6][2] && coin(1.0 / 4.0);
    bitmap_[6][3] = bitmap_[6][2] && bitmap_[6][4] && coin(1.0 / 4.0);
}

int Rune::getWeight() const {
    int weight = 0;
    for (const auto& row : bitmap_) {
        for (bool cell : row) {
            if (cell) {
                ++weight;
            }
        }
    }
    return weight;
}

bool Rune::isConnected(int weight) {
    bool left = false;
    bool right = false;
    for (int y = 0; y < HEIGHT; ++y) {
        left = left || bitmap_[y][0];
        right = right || bitmap_[y][WIDTH - 1];
    }
    if (!left || !right) {
        return false;
    }

    bool top = false;
    bool bottom = false;
    for (int x = 0; x < WIDTH; ++x) {
        top = top || bitmap_[0][x];
        bottom = bottom || bitmap_[HEIGHT - 1][x];
    }
    if (!top || !bottom) {
        return false;
    }

    dotx_ = -1;
    doty_ = -1;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (bitmap_[y][x] && countNeighbours(x, y) == 0) {
                if (dotx_ == -1) {
                    if (countNeighbours(x, y, 2) == 0) {
                        return false;
                    }
                    dotx_ = x;
                    doty_ = y;
                } else {
                    return false;
                }
            }
        }
    }

    int startX = -1;
    int startY = -1;
    for (int y = 0; y < HEIGHT && startX == -1; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (bitmap_[y][x] && (y != doty_ || x != dotx_)) {
                startX = x;
                startY = y;
                break;
            }
        }
    }

    if (startX == -1) {
        return false;
    }

    std::vector<int> checked;
    std::function<int(int, int)> fill = [&](int x, int y) -> int {
        const int index = x + y * WIDTH;
        if (!bitmap_[y][x] || std::find(checked.begin(), checked.end(), index) != checked.end()) {
            return 0;
        }

        checked.push_back(index);

        int area = 1;
        if (x > 0) {
            area += fill(x - 1, y);
        }
        if (x < WIDTH - 1) {
            area += fill(x + 1, y);
        }
        if (y > 0) {
            area += fill(x, y - 1);
        }
        if (y < HEIGHT - 1) {
            area += fill(x, y + 1);
        }
        return area;
    };

    const int area = fill(startX, startY);
    return area == weight || area == weight - 1;
}

int Rune::diff(const Rune& another) const {
    int result = 0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (bitmap_[y][x] != another.bitmap_[y][x]) {
                ++result;
            }
        }
    }
    return result;
}

int Rune::countNeighbours(int x, int y, int radius) const {
    int neighbours = 0;
    if (x > radius - 1 && bitmap_[y][x - radius]) {
        ++neighbours;
    }
    if (x < WIDTH - radius && bitmap_[y][x + radius]) {
        ++neighbours;
    }
    if (y > radius - 1 && bitmap_[y - radius][x]) {
        ++neighbours;
    }
    if (y < HEIGHT - radius && bitmap_[y + radius][x]) {
        ++neighbours;
    }
    return neighbours;
}

Rune Rune::mirrorX() const {
    Bitmap bitmap{};
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            bitmap[y][x] = bitmap_[y][WIDTH - x - 1];
        }
    }
    return Rune(bitmap);
}

Rune Rune::mirrorY() const {
    Bitmap bitmap{};
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            bitmap[y][x] = bitmap_[HEIGHT - y - 1][x];
        }
    }
    return Rune(bitmap);
}

Rune Rune::rotate180() const {
    Bitmap bitmap{};
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            bitmap[y][x] = bitmap_[HEIGHT - y - 1][WIDTH - x - 1];
        }
    }
    return Rune(bitmap);
}

bool Rune::isSymmetric() {
    hSym = true;
    for (const auto& row : bitmap_) {
        if (row[0] != row[4] || row[1] != row[3]) {
            hSym = false;
            break;
        }
    }

    vSym = true;
    for (int x = 0; x < WIDTH; ++x) {
        if (bitmap_[0][x] != bitmap_[6][x] || bitmap_[1][x] != bitmap_[5][x] || bitmap_[2][x] != bitmap_[4][x]) {
            vSym = false;
            break;
        }
    }

    return hSym || vSym;
}

bool Rune::coin(double chance) {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(rng()) < chance;
}

std::mt19937& Rune::rng() {
    static std::random_device device;
    static std::mt19937 generator(device());
    return generator;
}

} // namespace runegenerator
