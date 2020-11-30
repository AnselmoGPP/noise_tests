
#include <iostream>
#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry.hpp"

// noiseSet -----------------------------------------------------------------

noiseSet::noiseSet(
                    unsigned int NumOctaves,
                    float Lacunarity,
                    float Persistance,
                    float Scale,
                    float Multiplier,
                    unsigned CurveDegree,
                    float OffsetX,
                    float OffsetY,
                    FastNoiseLite::NoiseType NoiseType,
                    bool addRandomOffset,
                    unsigned int Seed)
{
    // Set values
    noiseType       = NoiseType;
    numOctaves      = NumOctaves;
    lacunarity      = Lacunarity;
    persistance     = Persistance;
    scale           = Scale;
    multiplier      = Multiplier;
    curveDegree     = CurveDegree;
    offsetX         = OffsetX;
    offsetY         = OffsetY;
    seed            = Seed;
    octaveOffsets   = new float[numOctaves][2];


    // Clamp values
    if(lacunarity < 1) lacunarity  = 1;
    if(persistance < 0) persistance = 0;
    else
        if (persistance > 1) persistance = 1;
    if(scale <= 0) scale = 0.01f;

    // Set noise type
    noise.SetNoiseType(noiseType);

    // Set offsets for each octave
    if(addRandomOffset)
    {
        std::mt19937_64 engine;
        engine.seed(seed);
        std::uniform_int_distribution<int> distribution(-10000, 10000);

        for(size_t i = 0; i < numOctaves; i++)
        {
            octaveOffsets[i][0] = distribution(engine) + offsetX;
            octaveOffsets[i][1] = distribution(engine) + offsetY;
        }
    }
    else
    {
        for(size_t i = 0; i < numOctaves; i++)
        {
            octaveOffsets[i][0] = offsetX;
            octaveOffsets[i][1] = offsetY;
        }
    }

    // Get maximum noise
    maxHeight = 0;
    float amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        maxHeight += 1 * amplitude;
        amplitude *= persistance;
    }

    maxHeight *= scale * multiplier;
}

noiseSet::~noiseSet()
{
    delete[] octaveOffsets;
}

noiseSet::noiseSet(const noiseSet& obj)
{
    noise       = obj.noise;
    noiseType   = obj.noiseType;
    numOctaves  = obj.numOctaves;
    lacunarity  = obj.lacunarity;
    persistance = obj.persistance;
    scale       = obj.scale;
    multiplier  = obj.multiplier;
    curveDegree = obj.curveDegree;
    offsetX     = obj.offsetX;
    offsetY     = obj.offsetY;
    seed        = obj.seed;
    
    maxHeight     = obj.maxHeight;

    delete[] octaveOffsets;
    for(size_t i = 0; i < numOctaves; i++)
    {
        octaveOffsets[i][0] = obj.octaveOffsets[i][0];
        octaveOffsets[i][1] = obj.octaveOffsets[i][1];
    }
}

bool noiseSet::operator != (noiseSet& obj)
{
    if( noiseType   != obj.noiseType ||
        numOctaves  != obj.numOctaves ||
        lacunarity  != obj.lacunarity ||
        persistance != obj.persistance ||
        scale       != obj.scale ||
        multiplier  != obj.multiplier ||
        curveDegree != obj.curveDegree ||
        offsetX     != obj.offsetX ||
        offsetY     != obj.offsetY ||
        seed        != obj.seed )
        return true;

    return false;
}

std::ostream& operator << (std::ostream& os, const noiseSet& obj)
{
    os << "\n----------"
       << "\nNoise type: "   << obj.noiseType
       << "\nNum. octaves: " << obj.numOctaves
       << "\nLacunarity: "   << obj.lacunarity
       << "\nPersistance: "  << obj.persistance
       << "\nScale: "        << obj.scale
       << "\nMultiplier: "   << obj.multiplier
       << "\nCurve degree: " << obj.curveDegree
       << "\nOffset X: "     << obj.offsetX
       << "\nOffset Y: "     << obj.offsetY
       << "\nSeed: "         << obj.seed
       << "\nMax. height: "  << obj.maxHeight
       << std::endl;

    return os;
}

float noiseSet::GetNoise(float X, float Y)
{
    float result = 0;
    float frequency = 1, amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        X = (X / scale) * frequency + octaveOffsets[i][0];
        Y = (Y / scale) * frequency + octaveOffsets[i][1];

        result += ((1 + noise.GetNoise(X, Y)) / 2) * amplitude;   // noise.GetNoise() returns in range [-1, 1]. We convert it to [0, 1]

        frequency *= lacunarity;
        amplitude *= persistance;
    }

    float curveFactor = std::pow(result, curveDegree);

    return result * curveFactor * scale * multiplier;
}

