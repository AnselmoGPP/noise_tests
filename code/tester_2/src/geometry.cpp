
#include <iostream>

#include "FastNoiseLite.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry.hpp"

// noiseSet -----------------------------------------------------------------

noiseSet::noiseSet(size_t NumOctaves, float Lacunarity, float Persistance, float Scale, float Multiplier, unsigned int Seed, float (*Offset)[2], FastNoiseLite::NoiseType NoiseType)
    : noiseType(NoiseType), numOctaves(NumOctaves), lacunarity(Lacunarity), persistance(Persistance), scale(Scale), multiplier(Multiplier)
{
    if(scale <= 0) scale = 0.001f;

    // set noise type
    noise.SetNoiseType(noiseType);

    // set offsets for each octave
    std::mt19937_64 engine;
    engine.seed(Seed);
    std::uniform_int_distribution<int> distribution(-100000, 100000);

    octaveOffsets = std::vector<float[2]>(numOctaves);
    for(size_t i = 0; i < numOctaves; i++)
    {
        if(Offset)
        {
            octaveOffsets[i][x] = distribution(engine) + Offset[i][x];
            octaveOffsets[i][y] = distribution(engine) + Offset[i][y];
        }
        else
        {
            octaveOffsets[i][x] = distribution(engine);
            octaveOffsets[i][y] = distribution(engine);
        }
    }

    // get extreme value
    float amplitude = 0;
    for(int i = 0; i < numOctaves; i++)
    {
        extreme += sqrt(1/9) * amplitude;
        amplitude *= persistance;
    }
    extreme *= scale * multiplier;
}

float noiseSet::GetNoise(float X, float Y)
{
    float result = 0;
    float frequency = 1, amplitude = 1;

    for(size_t i = 0; i < numOctaves; i++)
    {
        X = frequency * X/scale + octaveOffsets[i][x];
        Y = frequency * Y/scale + octaveOffsets[i][y];
        result += noise.GetNoise(X, Y) * amplitude;

        frequency *= lacunarity;
        amplitude *= persistance;
    }

    return result * scale * multiplier;
}

float noiseSet::GetExtreme() { return extreme * multiplier * scale; };

// terrain -----------------------------------------------------------------

void terrain::fillArray()
{
    noiseSet noise(6, 1.3f, 0.4f, 1.f, 40 /*, 0, nullptr, FastNoiseLite::NoiseType_Perlin*/);

    float textFactor = 0.1;

    // Vertex data
    for(size_t y = 0; y < side; y++)
        for(size_t x = 0; x < side; x++)
        {
            size_t pos = getPos(x, y);

            // positions
            field[pos][0] = x;
            field[pos][1] = y;
            field[pos][2] = noise.GetNoise((float)x, (float)y);

            // colors
            field[pos][3] = 0.5f;
            field[pos][4] = 0.1f;
            field[pos][5] = 0.2f;

            // textures
            field[pos][6] = x * textFactor;
            field[pos][7] = y * textFactor;

            // normals
            //field[pos][8] = 0.0f;
            //field[pos][9] = 0.0f;
            //field[pos][10]= 0.0f;
        }

    // Normals
    glm::vec3 *tempNormals = new glm::vec3[totalVert];
    for(size_t i = 0; i < totalVert; i++)
        tempNormals[i] = glm::vec3(0.f, 0.f, 0.f);

    for(size_t y = 0; y < side-1; y++)
        for(size_t x = 0; x < side-1; x++)
        {
            /*
                           Cside
                       (D)-------(C)
                        |         |
                  Dside |         | Bside
                        |         |
                       (A)-------(B)
                           Aside
             */

            size_t A = getPos(x, y);
            size_t B = getPos(x+1, y);
            size_t C = getPos(x+1, y+1);
            size_t D = getPos(x, y+1);

            glm::vec3 Aside = glm::vec3(field[B][0], field[B][1], field[B][2]) - glm::vec3(field[A][0], field[A][1], field[A][2]);
            glm::vec3 Bside = glm::vec3(field[C][0], field[C][1], field[C][2]) - glm::vec3(field[B][0], field[B][1], field[B][2]);
            glm::vec3 Cside = glm::vec3(field[C][0], field[C][1], field[C][2]) - glm::vec3(field[D][0], field[D][1], field[D][2]);
            glm::vec3 Dside = glm::vec3(field[D][0], field[D][1], field[D][2]) - glm::vec3(field[A][0], field[A][1], field[A][2]);

            glm::vec3 Anormal = glm::cross(Aside,  Dside);
            glm::vec3 Bnormal = glm::cross(Bside, -Aside);
            glm::vec3 Cnormal = glm::cross(Cside,  Bside);
            glm::vec3 Dnormal = glm::cross(Dside, -Cside);

            tempNormals[A] += Anormal;
            tempNormals[B] += Bnormal;
            tempNormals[C] += Cnormal;
            tempNormals[D] += Dnormal;
        }

    for(int i = 0; i < totalVert; i++)
    {
        tempNormals[i] = glm::normalize(tempNormals[i]);

        field[i][8] = tempNormals[i].x;
        field[i][9] = tempNormals[i].y;
        field[i][10]= tempNormals[i].z;
    }

    delete[] tempNormals;

    // Indices
    size_t index = 0;

    for(size_t y = 0; y < side-1; y++)
        for(size_t x = 0; x < side-1; x++)
        {
            unsigned int pos = y * side + x;

            indices[index  ][0] = pos;
            indices[index  ][1] = pos + side + 1;
            indices[index++][2] = pos + side;
            indices[index  ][0] = pos;
            indices[index  ][1] = pos + 1;
            indices[index++][2] = pos + side + 1;
        }
}

terrain::terrain(size_t sideSize) :
    side(sideSize), totalVert(side * side), totalVertUsed((side-1)*(side-1)*2*3)
{
    field = new float[totalVert][11];
    indices = new unsigned int[totalVertUsed/3][3];
    fillArray();
}

terrain::~terrain()
{
    delete[] field;
    delete[] indices;
}

void terrain::printData()
{
    std::cout << "Side: " << side << std::endl;
    std::cout << "Drawn vertices: " << totalVertUsed << std::endl;

    std::cout << "Vertices: " << std::endl;
    for(size_t y = side-1; y < side; y--)
    {
        for(size_t x = 0; x < side; x++)
            std::cout << "(" << field[y*side+x][0] << ", " << field[y*side+x][1] << ") ";
        std::cout << std::endl;
    }

    std::cout << "Indices: " << std::endl;
    for(size_t i = 0; i < totalVertUsed/3; i++)
        std::cout << indices[i][0] << ", " << indices[i][1] << ", " << indices[i][2] << std::endl;
}

size_t terrain::getPos(size_t x, size_t y) { return y * side + x; }
