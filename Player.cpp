#include <string>
#include <vector>
#include "Player.h"
#include "Types.h"

using std::vector;

Player::Player(std::string name) {
    this->name = name;
    points = 0;

    // intialize storage rows
    for (int i = 0; i < MOSAIC_DIM; ++i) {
        storageRow[i] = new vector<Tile>();
    }
    for (int i = 0; i < MOSAIC_DIM; ++i) {
        for (int j = 0; j < i+1; ++j) {
            storageRow[i]->push_back(EMPTY_SLOT);
        }
    }

    // initialize wall
    wall = new Tile*[MOSAIC_DIM];
    for (int i = 0; i < MOSAIC_DIM; ++i) {
         wall[i] = new Tile[MOSAIC_DIM];
    }
    for (int i = 0; i < MOSAIC_DIM; ++i) {
        for (int j = 0; j < MOSAIC_DIM; ++j) {
            wall[i][j] = EMPTY_SLOT;
        }
    }

    //intitialize floor line
    floorLine = new LinkedList();
}

Player::~Player() {
    // delete storage rows
    for (int i = 0; i < MOSAIC_DIM; ++i) {
        delete storageRow[i];
    }

    // delete wall
    for (int i = 0; i != MOSAIC_DIM; ++i) {
        delete[] wall[i];
    }
    delete[] wall;

    delete floorLine;
}

std::string Player::getName() {
    return name;
}

int Player::getPoints() {
    return points;
}

std::string Player::printPlayerBoard() {
    std::string result;
    
    // print storage rows and mosaic
    for (int i = 0; i < MOSAIC_DIM; ++i) {
        // row number
        result += std::to_string(i + 1) + ": ";

        // add empty space
        for (int j = 0; j < MOSAIC_DIM - i - 1; ++j) {
            result += "  ";
        }

        // add storage rows (in reverse)
        for (int j = i; j >= 0; --j) {
            result += std::string(1, storageRow[i]->at(j)) + " ";
        }

        // partition
        result += "||";

        // add mosaic
        for (int j = 0; j < MOSAIC_DIM; ++j) {
            result += " " + std::string(1, wall[i][j]);
        }

        result += "\n";
    }

    // add broken tiles
    result += "broken: ";
    result += floorLine->toString();

    result += "\n";
    return result;
}

bool Player::validateTurn(Tile tile, int row, string* errorMessage) {
    bool valid = true;
    if (row != 6) {
        // checks if the first filled slot in the storage row is filled by the same tile or empty
        Tile ch = storageRow[row-1]->at(0);
        if (ch == tile) {
            // check if storage is full 
            valid = false;
            for (int i = 0; i < row; ++i) {
                if (storageRow[row-1]->at(i) == EMPTY_SLOT) {
                    valid = true;
                    // stop the loop
                    i = row;
                }
            }
            if (!valid) {
                *errorMessage += "Row " + std::to_string(row) + " is already full\n";
            }
        } else if (ch == EMPTY_SLOT) {
            // check if corresponding mosaic is already filled
            for (int i = 0; i < MOSAIC_DIM; ++i) {
                if (wall[row-1][i] == tile) {
                    valid = false;
                    *errorMessage += "Tile " + std::string(1, tile) + " already filled in row " + 
                                        std::to_string(row) + " of Mosaic\n";
                    // stop the loop
                    i = MOSAIC_DIM;
                }
            }
        } else {
            valid = false;
            *errorMessage += "Storage Row " + std::to_string(row) + 
                        " already holds tiles.\nYou may only add tiles of the same color to it\n";
        }
    }
    return valid;
}

int Player::addToStorageRow(int row, Tile tile, int numTilesToAdd) {
    if (row != 6) {
        for (int i = 0; i < row && numTilesToAdd > 0; ++i) {
            if (storageRow[row-1]->at(i) == EMPTY_SLOT) {
                storageRow[row-1]->at(i) = tile;
                --numTilesToAdd;
            }
        }
    }
    return addToFloorLine(tile, numTilesToAdd);
}

int Player::addToFloorLine(Tile tile, int numTilesToAdd) {
    while (numTilesToAdd > 0 && floorLine->getSize() < FLOOR_LINE_MAX_SIZE) {
        floorLine->addBack(tile);
        --numTilesToAdd;
    }
    return numTilesToAdd;
}

void Player::addToWall(int row, int col, Tile tile) {
    wall[row][col] = tile;
}

int Player::updateScore(Mosaic defaultMosaic, LinkedList* tilebag) {
    // Go through pattern lines from top to bottom   
    int pointsToAdd = 0;
    Tile temp;
    for (unsigned int i = 0; i < MOSAIC_DIM; ++i) {
        // check if a row is completed
        temp = storageRow[i]->at(i);
        if (temp != EMPTY_SLOT) {
            ++pointsToAdd;
            // For each complete line add a tile of the same color in the corresponding line of the wall
            for (int j = 0; j < MOSAIC_DIM; ++j) {
                if (temp == defaultMosaic[i][j]) {
                    wall[i][j] = temp;

                    // add all tiles from any pattern lines that now have no tile in the rightmost space to tilebag
                    for (unsigned int k = 0; k < i; ++k) {
                        tilebag->addBack(temp);
                    }
                    for (unsigned int k = 0; k <= i; ++k) {
                        storageRow[i]->at(k) = EMPTY_SLOT;
                    }
                                        
                    // Each time a tile is added to the wall, score points immediately (as per official rules)
                    // check for vertically adjacent tiles
                    for (int k = i-1; k >= 0; --k) {
                        if (wall[k][j] != EMPTY_SLOT) {
                            ++pointsToAdd;
                        } else {
                            // stops the loop
                            k = -1;
                        }
                    }
                    for (int k = i+1; k < MOSAIC_DIM; ++k) {
                        if (wall[k][j] != EMPTY_SLOT) {
                            ++pointsToAdd;
                        } else {
                            // stops the loop
                            k = MOSAIC_DIM;
                        }
                    }

                    // check for horizontally adjacent tiles
                    for (int k = j-1; k >= 0; --k) {
                        if (wall[i][k] != EMPTY_SLOT) {
                            ++pointsToAdd;
                        } else {
                            // stops the loop
                            k = -1;
                        }
                    }

                    for (int k = j+1; k < MOSAIC_DIM; ++k) {
                        if (wall[i][k] != EMPTY_SLOT) {
                            ++pointsToAdd;
                        } else {
                            // stops the loop
                            k = MOSAIC_DIM;
                        }
                    }

                    // stops the loop
                    j = MOSAIC_DIM;
                }
            }
        } // if a row is completed
    }
    // decrease points for tiles in floor line (as per official azul rules)
    int floorLineSize = floorLine->getSize();
    if (floorLineSize >= 0 && floorLineSize < 3) {
        pointsToAdd -= floorLineSize;
    } else if (floorLineSize >= 3 && floorLineSize < 6) {
        pointsToAdd -= (2*floorLineSize - 2);
    } else if (floorLineSize == 6) {
        pointsToAdd -= 11;
    } else {
        pointsToAdd -= 14;
    } // for 7 (or more) tiles

    this->points += pointsToAdd;
    if (this->points < 0) {
        // player score cannot be less than zero
        this->points = 0;
    }

    return pointsToAdd;
}

bool Player::resetFloorline(LinkedList* tilebag) {
    bool result = false;
    Tile temp;
    while (floorLine->getSize() > 0) {
        temp = floorLine->removeFront();
        if (temp == FIRST_PLAYER_TILE) {
            result = true;
        } else {
            tilebag->addBack(temp);
        }
    }
    return result;
}
