#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <cinttypes>

namespace audio
{

class Buffer
{
public:
    Buffer(uint32_t size);
    ~Buffer();

    uint8_t* getData(uint32_t& size);
    void writeData(const uint8_t* pData, uint32_t size);
    uint32_t bytesFree();
    uint32_t bytesUsed();

    void clear();

private:
    uint32_t                m_Size;
    uint32_t                m_Fill;

    uint8_t*                m_pAudioBuffer;
    uint8_t*                m_pReadPtr;
    uint8_t*                m_pWritePtr;
    uint8_t*                m_pEnd;
};

}

#endif
