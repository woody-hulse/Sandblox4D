#include "terrain4.h"

#include "glm/gtx/string_cast.hpp"

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <random>
#include <algorithm>

Terrain4::Terrain4() : Terrain()
{
    m_vertexData = std::vector<float>();

    terrain4 = new uint8_t***[sizeX];
    terrain_p = new uint8_t***[sizeX];
    rendered4 = new bool***[sizeX];
    for (int x = 0; x < sizeX; x++) {
        terrain4[x] = new uint8_t**[sizeY];
        terrain_p[x] = new uint8_t**[sizeY];
        rendered4[x] = new bool**[sizeY];
        for (int y = 0; y < sizeY; y++) {
            terrain4[x][y] = new uint8_t*[sizeZ];
            terrain_p[x][y] = new uint8_t*[sizeZ];
            rendered4[x][y] = new bool*[sizeZ];
            for (int z = 0; z < sizeZ; z++) {
                terrain4[x][y][z] = new uint8_t[sizeW];
                rendered4[x][y][z] = new bool[sizeW];
                for (int w = 0; w < sizeW; w++) {
                    terrain4[x][y][z][w] = 0;
                    rendered4[x][y][z][w] = false;
                }
                terrain_p[x][y][z] = &terrain4[x][y][z][0];
            }
        }
    }

    crossSection.origin = glm::vec4(sizeX / 2.f, sizeW / 2.f, 0.f, 1.f);
    crossSection.direction = glm::normalize(glm::vec4(0.f, 1.f, 0.f, 0.f));
}


std::vector<std::vector<std::vector<float>>> Terrain4::generateHeightMap(float scale) {
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);

    std::vector<std::vector<std::vector<glm::vec3>>> gradients(sizeX + 1,
                                                               std::vector<std::vector<glm::vec3>>(sizeY + 1,
                                                               std::vector<glm::vec3>(sizeZ + 1)));
    for (int i = 0; i <= sizeX; i++) {
        for (int j = 0; j <= sizeY; j++) {
            for (int k = 0; k <= sizeW; k++) {
                gradients[i][j][k] = glm::normalize(glm::vec3(distribution(generator), distribution(generator), distribution(generator)));
            }
        }
    }

    std::vector<std::vector<std::vector<float>>> noise(sizeX,
                                                       std::vector<std::vector<float>>(sizeY,
                                                       std::vector<float>(sizeW)));
    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeW; k++) {
                float x = static_cast<float>(i) / sizeX * scale;
                float y = static_cast<float>(j) / sizeY * scale;
                float z = static_cast<float>(k) / sizeW * scale;

                int x0 = static_cast<int>(x);
                int x1 = x0 + 1;
                int y0 = static_cast<int>(y);
                int y1 = y0 + 1;
                int z0 = static_cast<int>(z);
                int z1 = z0 + 1;

                float u = x - x0;
                float v = y - y0;
                float w = z - z0;

                float x00 = glm::mix(glm::dot(gradients[x0][y0][z0], glm::vec3(u, v, w)),
                                     glm::dot(gradients[x1][y0][z0], glm::vec3(u - 1.0f, v, w)), u);
                float x10 = glm::mix(glm::dot(gradients[x0][y1][z0], glm::vec3(u, v - 1.0f, w)),
                                     glm::dot(gradients[x1][y1][z0], glm::vec3(u - 1.0f, v - 1.0f, w)), u);
                float x01 = glm::mix(glm::dot(gradients[x0][y0][z1], glm::vec3(u, v, w - 1.0f)),
                                     glm::dot(gradients[x1][y0][z1], glm::vec3(u - 1.0f, v, w - 1.0f)), u);
                float x11 = glm::mix(glm::dot(gradients[x0][y1][z1], glm::vec3(u, v - 1.0f, w - 1.0f)),
                                     glm::dot(gradients[x1][y1][z1], glm::vec3(u - 1.0f, v - 1.0f, w - 1.0f)), u);

                float a0 = glm::mix(x00, x10, v);
                float a1 = glm::mix(x01, x11, v);

                noise[i][j][k] = glm::mix(a0, a1, w);
            }
        }
    }
    return noise;
}

void Terrain4::generateTerrain4() {
    int numLayers = 3;
    float baseScale = 1.0f;

    for (int layer = 0; layer < numLayers; layer++) {
        float scale = baseScale * std::pow(2, layer);
        heightMap4 = generateHeightMap(scale);

        for (int x = 0; x < sizeX; x++) {
            for (int y = 0; y < sizeY; y++) {
                for (int z = 0; z < sizeZ; z++) {
                    for (int w = 0; w < sizeW; w++) {
                        if (z < (heightMap4[x][y][z] + 0.6) * sizeZ)
                            terrain4[x][y][z][w] = 1;
                    }
                }
            }
        }
    }
}


