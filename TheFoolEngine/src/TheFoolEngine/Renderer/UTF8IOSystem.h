#pragma once

#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/DefaultIOSystem.h>
#include <filesystem>

namespace TheFoolEngine
{
    class CustomIOStream : public Assimp::IOStream
    {
    public:
        explicit CustomIOStream(Ref<std::ifstream> stream);
        ~CustomIOStream() override;

        std::size_t Read(void* pvBuffer, std::size_t pSize, std::size_t pCount) override;
        std::size_t Write(const void*, std::size_t, std::size_t) override { return 0; };
        aiReturn Seek(std::size_t pOffset, aiOrigin pOrigin) override;
        std::size_t Tell() const override;
        std::size_t FileSize() const override;
        void Flush() override {};
    private:
        Ref<std::ifstream> m_IOStream;
    };
    //////////////////////////////////////////////////////////////////////////////
    class UTF8IOSystem : public Assimp::IOSystem
    {
    public:
        UTF8IOSystem();

        bool Exists(const char* pFile) const override;
        char getOsSeparator() const override;
        Assimp::IOStream* Open(const char* pFile, const char* pMode) override;
        void Close(Assimp::IOStream* pFile) override;
    private:
        Scope<Assimp::DefaultIOSystem> m_DefaultIOSystem;
    };
}