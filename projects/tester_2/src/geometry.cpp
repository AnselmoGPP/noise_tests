
#include <iostream>
#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry.hpp"

// terrainData -----------------------------------------------------------------

terrainData::terrainData()
{
    newConfig = true;

    dimensions[0] = 128;
    dimensions[1] = 128;

    noiseType = FastNoiseLite::NoiseType_Perlin;
    octaves = 6;
    lacunarity = 1.5;
    persistance = 0.5;
    scale = 1.0f;
    multiplier = 30;
    seed = 0;
    offset[0] = 0.0f;
    offset[1] = 0.0f;
}

terrainData::terrainData(const terrainData& obj)
{
    newConfig = obj.newConfig;

    dimensions[0] = obj.dimensions[0];
    dimensions[1] = obj.dimensions[1];

    noiseType = obj.noiseType;
    octaves = obj.octaves;
    lacunarity = obj.lacunarity;
    persistance = obj.persistance;
    scale = obj.scale;
    multiplier = obj.multiplier;
    seed = obj.seed;
    offset[0] = obj.offset[0];
    offset[1] = obj.offset[1];
}

bool terrainData::operator!= (terrainData& obj)
{
    if (
        obj.dimensions[0] != dimensions[0] ||
        obj.dimensions[1] != dimensions[1] ||

        obj.noiseType != noiseType ||
        obj.octaves != octaves ||
        obj.lacunarity != lacunarity ||
        obj.persistance != persistance ||
        obj.scale != scale ||
        obj.multiplier != multiplier ||
        obj.seed != seed ||
        obj.offset[0] != offset[0] ||
        obj.offset[1] != offset[1]
        )
        return true;
    else return false;
}

std::ostream& operator << (std::ostream& os, const terrainData& obj)
{
    const char* noiseTypeString[6] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };

    os << "----------------------------" << std::endl;
    os << "newConfig: " << obj.newConfig << std::endl;
    os << "Dimensions (x,y): " << obj.dimensions[0] << ", " << obj.dimensions[1] << std::endl;
    os << "Noise type: " << noiseTypeString[obj.noiseType] << std::endl;
    os << "Octaves: " << obj.octaves << std::endl;
    os << "Lacunarity: " << obj.lacunarity << "      Lacunarity ^ Octaves: " << std::pow(obj.lacunarity, (float)(obj.octaves - 1)) << std::endl;
    os << "Persistance: " << obj.persistance << "      Persistance ^ Octaves: " << std::pow(obj.persistance, (float)(obj.octaves - 1)) << std::endl;
    os << "Scale: " << obj.scale << std::endl;
    os << "Multiplier: " << obj.multiplier << std::endl;
    os << "Seed: " << obj.seed << std::endl;
    os << "Offset (x, y): " << obj.offset[0] << ", " << obj.offset[1] << std::endl;

    return os;
}

// noiseSet -----------------------------------------------------------------

noiseSet::noiseSet(terrainData& td, bool addRandomOffset)
    : noiseType(td.noiseType), numOctaves(td.octaves), lacunarity(td.lacunarity), persistance(td.persistance), scale(td.scale), multiplier(td.multiplier)
{
    // unsigned int NumOctaves, float Lacunarity, float Persistance, float Scale, float Multiplier, unsigned int Seed, float (*Offset)[2], FastNoiseLite::NoiseType NoiseType
    // td.octaves, td.lacunarity, td.persistance, td.scale, td.multiplier, td.seed, nullptr, td.noiseType[2]

    if(scale <= 0) scale = 0.001f;

    // set noise type
    noise.SetNoiseType(noiseType);

    // set offsets for each octave
    std::mt19937_64 engine;
    engine.seed(td.seed);
    std::uniform_int_distribution<int> distribution(-10000, 10000);

    octaveOffsets = std::vector<float[2]>(numOctaves);
    for(size_t i = 0; i < numOctaves; i++)
    {
        if(addRandomOffset)
        {
            octaveOffsets[i][x] = distribution(engine) + td.offset[x];
            octaveOffsets[i][y] = distribution(engine) + td.offset[y];
        }
        else {
            octaveOffsets[i][x] = td.offset[x];
            octaveOffsets[i][y] = td.offset[y];
        }

    }

    // get extreme value
    extreme = 0;
    float maxmin = std::sqrt((float)1/9);
    float amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        extreme += maxmin * amplitude;
        amplitude *= persistance;
    }

    extreme *= scale * multiplier;
}

float noiseSet::GetNoise(float X, float Y, bool includeOffset)
{
    float result = 0;
    float frequency = 1, amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        if (includeOffset)
        {
            X = X / scale * frequency + octaveOffsets[i][x];
            Y = Y / scale * frequency + octaveOffsets[i][y];
        }
        else
        { 
            X = X / scale * frequency;
            Y = Y / scale * frequency;
        }

        result += noise.GetNoise(X, Y) * amplitude;

        frequency *= lacunarity;
        amplitude *= persistance;
    }

    return result * scale * multiplier;
}

