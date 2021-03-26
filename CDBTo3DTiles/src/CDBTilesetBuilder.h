#pragma once

#include "CDB.h"
#include "Gltf.h"
#include <filesystem>
#include <vector>
#include <tbb/concurrent_vector.h>

using namespace CDBTo3DTiles;

struct SubtreeAvailability
{
    tbb::concurrent_vector<uint8_t> nodeBuffer;
    tbb::concurrent_vector<uint8_t> childBuffer;
    uint64_t nodeCount = 0;
    uint64_t childCount = 0;
};

struct DatasetGroup
{
    std::vector<CDBDataset> datasets;
    std::vector<std::filesystem::path> tilesetsToCombine;
    bool replace;
    int maxLevel = INT_MIN;
};

class CDBTilesetBuilder
{
public:
    struct TilesetCollection
    {
        std::unordered_map<size_t, std::filesystem::path> CSToPaths;
        std::unordered_map<size_t, CDBTileset> CSToTilesets;
    };
    CDBTilesetBuilder(const std::filesystem::path &cdbInputPath, const std::filesystem::path &output)
        : elevationNormal{false}
        , elevationLOD{false}
        , use3dTilesNext{false}
        , subtreeLevels{7}
        , elevationDecimateError{0.01f}
        , elevationThresholdIndices{0.3f}
        , cdbPath{cdbInputPath}
        , outputPath{output}
    {
        if (std::filesystem::exists(output)) {
            std::filesystem::remove_all(output);
        }
    }

    SubtreeAvailability createSubtreeAvailability()
    {
        SubtreeAvailability subtree;
        subtree.nodeBuffer.resize(nodeAvailabilityByteLengthWithPadding);
        subtree.childBuffer.resize(childSubtreeAvailabilityByteLengthWithPadding);
        memset(&subtree.nodeBuffer[0], 0, nodeAvailabilityByteLengthWithPadding);
        memset(&subtree.childBuffer[0], 0, childSubtreeAvailabilityByteLengthWithPadding);
        return subtree;
    }

    void createTileAndChildSubtreeAtKey(std::map<std::string, SubtreeAvailability> &tileAndChildAvailabilities,
                                        std::string subtreeKey)
    {
        if (tileAndChildAvailabilities.count(subtreeKey) == 0) {
            tileAndChildAvailabilities.insert(
                std::pair<std::string, SubtreeAvailability>(subtreeKey, createSubtreeAvailability()));
        }
    }

    void flushTilesetCollection(const CDBGeoCell &geoCell,
                                std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
                                bool replace = true);

    std::vector<std::string> flushDatasetGroupTilesetCollections(const CDBGeoCell &geocell,
                                                                 DatasetGroup &group,
                                                                 std::string datasetGroupName);

    std::map<std::string, std::vector<std::string>> flushTilesetCollectionsMultiContent(
        const CDBGeoCell &geoCell);

    std::string levelXYtoSubtreeKey(int level, int x, int y);

    std::string cs1cs2ToCSKey(int cs1, int cs2);

    void addAvailability(const CDBTile &cdbTile);

    void addDatasetAvailability(const CDBTile &cdbTile,
                                SubtreeAvailability *subtree,
                                int subtreeRootLevel,
                                int subtreeRootX,
                                int subtreeRootY);

    bool setBitAtXYLevelMorton(tbb::concurrent_vector<uint8_t> &buffer, int localX, int localY, int localLevel = 0);

    void setParentBitsRecursively(std::map<std::string, SubtreeAvailability> &tileAndChildAvailabilities,
                                  int level,
                                  int x,
                                  int y,
                                  int subtreeRootLevel,
                                  int subtreeRootX,
                                  int subtreeRootY);

    void addElevationToTilesetCollection(CDBElevation &elevation,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory);

    void addElevationToTileset(CDBElevation &elevation,
                               const Texture *imagery,
                               const CDB &cdb,
                               const std::filesystem::path &outputDirectory);
                            //    CDBTileset &tileset);

    void fillMissingPositiveLODElevation(const CDBElevation &elevation,
                                         const Texture *currentImagery,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory);
                                        //  CDBTileset &tileset);

    void fillMissingNegativeLODElevation(CDBElevation &elevation,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory);
                                        //  CDBTileset &tileset);

    void addSubRegionElevationToTileset(CDBElevation &subRegion,
                                        const CDB &cdb,
                                        std::optional<CDBImagery> &subRegionImagery,
                                        const Texture *parentTexture,
                                        const std::filesystem::path &outputDirectory);
                                        // CDBTileset &tileset);

    void generateElevationNormal(Mesh &simplifed);

    Texture createImageryTexture(CDBImagery &imagery, const std::filesystem::path &tilesetDirectory) const;

    void addVectorToTilesetCollection(const CDBGeometryVectors &vectors,
                                      const std::filesystem::path &collectionOutputDirectory,
                                      std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections);

    std::vector<Texture> writeModeTextures(const std::vector<Texture> &modelTextures,
                                           const std::vector<osg::ref_ptr<osg::Image>> &images,
                                           const std::filesystem::path &textureSubDir,
                                           const std::filesystem::path &gltfPath);

    void addGTModelToTilesetCollection(const CDBGTModels &model, const std::filesystem::path &outputDirectory);

    void addGSModelToTilesetCollection(const CDBGSModels &model, const std::filesystem::path &outputDirectory);

