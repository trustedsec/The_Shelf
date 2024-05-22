#include "bofpack.h"

bofpack::bofpack()
{
    buffer.resize(4); // make space cmdid at front
    
}

void* bofpack::getBuffer(unsigned int offset)
{
    if (!finalized)
    {
        finalized = 1;
    }
    if (offset > buffer.size()) return nullptr;
    return &(buffer[offset]);
}

void bofpack::setCmdId(uint32_t cmdid)
{
    memcpy(&(buffer[0]), &cmdid, 4);
}

uint32_t bofpack::getSize()
{
    return buffer.size();
}

void bofpack::clear()
{
    buffer.clear();
    buffer.resize(4);
}


