
#include <iostream>
#include <cmath>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "geometry.hpp"
/*
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
*/
// noiseSet -----------------------------------------------------------------

noiseSet::noiseSet(
                    unsigned int NumOctaves,
                    float Lacunarity,
                    float Persistance,
                    float Scale,
                    float Multiplier,
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
/*
noiseSet::noiseSet(const noiseSet& obj)
{
    noise = obj.noise;
    noiseType = obj.noiseType;
    numOctaves = obj.numOctaves;
    lacunarity = obj.lacunarity;
    persistance = obj.persistance;
    scale = obj.scale;
    multiplier = obj.multiplier;

    for(size_t i = 0; i < numOctaves; i++)
    {
        octaveOffsets[i][0] = obj.octaveOffsets[i][0];
        octaveOffsets[i][1] = obj.octaveOffsets[i][1];
    }
}

bool noiseSet::operator== (noiseSet& obj)
{
    if( noiseType != obj.noiseType ||
        numOctaves != obj.numOctaves ||
        lacunarity != obj.lacunarity ||
        persistance != obj.persistance ||
        scale != obj.scale ||
        multiplier != obj.multiplier)
        return false;

    for(size_t i = 0; i < numOctaves; i++)
    {
        if( octaveOffsets[i][0] != obj.octaveOffsets[i][0] ||
            octaveOffsets[i][1] != obj.octaveOffsets[i][1])
            return false;
    }

    return true;
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
       << std::endl;
}
*/
float                 noiseSet::GetNoise(float X, float Y, bool includeOffset)
{
    float result = 0;
    float frequency = 1, amplitude = 1;

    for(int i = 0; i < numOctaves; i++)
    {
        if (includeOffset)
        {
            X = X / scale * frequency + octaveOffsets[i][0];
            Y = Y / scale * frequency + octaveOffsets[i][1];
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

float                 noiseSet::getExtreme() const { return extreme; };

unsigned              noiseSet::getNoiseType()      const { return noiseType; };
unsigned              noiseSet::getNumOctaves()     const { return numOctaves; };
float                 noiseSet::getLacunarity()     const { return lacunarity; };
float                 noiseSet::getPersistance()    const { return persistance; };
float                 noiseSet::getScale()          const { return scale; };
float                 noiseSet::getMultiplier()     const { return multiplier; };
float                 noiseSet::getOffsetX()        const { return offsetX; }
float                 noiseSet::getOffsetY()        const { return offsetY; }
unsigned int          noiseSet::getSeed()           const { return seed; }
float*                noiseSet::getOffsets()        const { return &octaveOffsets[0][0]; };

// terrainGenerator -----------------------------------------------------------------

terrainGenerator::terrainGenerator(noiseSet &noise, unsigned dimensionX, unsigned dimensionY)
{
    Xside = 0;
    Yside = 0;

    computeTerrain(noise, dimensionX, dimensionY);
}
/*
terrainGenerator::terrainGenerator(const terrainGenerator& obj)
{
    Xside = obj.Xside;
    Yside = obj.Yside;

    totalVert = obj.totalVert;
    totalVertUsed = obj.totalVertUsed;

    delete[] field;
    field = new float[totalVert][11];
    for(size_t i = 0; i < totalVert; i++)
    {
        for(int j = 0; j < 11; j++)
            field[i][j] = obj.field[i][j];
    }

    delete[] indices;
    indices = new unsigned int[totalVertUsed/3][3];
    for(size_t i = 0; i < totalVertUsed; i++)
    {
        indices[i][0] = obj.indices[i][0];
        indices[i][1] = obj.indices[i][1];
        indices[i][2] = obj.indices[i][2];
    }
}

bool terrainGenerator::operator== (terrainGenerator& obj)
{
    if( Xside == obj.Xside &&
        Yside == obj.Yside &&
        totalVert == obj.totalVert &&
        totalVertUsed == obj.totalVertUsed)
        return true;
    else
        return false;
}

std::ostream& operator << (std::ostream& os, const terrainGenerator& obj)
{
    unsigned Xside = obj.Xside;
    unsigned Yside = obj.Yside;

    os << "Side: " << Xside << " x " << Yside << std::endl;
    os << "Drawn vertices: " << obj.totalVertUsed << std::endl;

    os << "Vertices: " << std::endl;
    for(size_t y = Yside-1; y < Yside; y--)
    {
        for(size_t x = 0; x < Xside; x++)
            os << "(" << obj.field[obj.getPos(x, y)][0] << ", " << obj.field[obj.getPos(x, y)][1] << ") ";
        os << std::endl;
    }

    os << "Indices: " << std::endl;
    for(size_t i = 0; i < obj.totalVertUsed/3; i++)
        os << obj.indices[i][0] << ", " << obj.indices[i][1] << ", " << obj.indices[i][2] << std::endl;
}
*/
terrainGenerator::~terrainGenerator()
{
    delete[] field;
    delete[] indices;
}

void terrainGenerator::computeTerrain(noiseSet &noise, unsigned dimensionX, unsigned dimensionY)
{
    if (dimensionX != Xside || dimensionY != Yside)
    {
        Xside = dimensionX;
        Yside = dimensionY;
        totalVert = Xside * Yside;
        totalVertUsed = (Xside - 1) * (Yside - 1) * 2 * 3;

        delete[] field;
        field = new float[totalVert][11];
        delete[] indices;
        indices = new unsigned int[totalVertUsed/3][3];
    }

    float textureFactor = 0.1;

    // Vertex data
    for (size_t y = 0; y < Yside; y++)
        for (size_t x = 0; x < Xside; x++)
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

    for (size_t y = 0; y < Yside - 1; y++)            // Compute normals
        for (size_t x = 0; x < Xside - 1; x++)
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

    for (size_t y = 0; y < Yside - 1; y++)
        for (size_t x = 0; x < Xside - 1; x++)
        {
            unsigned int pos = getPos(x, y);

            indices[index][0] = pos;
            indices[index][1] = pos + Xside + 1;
            indices[index++][2] = pos + Xside;
            indices[index][0] = pos;
            indices[index][1] = pos + 1;
            indices[index++][2] = pos + Xside + 1;
        }
}

size_t terrainGenerator::getPos(size_t x, size_t y) const { return y * Xside + x; }

unsigned terrainGenerator::getXside() const { return Xside; }

unsigned terrainGenerator::getYside() const { return Yside; }

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
