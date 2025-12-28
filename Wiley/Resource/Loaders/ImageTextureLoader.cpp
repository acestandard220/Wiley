#include "../ResourceLoader.h"
#include "../ResourceCache.h"

#include "stb_image.h"
#include "stb_image_write.h"

#include "DDSTextureLoader.h"

namespace Wiley {


    Resource::Ref ImageTextureLoader::LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
    {
        if (path.extension().string() == ".dds") {
            return LoadFromDDSFile(path, loadDesc);
        }

        std::shared_ptr<ImageTexture> imageTextureRef = std::make_shared<ImageTexture>();
        void* data;
        int width, height, nChannel;

        stbi_set_flip_vertically_on_load(loadDesc.flipUV);

        bool is16Bit = false;
        if (stbi_is_16_bit(path.string().c_str())) {
            imageTextureRef->bitPerChannel = 16;

            data = stbi_load_16(path.string().c_str(), &width, &height, &nChannel, 0);
            is16Bit = true;
        }
        else {
            imageTextureRef->bitPerChannel = 8;
            data = stbi_load(path.string().c_str(), &width, &height, &nChannel, 0);
            is16Bit = false;
        }

        if (!data) {
            std::cout << "Failed to load Image Texture File." << std::endl;
            return nullptr;
        }

        std::vector<UINT8> pixelData;
        UINT nPixel = width * height;
        const UINT desiredComponentCount = 4;

        if (is16Bit) {
            pixelData.resize(nPixel * desiredComponentCount * 2);
            USHORT* dst = reinterpret_cast<USHORT*>(pixelData.data());
            USHORT* src = reinterpret_cast<USHORT*>(data);
            for (uint32_t i = 0; i < nPixel; i++) {
                dst[i * 4 + 0] = src[i * nChannel + 0];
                dst[i * 4 + 1] = src[i * nChannel + 1];
                dst[i * 4 + 2] = src[i * nChannel + 2];
                dst[i * 4 + 3] = (nChannel == 4) ? src[i * nChannel + 3] : USHRT_MAX;
            }
        }
        else {
            pixelData.resize(nPixel * desiredComponentCount * 1);
            UCHAR* dst = reinterpret_cast<UCHAR*>(pixelData.data());
            UCHAR* src = reinterpret_cast<UCHAR*>(data);
            for (uint32_t i = 0; i < nPixel; i++) {
                dst[i * 4 + 0] = src[i * nChannel + 0];
                dst[i * 4 + 1] = src[i * nChannel + 1];
                dst[i * 4 + 2] = src[i * nChannel + 2];
                dst[i * 4 + 3] = (nChannel == 4) ? src[i * nChannel + 3] : 255;
            }
        }

        imageTextureRef->width = width;
        imageTextureRef->height = height;
        imageTextureRef->nChannels = 4;
        imageTextureRef->mapType = loadDesc.desc.imageTextureDesc.type;
        nChannel = 4;

        stbi_image_free(data);

        imageTextureRef->textureResource = resourceCache->rctx->CreateShaderResourceTexture(pixelData.data(),
            width, height, nChannel, imageTextureRef->bitPerChannel);

        auto descManager = resourceCache->GetImageTextureDescriptorManager(loadDesc.desc.imageTextureDesc.type);
        UINT descriptorIndex = resourceCache->GetFreeImageDescriptorIndex(descManager);
        imageTextureRef->textureResource->BuildSRV(descManager->descriptors[descriptorIndex]);
        imageTextureRef->srvIndex = descriptorIndex;

        return imageTextureRef;
    }

    Resource::Ref ImageTextureLoader::LoadFromDDSFile(filespace::filepath path, ResourceLoadDesc& loadDesc) {
        std::shared_ptr<ImageTexture> imageTextureRef = std::make_shared<ImageTexture>();

        int width, height, nChannel, bitPerChannel;
        imageTextureRef->textureResource = resourceCache->rctx->CreateShaderResourceTextureFromFile(path, width, height, nChannel, bitPerChannel);

        imageTextureRef->width = width;
        imageTextureRef->height = height;
        imageTextureRef->nChannels = nChannel;
        imageTextureRef->bitPerChannel = bitPerChannel;

        auto descManager = resourceCache->GetImageTextureDescriptorManager(loadDesc.desc.imageTextureDesc.type);
        UINT descriptorIndex = resourceCache->GetFreeImageDescriptorIndex(descManager);
        imageTextureRef->textureResource->BuildSRV(descManager->descriptors[descriptorIndex]);
        imageTextureRef->srvIndex = descriptorIndex;
        return imageTextureRef;
    }

    void ImageTextureLoader::SaveToFile(filespace::filepath path, ImageTexture* imageTexture)
    {
        std::vector<uint8_t> textureBytes = resourceCache->rctx->GetTextureBytes(imageTexture->textureResource);

        std::string extension = path.extension().string();
        int result = 0;

        if (extension == ".bmp") {
            result = stbi_write_bmp(path.string().c_str(), imageTexture->width,
                imageTexture->height, imageTexture->nChannels, textureBytes.data());
        }
        else if (extension == ".jpeg" || extension == ".jpg") {
            result = stbi_write_jpg(path.string().c_str(), imageTexture->width,
                imageTexture->height, imageTexture->nChannels, textureBytes.data(), 100);
        }
        else if (extension == ".png") {
            result = stbi_write_png(path.string().c_str(), imageTexture->width,
                imageTexture->height, imageTexture->nChannels, textureBytes.data(),
                imageTexture->width * imageTexture->nChannels);
        }
        else if (extension == ".tga") {
            result = stbi_write_tga(path.string().c_str(), imageTexture->width,
                imageTexture->height, imageTexture->nChannels, textureBytes.data());
        }

        if (!result) {
            std::cout << "Failed to save Image Texture to file." << std::endl;
            return;
        }
        return;
    }



}