#include "ResourceLoader.h"
#include "ResourceCache.h"

#include "tiny_obj_loader.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include "toml.hpp"

#include <cstdlib>
#include <fstream>

namespace Wiley
{


    MeshLoader::MeshLoader(ResourceCache* resourceCache)
        :resourceCache(resourceCache)
    {

    }

    MaterialLoader::MaterialLoader(ResourceCache* resourceCache)
        :resourceCache(resourceCache)
    {

    }

    ImageTextureLoader::ImageTextureLoader(ResourceCache* resourceCache)
        :resourceCache(resourceCache)
    {

    }

  
}