float noiseSet::GetExtreme() { return extreme; };

// terrainGenerator -----------------------------------------------------------------

terrainGenerator::terrainGenerator()
{
    terrainData td;
    computeTerrain(td);
}

terrainGenerator::~terrainGenerator()
{
    delete[] field;
    delete[] indices;
}

void terrainGenerator::computeTerrain(terrainData &td)
{
    noiseSet noise(td, false);

    size_t siz[2] = { td.dimensions[0], td.dimensions[1] };

    if (siz[0] != side[0] || siz[0] != side[1])
    {
        side[0] = siz[0];
        side[1] = siz[1];
        totalVert = side[0] * side[1];
        totalVertUsed = (side[0] - 1) * (side[1] - 1) * 2 * 3;

        delete[] field;
        field = new float[totalVert][11];
        delete[] indices;
        indices = new unsigned int[totalVertUsed / 3][3];
    }

    float textureFactor = 0.1;

    // Vertex data
    for (size_t y = 0; y < side[1]; y++)
        for (size_t x = 0; x < side[0]; x++)
        {
            size_t pos = getPos(x, y);

            // positions
            field[pos][0] = x;
            field[pos][1] = y;
            field[pos][2] = noise.GetNoise((float)x, (float)y, true);

            // colors
            field[pos][3] = 0.5f;
            field[pos][4] = 0.1f;
            field[pos][5] = 0.2f;

            // textures
            field[pos][6] = x * textureFactor;
            field[pos][7] = y * textureFactor;

            // normals
            //field[pos][8] = 0.0f;
            //field[pos][9] = 0.0f;
            //field[pos][10]= 0.0f;
        }

    // Normals
    glm::vec3* tempNormals = new glm::vec3[totalVert];  // Initialize normals to 0
    for (size_t i = 0; i < totalVert; i++)
        tempNormals[i] = glm::vec3(0.f, 0.f, 0.f);

    for (size_t y = 0; y < side[1] - 1; y++)            // Compute normals
        for (size_t x = 0; x < side[0] - 1; x++)
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
            size_t B = getPos(x + 1, y);
            size_t C = getPos(x + 1, y + 1);
            size_t D = getPos(x, y + 1);

            glm::vec3 Aside = glm::vec3(field[B][0], field[B][1], field[B][2]) - glm::vec3(field[A][0], field[A][1], field[A][2]);
            glm::vec3 Bside = glm::vec3(field[C][0], field[C][1], field[C][2]) - glm::vec3(field[B][0], field[B][1], field[B][2]);
            glm::vec3 Cside = glm::vec3(field[C][0], field[C][1], field[C][2]) - glm::vec3(field[D][0], field[D][1], field[D][2]);
            glm::vec3 Dside = glm::vec3(field[D][0], field[D][1], field[D][2]) - glm::vec3(field[A][0], field[A][1], field[A][2]);

            glm::vec3 Anormal = glm::cross(Aside, Dside);
            glm::vec3 Bnormal = glm::cross(Bside, -Aside);
            glm::vec3 Cnormal = glm::cross(Cside, Bside);
            glm::vec3 Dnormal = glm::cross(Dside, -Cside);

            tempNormals[A] += Anormal;
            tempNormals[B] += Bnormal;
            tempNormals[C] += Cnormal;
            tempNormals[D] += Dnormal;
        }

    for (int i = 0; i < totalVert; i++)                 // Normalize the normals
    {
        tempNormals[i] = glm::normalize(tempNormals[i]);

        field[i][8] = tempNormals[i].x;
        field[i][9] = tempNormals[i].y;
        field[i][10] = tempNormals[i].z;
    }
    delete[] tempNormals;

    // Indices
    size_t index = 0;

    for (size_t y = 0; y < side[1] - 1; y++)
        for (size_t x = 0; x < side[0] - 1; x++)
        {
            unsigned int pos = getPos(x, y);

            indices[index][0] = pos;
            indices[index][1] = pos + side[0] + 1;
            indices[index++][2] = pos + side[0];
            indices[index][0] = pos;
            indices[index][1] = pos + 1;
            indices[index++][2] = pos + side[0] + 1;
        }
}

void terrainGenerator::printData()
{
    std::cout << "Side: " << side[0] << " x " << side[1] << std::endl;
    std::cout << "Drawn vertices: " << totalVertUsed << std::endl;

    std::cout << "Vertices: " << std::endl;
    for(size_t y = side[1]-1; y < side[1]; y--)
    {
        for(size_t x = 0; x < side[0]; x++)
            std::cout << "(" << field[getPos(x, y)][0] << ", " << field[getPos(x, y)][1] << ") ";
        std::cout << std::endl;
    }

    std::cout << "Indices: " << std::endl;
    for(size_t i = 0; i < totalVertUsed/3; i++)
        std::cout << indices[i][0] << ", " << indices[i][1] << ", " << indices[i][2] << std::endl;
}

size_t terrainGenerator::getPos(size_t x, size_t y) { return y * side[0] + x; }