std::vector<IntersectData> Terrain4::terrainRayIntersect() {
    std::vector<IntersectData> intersections;

    float x = crossSection.origin.x;
    float y = crossSection.origin.y;

    glm::vec2 pos(x, y);

    float dx = crossSection.direction.x;
    float dy = crossSection.direction.y;

    float epsilon = 0.01f;
    float step = fmin(1 / (abs(dx) + epsilon), 1 / (abs(dy) + epsilon));

    glm::vec2 dir = glm::vec2(dx, dy) * step;

    while (0 <= pos.x && int(pos.x) < sizeX && 0 <= pos.y && int(pos.y) < sizeW) {
        IntersectData data;
        data.x = int(pos.x);
        data.y = int(pos.y);
        intersections.push_back(data);

        pos += dir;
    }

    return intersections;
}

void Terrain4::rotateCrossSection(float theta, float t) {
    float newX = crossSection.direction.x * cos(theta) - crossSection.direction.y * sin(theta);
    float newY = crossSection.direction.x * sin(theta) + crossSection.direction.y * cos(theta);
    crossSection.direction = glm::vec4(newX, newY, 0.f, 0.f);

    generateTerrain();
}


void Terrain4::breakBlock(IntersectData& intersectData) {
    terrain[intersectData.x][intersectData.y][intersectData.z] = 0;
    *terrain_p[intersectData.x][intersectData.y][intersectData.z] = 0;
    generateTerrainMesh();
}

void Terrain4::placeBlock(IntersectData& intersectData) {

    if (intersectData.face == 0) intersectData.x += 1;
    if (intersectData.face == 1) intersectData.x -= 1;
    if (intersectData.face == 2) intersectData.z += 1;
    if (intersectData.face == 3) intersectData.z -= 1;
    if (intersectData.face == 4) intersectData.y += 1;
    if (intersectData.face == 5) intersectData.y -= 1;

    if (intersectData.x >= 0 && intersectData.x < sizeX) {
        if (intersectData.y >= 0 && intersectData.y < sizeY) {
            if (intersectData.z >= 0 && intersectData.z < sizeZ) {
                terrain[intersectData.x][intersectData.y][intersectData.z] = 1;
                *terrain_p[intersectData.x][intersectData.y][intersectData.z] = 1;
                generateTerrainMesh();
            }
        }
    }
}


void Terrain4::generateTerrain() {
    std::vector<IntersectData> pos = terrainRayIntersect();
    crossSection.direction = -crossSection.direction;

    std::vector<IntersectData> neg = terrainRayIntersect();
    neg.pop_back();
    crossSection.direction = -crossSection.direction;

    int cellX = int(crossSection.origin.x);
    int cellY = int(crossSection.origin.y);

    float heightMap[sizeX][sizeY] = {{0}};

    IntersectData pData;
    pData.x = -1;
    pData.y = -1;
    int i = 0;
    int cellIndex = 0;
    int direction = 0;
    while (i < neg.size()) {
        // std::cout << neg[i].x << " " << neg[i].y << std::endl;
        if (pData.x == neg[i].x || pData.y == neg[i].y) {
        }

        pData = neg[i];
        i++;
    }

    pData.x = -1;
    pData.y = -1;
    i = 0;
    while (i < pos.size()) {
        // std::cout << pos[i].x << " " << pos[i].y << std::endl;
        if (pData.x == pos[i].x || pData.y == pos[i].y) {

        }

        pData = pos[i];
        i++;
    }

    // std::cout << glm::to_string(crossSection.direction) <<  std::endl;

    // neg.insert(neg.end(), pos.begin(), pos.end());

    for (int i = cellX; i >= 0; i--) {
        for (int j = 0; j < sizeY; j++) {
            // std::cout << neg.size() << " " << neg[i].x << " " << neg[i].y << " " << i << " " << j << " " << heightMap4[neg[i].x][neg[i].y][j] << std::endl;
            for (int k = 0; k < sizeZ; k++) {
                if (cellX - i < neg.size()) {
                    terrain[i][j][k] = terrain4[j][neg[cellX - i].x][k][neg[cellX - i].y]; // fix?
                    terrain_p[i][j][k] = &terrain4[j][neg[cellX - i].x][k][neg[cellX - i].y];
                } else terrain[i][j][k] = 0; // fix
            }
        }
    }

    // std::cout << "a " << cellX << " " << sizeX << " " << cellY << " " << sizeY << std::endl;

    for (int i = cellX + 1; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeZ; k++) {
                if (i - cellX - 1 < pos.size()) {
                    terrain[i][j][k] = terrain4[j][pos[i - cellX - 1].x][k][pos[i - cellX - 1].y];
                    terrain_p[i][j][k] = &terrain4[j][pos[i - cellX - 1].x][k][pos[i - cellX - 1].y];
                } else terrain[i][j][k] = 0;
            }
        }
    }

    // sstd::cout << "test " << glm::to_string(crossSection.origin) << std::endl;

    // generateTerrainFromHeightMap(heightMap);
}