float                 noiseSet::getMaxHeight() const { return maxHeight; };

unsigned              noiseSet::getNoiseType()      const { return noiseType; }
unsigned              noiseSet::getNumOctaves()     const { return numOctaves; }
float                 noiseSet::getLacunarity()     const { return lacunarity; }
float                 noiseSet::getPersistance()    const { return persistance; }
float                 noiseSet::getScale()          const { return scale; }
float                 noiseSet::getMultiplier()     const { return multiplier; }
float                 noiseSet::getCurveDegree()    const { return curveDegree; }
float                 noiseSet::getOffsetX()        const { return offsetX; }
float                 noiseSet::getOffsetY()        const { return offsetY; }
unsigned int          noiseSet::getSeed()           const { return seed; }
float*                noiseSet::getOffsets()        const { return &octaveOffsets[0][0]; }

void noiseSet::noiseTester(size_t size)
{
    float max = 0, min = 0;
    float noise;

    std::cout << "Size: " << size << std::endl;

    for(size_t i = 1; i != size; i++)
    {
        std::cout << "%: " << 100 * (float)i/size << "     \r";
        for(size_t j = 1; j != size; j++)
        {
            noise = GetNoise(i, j);
            if      (noise > max) max = noise;
            else if (noise < min) min = noise;
        }
    }
}

// terrainGenerator -----------------------------------------------------------------

terrainGenerator::terrainGenerator(noiseSet &noise, float x0, float y0, float stride, unsigned numVertex_X, unsigned numVertex_Y)
{
    computeTerrain(noise, x0, y0, stride, numVertex_X, numVertex_Y);
}

terrainGenerator::~terrainGenerator()
{
    delete[] vertex;
    delete[] indices;
}

void terrainGenerator::computeTerrain(noiseSet &noise, float x0, float y0, float stride, unsigned numVertex_X, unsigned numVertex_Y, float textureFactor)
{
    if (numVertexX != numVertex_X || numVertexY != numVertex_Y)
    {
        numVertexX = numVertex_X;
        numVertexY = numVertex_Y;
        numVertex  = numVertexX * numVertexY;
        numIndices = (numVertexX - 1) * (numVertexY - 1) * 2 * 3;

        delete[] vertex;
        vertex = new float[numVertex][11];
        delete[] indices;
        indices = new unsigned int[numIndices/3][3];
    }

    // Vertex data
    for (size_t y = 0; y < numVertexY; y++)
        for (size_t x = 0; x < numVertexX; x++)
        {
            size_t pos = getPos(x, y);

            // positions
            vertex[pos][0] = x0 + x * stride;
            vertex[pos][1] = y0 + y * stride;
            vertex[pos][2] = noise.GetNoise((float)vertex[pos][0], (float)vertex[pos][1]);

            // colors
            vertex[pos][3] = 0.5f;
            vertex[pos][4] = 0.1f;
            vertex[pos][5] = 0.2f;

            // textures
            vertex[pos][6] = x * textureFactor;
            vertex[pos][7] = y * textureFactor;

            // normals
            //field[pos][8] = 0.0f;
            //field[pos][9] = 0.0f;
            //field[pos][10]= 0.0f;
        }

    // Normals
    glm::vec3* tempNormals = new glm::vec3[numVertex];      // Initialize normals to 0
    for (size_t i = 0; i < numVertex; i++)
        tempNormals[i] = glm::vec3(0.f, 0.f, 0.f);

    for (size_t y = 0; y < numVertexY - 1; y++)            // Compute normals
        for (size_t x = 0; x < numVertexX - 1; x++)
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

            glm::vec3 Aside = glm::vec3(vertex[B][0], vertex[B][1], vertex[B][2]) - glm::vec3(vertex[A][0], vertex[A][1], vertex[A][2]);
            glm::vec3 Bside = glm::vec3(vertex[C][0], vertex[C][1], vertex[C][2]) - glm::vec3(vertex[B][0], vertex[B][1], vertex[B][2]);
            glm::vec3 Cside = glm::vec3(vertex[C][0], vertex[C][1], vertex[C][2]) - glm::vec3(vertex[D][0], vertex[D][1], vertex[D][2]);
            glm::vec3 Dside = glm::vec3(vertex[D][0], vertex[D][1], vertex[D][2]) - glm::vec3(vertex[A][0], vertex[A][1], vertex[A][2]);

            glm::vec3 Anormal = glm::cross(Aside, Dside);
            glm::vec3 Bnormal = glm::cross(Bside, -Aside);
            glm::vec3 Cnormal = glm::cross(Cside, Bside);
            glm::vec3 Dnormal = glm::cross(Dside, -Cside);

            tempNormals[A] += Anormal;
            tempNormals[B] += Bnormal;
            tempNormals[C] += Cnormal;
            tempNormals[D] += Dnormal;
        }

    for (int i = 0; i < numVertex; i++)                 // Normalize the normals
    {
        tempNormals[i] = glm::normalize(tempNormals[i]);

        vertex[i][8] = tempNormals[i].x;
        vertex[i][9] = tempNormals[i].y;
        vertex[i][10] = tempNormals[i].z;
    }

    delete[] tempNormals;

    // Indices
    size_t index = 0;

    for (size_t y = 0; y < numVertexY - 1; y++)
        for (size_t x = 0; x < numVertexX - 1; x++)
        {
            unsigned int pos = getPos(x, y);

            indices[index][0] = pos;
            indices[index][1] = pos + numVertexX + 1;
            indices[index++][2] = pos + numVertexX;
            indices[index][0] = pos;
            indices[index][1] = pos + 1;
            indices[index++][2] = pos + numVertexX + 1;
        }
}

