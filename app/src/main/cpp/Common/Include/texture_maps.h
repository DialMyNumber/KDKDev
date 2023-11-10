#ifndef TEXTURE_MAPS_H
#define TEXTURE_MAPS_H

#include <map>

class TextureMaps {

public:
    typedef struct {
        unsigned int id;
        unsigned int target;
        unsigned char isMipmapped;
    } MapConfigs;

    void addTextureMap(std::string texMapFilename);
    //bool findTextureMap(std::string texMapFilename);
    bool findTextureMap(std::string texMapFilename, MapConfigs *ret_configs);
    void setMapConfigs(std::string texMapFilename, unsigned int id, unsigned int target, unsigned char isMipmapped);
    MapConfigs *getMapConfigs(std::string texMapFilename);
    std::map<std::string, MapConfigs> getMaps() const;
    void clearMaps(); // Added by Jae-ho Nah(2014.11)

private:
    std::map<std::string, MapConfigs> m_maps;
};


inline std::map<std::string, TextureMaps::MapConfigs> TextureMaps::getMaps() const
{
    return m_maps;
}

//Added by Jae-ho Nah (2014.11) for memory deallocation
inline void TextureMaps::clearMaps()
{
    m_maps.clear();
}

#endif