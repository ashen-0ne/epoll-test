#include "shmbuffer.h"

void ShmBuffer::init(char * buffer,int size)
{
    m_write_pos = reinterpret_cast<int *>(buffer);
    m_read_pos = reinterpret_cast<int *>(buffer + 4);
    m_buffer = buffer + 8;
    m_size = size - 8;
}

bool ShmBuffer::write(char * data,int len)
{
    if(*m_write_pos + 4 + len < m_size)
    {
        if(len <= (m_size + m_read_pos - m_write_pos - 1 - 4) % m_size)
        {
            memcpy(m_buffer + *m_write_pos,&len,4);
            memcpy(m_buffer + *m_write_pos + 4,data,len);
            *m_write_pos = (*m_write_pos + len + 4) % m_size;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(len <= (m_size + m_read_pos - m_write_pos - 1 - 4) % m_size)
        {
            memcpy(m_buffer + *m_write_pos,&len,4);
            memcpy(m_buffer + *m_write_pos + 4,data,m_size - (*m_write_pos + 4));
            memcpy(m_buffer,data + (m_size - *m_write_pos - 4),len - (m_size - *m_write_pos -4));
            *m_write_pos = (*m_write_pos + len + 4) % m_size;
            return true;
        }
        else
        {
            return false;
        }
    }
}

int ShmBuffer::read(char ** data)
{
    int len = *(reinterpret_cast<int *>(m_buffer + *m_read_pos));

    if(len == 0)
    {
        return false;
    }

    if(len <= m_size - *m_read_pos - 4)
    {
        memcpy(*data,reinterpret_cast<char *>(m_buffer + *m_read_pos + 4),len);
        memset(reinterpret_cast<char *>(m_buffer + *m_read_pos),0,len + 4);
        *m_read_pos = (*m_read_pos + m_size + len + 4) % m_size;
        return len;
    }
    else
    {
        memcpy(*data,reinterpret_cast<char *>(m_buffer + *m_read_pos + 4),m_size - *m_read_pos - 4);
        memcpy(*data + m_size - *m_read_pos - 4,reinterpret_cast<char *>(m_buffer),len - m_size + *m_read_pos + 4);
        memset(reinterpret_cast<char *>(m_buffer + *m_read_pos),0,m_size - *m_read_pos);
        memset(reinterpret_cast<char *>(m_buffer),0,len - m_size + *m_read_pos);
        *m_read_pos = (*m_read_pos + m_size + len + 4) % m_size;
        return len;
    }
}