unsigned terrainGenerator::getXside() const { return numVertexX; }
unsigned terrainGenerator::getYside() const { return numVertexY; }
unsigned terrainGenerator::getNumVertex() const { return numVertex; }
unsigned terrainGenerator::getNumIndices() const { return numIndices; }

size_t terrainGenerator::getPos(size_t x, size_t y) const { return y * numVertexX + x; }

// ----------------------------------------------------------------------------------

void fillAxis(float array[12][3], float sizeOfAxis)
{
    array[0][0] = 0.f;  // First vertex
    array[0][1] = 0.f;
    array[0][2] = 0.f;
    array[1][0] = 1.f;  // Color
    array[1][1] = 0.f;
    array[1][2] = 0.f;

    array[2][0] = sizeOfAxis;
    array[2][1] = 0.f;
    array[2][2] = 0.f;
    array[3][0] = 1.f;
    array[3][1] = 0.f;
    array[3][2] = 0.f;

    array[4][0] = 0.f;
    array[4][1] = 0.f;
    array[4][2] = 0.f;
    array[5][0] = 0.f;
    array[5][1] = 1.f;
    array[5][2] = 0.f;

    array[6][0] = 0.f;
    array[6][1] = sizeOfAxis;
    array[6][2] = 0.f;
    array[7][0] = 0.f;
    array[7][1] = 1.f;
    array[7][2] = 0.f;

    array[8][0] = 0.f;
    array[8][1] = 0.f;
    array[8][2] = 0.f;
    array[9][0] = 0.f;
    array[9][1] = 0.f;
    array[9][2] = 1.f;

    array[10][0] = 0.f;
    array[10][1] = 0.f;
    array[10][2] = sizeOfAxis;
    array[11][0] = 0.f;
    array[11][1] = 0.f;
    array[11][2] = 1.f;
}

void fillSea(float array[6][10], float height, float x0, float y0, float x1, float y1)
{
    float color[4] = { 0.1f, 0.1f, 0.8f, 0.9 };

    array[0][0] = x0;
    array[0][1] = y0;
    array[0][2] = height;
    array[0][3] = color[0];
    array[0][4] = color[1];
    array[0][5] = color[2];
    array[0][6] = color[3];
    array[0][7] = 0;
    array[0][8] = 0;
    array[0][9] = 1;

    array[1][0] = x1;
    array[1][1] = y0;
    array[1][2] = height;
    array[1][3] = color[0];
    array[1][4] = color[1];
    array[1][5] = color[2];
    array[1][6] = color[3];
    array[1][7] = 0;
    array[1][8] = 0;
    array[1][9] = 1;

    array[2][0] = x0;
    array[2][1] = y1;
    array[2][2] = height;
    array[2][3] = color[0];
    array[2][4] = color[1];
    array[2][5] = color[2];
    array[2][6] = color[3];
    array[2][7] = 0;
    array[2][8] = 0;
    array[2][9] = 1;

    array[3][0] = x1;
    array[3][1] = y0;
    array[3][2] = height;
    array[3][3] = color[0];
    array[3][4] = color[1];
    array[3][5] = color[2];
    array[3][6] = color[3];
    array[3][7] = 0;
    array[3][8] = 0;
    array[3][9] = 1;

    array[4][0] = x1;
    array[4][1] = y1;
    array[4][2] = height;
    array[4][3] = color[0];
    array[4][4] = color[1];
    array[4][5] = color[2];
    array[4][6] = color[3];
    array[4][7] = 0;
    array[4][8] = 0;
    array[4][9] = 1;

    array[5][0] = x0;
    array[5][1] = y1;
    array[5][2] = height;
    array[5][3] = color[0];
    array[5][4] = color[1];
    array[5][5] = color[2];
    array[5][6] = color[3];
    array[5][7] = 0;
    array[5][8] = 0;
    array[5][9] = 1;
}
