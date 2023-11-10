
#include "Common/Include/texture_maps.h"

void TextureMaps::addTextureMap(std::string texMapFilename)
{
    MapConfigs configs = { 0, 0, 0 };
    m_maps.insert(std::pair<std::string, MapConfigs>(texMapFilename, configs));
}

bool TextureMaps::findTextureMap(std::string texMapFilename, MapConfigs *ret_configs)
{
    auto it = m_maps.find(texMapFilename);
    if (it == m_maps.end() || it->second.id == 0)
        return false;

    ret_configs->id = it->second.id;
    ret_configs->isMipmapped = it->second.isMipmapped;
    ret_configs->target= it->second.target;

    return true;
}

void TextureMaps::setMapConfigs(std::string texMapFilename, unsigned int id, unsigned int target, unsigned char isMipmapped)
{
    MapConfigs configs = { id, target, isMipmapped };
    m_maps[texMapFilename] = configs;
}

TextureMaps::MapConfigs *TextureMaps::getMapConfigs(std::string texMapFilename)
{
    auto it = m_maps.find(texMapFilename);
    return &(it->second);
}