    void createB3DMForTileset(tinygltf::Model &model,
                              CDBTile cdbTile,
                              const CDBInstancesAttributes *instancesAttribs,
                              const std::filesystem::path &outputDirectory);
                              //CDBTileset &tilesetCollections);

    size_t hashComponentSelectors(int CS_1, int CS_2);

    std::filesystem::path getTilesetDirectory(int CS_1,
                                              int CS_2,
                                              const std::filesystem::path &collectionOutputDirectory);

    void getTileset(const CDBTile &cdbTile,
                    const std::filesystem::path &outputDirectory,
                    std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
                    CDBTileset *&tileset,
                    std::filesystem::path &path);

    static const std::string ELEVATIONS_PATH;
    static const std::string ROAD_NETWORK_PATH;
    static const std::string RAILROAD_NETWORK_PATH;
    static const std::string POWERLINE_NETWORK_PATH;
    static const std::string HYDROGRAPHY_NETWORK_PATH;
    static const std::string GTMODEL_PATH;
    static const std::string GSMODEL_PATH;
    static const std::unordered_set<std::string> DATASET_PATHS;
    static const int MAX_LEVEL;

    bool elevationNormal;
    bool elevationLOD;
    bool use3dTilesNext;
    int subtreeLevels;
    uint64_t nodeAvailabilityByteLengthWithPadding;
    uint64_t childSubtreeAvailabilityByteLengthWithPadding;
    // dataset group name -> subtree root "level_x_y" -> subtree
    std::map<std::string, std::map<std::string, SubtreeAvailability>> datasetGroupTileAndChildAvailabilities;

    // Dataset -> component selectors "CS1_CS2" -> subtree root "level_x_y" -> subtree
    std::map<CDBDataset, std::map<std::string, std::map<std::string, SubtreeAvailability>>> datasetCSSubtrees;

    float elevationDecimateError;
    float elevationThresholdIndices;
    std::filesystem::path cdbPath;
    std::filesystem::path outputPath;
    std::vector<std::filesystem::path> defaultDatasetToCombine;
    std::vector<std::vector<std::string>> requestedDatasetToCombine;
    std::unordered_set<std::string> processedModelTextures;
    std::unordered_map<CDBTile, Texture> processedParentImagery;
    std::unordered_map<std::string, std::filesystem::path> GTModelsToGltf;
    std::unordered_map<CDBGeoCell, TilesetCollection> elevationTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> roadNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> railRoadNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> powerlineNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> hydrographyNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> GTModelTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> GSModelTilesets;

    std::map<CDBDataset, std::unordered_map<CDBGeoCell, TilesetCollection> *> datasetTilesetCollections
        = {{CDBDataset::Elevation, &elevationTilesets},
           {CDBDataset::GSFeature, &GSModelTilesets},
           {CDBDataset::GSModelGeometry, &GSModelTilesets},
           {CDBDataset::GSModelTexture, &GSModelTilesets},
           {CDBDataset::GTFeature, &GTModelTilesets},
           {CDBDataset::GTModelGeometry_500, &GTModelTilesets},
           {CDBDataset::GTModelTexture, &GTModelTilesets},
           {CDBDataset::RoadNetwork, &roadNetworkTilesets},
           {CDBDataset::RailRoadNetwork, &railRoadNetworkTilesets},
           {CDBDataset::PowerlineNetwork, &powerlineNetworkTilesets},
           {CDBDataset::HydrographyNetwork, &hydrographyNetworkTilesets}};

    std::map<std::string, DatasetGroup> datasetGroups
        = {{"Elevation", {{CDBDataset::Elevation}, {}, true}},
           {"GSFeature",
            {{CDBDataset::GSFeature, CDBDataset::GSModelGeometry, CDBDataset::GSModelTexture},
             {},
             false}}, // additive refinement
           {"GTandVectors",
            {{CDBDataset::GTFeature,
              CDBDataset::GTModelGeometry_500,
              CDBDataset::GTModelTexture,
              CDBDataset::RoadNetwork,
              CDBDataset::RailRoadNetwork,
              CDBDataset::PowerlineNetwork,
              CDBDataset::HydrographyNetwork},
             {},
             true}}};

    std::map<CDBDataset, std::string> datasetToGroupName = {{CDBDataset::Elevation, "Elevation"},
                                                            {CDBDataset::GSFeature, "GSFeature"},
                                                            {CDBDataset::GSModelGeometry, "GSFeature"},
                                                            {CDBDataset::GSModelTexture, "GSFeature"},
                                                            {CDBDataset::GTFeature, "GTandVectors"},
                                                            {CDBDataset::GTModelGeometry_500, "GTandVectors"},
                                                            {CDBDataset::GTModelTexture, "GTandVectors"},
                                                            {CDBDataset::RoadNetwork, "GTandVectors"},
                                                            {CDBDataset::RailRoadNetwork, "GTandVectors"},
                                                            {CDBDataset::PowerlineNetwork, "GTandVectors"},
                                                            {CDBDataset::HydrographyNetwork, "GTandVectors"}};

    std::map<CDBDataset, int> datasetMaxLevels;


    // Support for parallelism
    tbb::concurrent_vector<CDBTile> tilesToInsertInTilesets;
    std::map<CDBDataset, std::filesystem::path> datasetDirs;
    void flushTilesToInsert();
};