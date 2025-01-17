/*****************************************************************
|
|   Neptune - Ring Buffer
|
|   (c) 2001-2006 Gilles Boccon-Gibod
|   Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "NptRingBuffer.h"
#include "NptResults.h"
#include "NptUtils.h"
#include "NptStreams.h"

/*----------------------------------------------------------------------
|   NPT_RingBuffer::NPT_RingBuffer
+---------------------------------------------------------------------*/
NPT_RingBuffer::NPT_RingBuffer(NPT_Size size) :
    m_Size(size),
    m_BufferIsLocal(true),
    m_Closed(false)
{
    m_Data.start = new unsigned char[size];
    m_Data.end   = m_Data.start + size;

    m_In = m_Out = m_Data.start;
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::NPT_RingBuffer
+---------------------------------------------------------------------*/
NPT_RingBuffer::NPT_RingBuffer(void* buffer, NPT_Size size) :
    m_Size(size),
    m_BufferIsLocal(false)
{
    m_Data.start = (unsigned char*)buffer;
    m_Data.end   = m_Data.start + size;

    m_In = m_Out = m_Data.start;
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::~NPT_RingBuffer
+---------------------------------------------------------------------*/
NPT_RingBuffer::~NPT_RingBuffer()
{
    if (m_BufferIsLocal) delete[] m_Data.start;
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::GetContiguousSpace
+---------------------------------------------------------------------*/
NPT_Size
NPT_RingBuffer::GetContiguousSpace() const
{
    return 
        (m_In < m_Out) ?
        (NPT_Size)(m_Out - m_In - 1) :
        ((m_Out == m_Data.start) ? 
         (NPT_Size)(m_Data.end - m_In - 1) : 
         (NPT_Size)(m_Data.end - m_In));
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::GetSpace
+---------------------------------------------------------------------*/
NPT_Size
NPT_RingBuffer::GetSpace() const
{
    return 
        (m_In < m_Out) ? 
        (NPT_Size)(m_Out - m_In - 1) : 
        (NPT_Size)(m_Data.end - m_In + m_Out - m_Data.start - 1);
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::Write
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::Write(const void* buffer, NPT_Size byte_count)
{
    if (m_Closed) return NPT_ERROR_WRITE_FAILED;

    if (byte_count == 0) return NPT_SUCCESS;
    if (m_In < m_Out) {
        if (buffer) NPT_CopyMemory(m_In, buffer, byte_count);
        m_In += byte_count;
        if (m_In == m_Data.end) m_In = m_Data.start;
    } else {
        unsigned int chunk = (unsigned int)(m_Data.end - m_In);
        if (chunk >= byte_count) chunk = byte_count;

        if (buffer) NPT_CopyMemory(m_In, buffer, chunk);
        m_In += chunk;
        if (m_In == m_Data.end) m_In = m_Data.start;
        if (chunk != byte_count) {
            if (buffer) {
                NPT_CopyMemory(m_In, 
                               ((const char*)buffer)+chunk, 
                               byte_count-chunk);
            }
            m_In += byte_count-chunk;
            if (m_In == m_Data.end) m_In = m_Data.start;
        }
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::GetContiguousAvailable
+---------------------------------------------------------------------*/
NPT_Size
NPT_RingBuffer::GetContiguousAvailable() const
{
    return 
        (m_Out <= m_In) ? 
        (NPT_Size)(m_In-m_Out) :
        (NPT_Size)(m_Data.end - m_Out);
}

/*----------------------------------------------------------------------
|   NPT_RingBuffer::GetAvailable
+---------------------------------------------------------------------*/
NPT_Size
NPT_RingBuffer::GetAvailable() const
{
    return 
        (m_Out <= m_In) ? 
        (NPT_Size)(m_In-m_Out) :
        (NPT_Size)(m_Data.end - m_Out + m_In - m_Data.start);
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::Read
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::Read(void* buffer, NPT_Size byte_count)
{
    if (m_Closed) return NPT_ERROR_READ_FAILED;

    if (byte_count == 0) return NPT_SUCCESS;
    if (m_In > m_Out) {
        if (buffer) NPT_CopyMemory(buffer, m_Out, byte_count);
        m_Out += byte_count;
        if (m_Out == m_Data.end) m_Out = m_Data.start;
    } else {
        unsigned int chunk = (unsigned int)(m_Data.end - m_Out);
        if (chunk >= byte_count) chunk = byte_count;

        if (buffer) NPT_CopyMemory(buffer, m_Out, chunk);
        m_Out += chunk;
        if (m_Out == m_Data.end) m_Out = m_Data.start;
        if (chunk != byte_count) {
            if (buffer) {
                NPT_CopyMemory(((char*)buffer)+chunk, m_Out, byte_count-chunk);
            }
            m_Out += byte_count-chunk;
            if (m_Out == m_Data.end) m_Out = m_Data.start;
        }
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::ReadByte
+----------------------------------------------------------------------*/
unsigned char
NPT_RingBuffer::ReadByte()
{
    unsigned char result = *m_Out++;
    if (m_Out == m_Data.end) m_Out = m_Data.start;
    return result;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::PeekByte
+----------------------------------------------------------------------*/
unsigned char
NPT_RingBuffer::PeekByte(NPT_Position offset)
{
    unsigned char *where;

    where = m_Out+offset;
    if (where >= m_Data.end) where -= (m_Data.end - m_Data.start);

    return *where;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::MoveIn
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::MoveIn(NPT_Position offset)
{
    int fold;

    m_In += offset;
    fold = (int)(m_In - m_Data.end);
    if (fold >= 0) {
        m_In = m_Data.start + fold;
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::MoveOut
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::MoveOut(NPT_Position offset)
{
    int fold;

    m_Out += offset;
    fold = (int)(m_Out - m_Data.end);
    if (fold >= 0) {
        m_Out = m_Data.start + fold;
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::Flush
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::Flush()
{
    m_In = m_Out = m_Data.start;

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------+
|    NPT_RingBuffer::Close
+----------------------------------------------------------------------*/
NPT_Result
NPT_RingBuffer::Close()
{
    m_Closed = true;
    return NPT_SUCCESS;
}

