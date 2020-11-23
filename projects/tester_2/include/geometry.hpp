#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <iostream>
#include <random>

#include "FastNoiseLite.h"

// -----------------------------------------------------------------------------------
/*
/// Stores the configuration data that defines some terrain. Used in GUI and for drawing.
struct terrainData
{
    // Constructors and operator overloading
    terrainData();                                                                  ///< Default constructor. Configures a default terrain
    terrainData(const terrainData& obj);                                            ///< Copy constructor
    bool operator!= (terrainData& obj);                                             ///< Operator != overloading
    friend std::ostream& operator << (std::ostream& os, const terrainData& obj);    ///< Operator << overloading

    // Noise data   
    FastNoiseLite::NoiseType    noiseType;      ///< Enum (FastNoiseLite::NoiseType): <ul> <li>FastNoiseLite::NoiseType_Value</li> <li>FastNoiseLite::NoiseType_ValueCubic</li> <li>FastNoiseLite::NoiseType_Perlin</li> <li>FastNoiseLite::NoiseType_Cellular</li> <li>FastNoiseLite::NoiseType_OpenSimplex2</li> <li>FastNoiseLite::NoiseType_OpenSimplex2S</li> </ul>
    unsigned int                octaves;        ///< The bigger, the greater (x,y) computed for the highest octaves (exponential thanks to lacunarity), which has consequences: FastNoiseLite::GetNoise(x,y) output -nan(ind)
    float                       lacunarity;     ///< Range: [1, inf]. Determines frequency for each octave. Increases number of small features.
    float                       persistance;    ///< Range: [0, 1] Determines amplitude for each octave. Increases influence of small features.
    float                       scale;          ///< The bigger, the closer to detail; the smaller, the further away
    float                       multiplier;     ///< Increases height
    unsigned int                seed;           ///< Used for generating different octaves (used if addRandomOffset==true)
    float                       offset[2];      ///< Offset for all octaves

    // Others
    size_t                      dimensions[2];  ///< Dimensions of the map (terrain buffer)
    bool                        newConfig;      ///< Indicates whether the state has changed (used for recomputing and redrawing terrain)
};

std::ostream& operator << (std::ostream& os, const terrainData& obj);
*/
// -----------------------------------------------------------------------------------

/// Given some terrainData object, computes the heigth for certain (x,y) coordinates
class noiseSet
{
    FastNoiseLite noise;

    FastNoiseLite::NoiseType noiseType;
    unsigned int numOctaves;
    float lacunarity;
    float persistance;
    float scale;
    float multiplier;
    float offsetX, offsetY;
    unsigned int seed;
    float (*octaveOffsets)[2];

    float extreme;     // output range: [-extreme, extreme]

public:
    /*  Configure your noise generator:
     *      NumOctaves: Number of octaves
     *      Lacunarity: Should be in range [1, infinite]
     *      Persistance: Should be in range [0, 1]
     *      Scale: Scaling magnitude
     *      Multiplier: The resulting noise value is multiplied by this
     *      Seed: Generator's seed
     *      Offset: Offset for each octave. Array[n][2]. If nullptr, it's automatically configured
     *      NoiseType: FastNoiseLite::NoiseType_Perlin, FastNoiseLite::NoiseType_OpenSimplex2, etc.
     */
    noiseSet(unsigned int NumOctaves            = 6,
             float Lacunarity                   = 1.5,
             float Persistance                  = 0.5,
             float Scale                        = 1,
             float Multiplier                   = 30,
             float OffsetX                      = 0,
             float OffsetY                      = 0,
             FastNoiseLite::NoiseType NoiseType = FastNoiseLite::NoiseType_Perlin,
             bool addRandomOffset               = false,
             unsigned int Seed                  = 0);

    //noiseSet(const noiseSet& obj);
    //bool operator== (noiseSet& obj);
    //friend std::ostream& operator << (std::ostream& os, const noiseSet& obj);

    float                 GetNoise(float x, float y, bool includeOffset = false);

    float                 getExtreme() const;

    unsigned              getNoiseType() const;
    unsigned              getNumOctaves() const;
    float                 getLacunarity() const;
    float                 getPersistance() const;
    float                 getScale() const;
    float                 getMultiplier() const;
    float                 getOffsetX() const;
    float                 getOffsetY() const;
    unsigned int          getSeed() const;
    float*                getOffsets() const;
};

//std::ostream& operator << (std::ostream& os, const noiseSet& obj);

// -----------------------------------------------------------------------------------

/// Given a terrainData object, uses noiseSet for building a complete terrain buffer
class terrainGenerator
{
    size_t getPos(size_t x, size_t y) const;

    unsigned Xside, Yside;

public:
    terrainGenerator(noiseSet &noise, unsigned dimensionX, unsigned dimensionY);
    //terrainGenerator(const terrainGenerator& obj);                                      ///< Copy constructor
    //bool operator== (terrainGenerator& obj);                                            ///< Operator != overloading
    //friend std::ostream& operator << (std::ostream& os, const terrainGenerator& obj);   ///< Operator << overloading
    ~terrainGenerator();

    unsigned int totalVert;         ///< Amount of vertices passed to main (example: two triangles = 4)
    unsigned int totalVertUsed;     ///< Amount of vertices used for drawing (example: two triangles = 2*3)

    float (*field)[11];             ///< VBO
    unsigned int (*indices)[3];     ///< EBO

    void computeTerrain(noiseSet &noise, unsigned dimensionX, unsigned dimensionY);     ///< Compute VBO and EBO

    //float* getField() const;
    //unsigned* getIndices() const;
    unsigned getXside() const;
    unsigned getYside() const;
};

//std::ostream& operator << (std::ostream& os, const terrainGenerator& obj);

#endif

// -----------------------------------------------------------------------------------

/// Fills an array a coordinates axis (requires passing float array[12][3])
void fillAxis(float array[12][3], float sizeOfAxis);

// -----------------------------------------------------------------------------------
