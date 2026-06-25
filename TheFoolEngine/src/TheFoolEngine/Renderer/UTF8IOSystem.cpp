#include <tfpch.h>
#include "UTF8IOSystem.h"

TheFoolEngine::UTF8IOSystem::UTF8IOSystem()
    :m_DefaultIOSystem(new Assimp::DefaultIOSystem())
{
}

bool TheFoolEngine::UTF8IOSystem::Exists(const char* pFile) const
{
    std::filesystem::path filepath(pFile);
    return std::filesystem::exists(filepath);
}

char TheFoolEngine::UTF8IOSystem::getOsSeparator() const
{
    return m_DefaultIOSystem->getOsSeparator();
}

Assimp::IOStream* TheFoolEngine::UTF8IOSystem::Open(const char* pFile, const char* pMode)
{
    if (strcmp(pMode, "rb") != 0)
        return m_DefaultIOSystem->Open(pFile, pMode);

    Ref<std::ifstream> filestream = CreateRef<std::ifstream>(pFile, std::ios::binary);
    if (!filestream->is_open())
    {
        TF_CORE_ERROR("Failed to load model file, path::{0}", pFile);
        return nullptr;
    }

    return new CustomIOStream(filestream);
}

void TheFoolEngine::UTF8IOSystem::Close(Assimp::IOStream* pFile)
{
    delete pFile;
}

TheFoolEngine::CustomIOStream::CustomIOStream(Ref<std::ifstream> stream)
    :m_IOStream(stream)
{
}

TheFoolEngine::CustomIOStream::~CustomIOStream()
{
    if (m_IOStream)
        m_IOStream->close();
}

std::size_t TheFoolEngine::CustomIOStream::Read(void* pvBuffer, std::size_t pSize, std::size_t pCount)
{
    if (!m_IOStream)
        return 0;
    m_IOStream->read(static_cast<char*>(pvBuffer), pSize * pCount);
    return static_cast<std::size_t>(m_IOStream->gcount() / pSize);
}

aiReturn TheFoolEngine::CustomIOStream::Seek(std::size_t pOffset, aiOrigin pOrigin)
{
    if (!m_IOStream)
        return aiReturn_FAILURE;
    std::ios_base::seekdir dir;
    switch (pOrigin)
    {
        case aiOrigin_SET: dir = std::ios::beg; break;
        case aiOrigin_CUR: dir = std::ios::cur; break;
        case aiOrigin_END: dir = std::ios::end; break;
        default: return aiReturn_FAILURE;
    }
    m_IOStream->seekg(pOffset, dir);
    return m_IOStream->good() ? aiReturn_SUCCESS : aiReturn_FAILURE;
}

std::size_t TheFoolEngine::CustomIOStream::Tell() const
{
    if (!m_IOStream)
        return 0;
    return static_cast<std::size_t>(m_IOStream->tellg());
}

std::size_t TheFoolEngine::CustomIOStream::FileSize() const
{
    if (!m_IOStream)
        return 0;
    auto pos = m_IOStream->tellg();
    m_IOStream->seekg(0, std::ios::end);
    auto size = m_IOStream->tellg();
    m_IOStream->seekg(pos, std::ios::beg);
    return static_cast<std::size_t>(size);
}
