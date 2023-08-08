#ifndef SHMBUFFER_H
#define SHMBUFFER_H

#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define LEN_POS 4
#define DATA_POS 8

class ShmBuffer
{
public:
    void init(char * buffer,int size);
    bool write(char * data,int len);
    int read(char ** data);

private:
    int * m_write_pos{nullptr};
    int * m_read_pos{nullptr};
    char * m_buffer{nullptr};
    int m_size;
};

#endif