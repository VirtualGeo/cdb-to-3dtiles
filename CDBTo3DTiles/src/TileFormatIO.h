#pragma once

#include "CDBAttributes.h"
#include "CDBTileset.h"
#include "tiny_gltf.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include "nlohmann/json.hpp"

namespace CDBTo3DTiles {

struct B3dmHeader
{
    char magic[4];
    uint32_t version;
    uint32_t byteLength;
    uint32_t featureTableJsonByteLength;
    uint32_t featureTableBinByteLength;
    uint32_t batchTableJsonByteLength;
    uint32_t batchTableBinByteLength;
};

struct I3dmHeader
{
    char magic[4];
    uint32_t version;
    uint32_t byteLength;
    uint32_t featureTableJsonByteLength;
    uint32_t featureTableBinByteLength;
    uint32_t batchTableJsonByteLength;
    uint32_t batchTableBinByteLength;
    uint32_t gltfFormat;
};

struct CmptHeader
{
    char magic[4];
    uint32_t version;
    uint32_t byteLength;
    uint32_t titleLength;
};

void createImplicitTilesetJson(CDBGeoCell geoCell, int subtreeLevels, std::ofstream &fs);

// void writeImplicitJson(CDBGeoCell geoCell,
//                         std::ofstream &fs);

void combineTilesetJson(const std::vector<std::filesystem::path> &tilesetJsonPaths,
                        const std::vector<Core::BoundingRegion> &regions,
                        std::ofstream &fs);

void writeToTilesetJson(const CDBTileset &tileset, bool replace, std::ofstream &fs, bool threeDTilesNext = false, int subtreeLevels = 7, int maxLevel = 0);

size_t writeToI3DM(std::string GltfURI,
                   const CDBModelsAttributes &modelsAttribs,
                   const std::vector<int> &attribIndices,
                   std::ofstream &fs);

void writeToB3DM(tinygltf::Model *gltf, const CDBInstancesAttributes *instancesAttribs, std::ofstream &fs);

void writeToCMPT(uint32_t numOfTiles,
                 std::ofstream &fs,
                 std::function<uint32_t(std::ofstream &fs, size_t tileIdx)> writeToTileFormat);

} // namespace CDBTo3DTiles
