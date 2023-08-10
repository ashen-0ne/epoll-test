#include "shmbuffer.h"

void ShmBuffer::init(char * buffer,int size)
{
    m_write_pos = reinterpret_cast<int *>(buffer);
    m_read_pos = reinterpret_cast<int *>(buffer + 4);
    m_buffer = buffer + 8;
    m_size = size - 8;

    lock_fd = open("/tmp/shm_lock_file_zbwtest", O_CREAT | O_RDWR, 0666);
}

bool ShmBuffer::write(char * data,int len)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // 从锁定位置到文件末尾
    fcntl(lock_fd, F_SETLKW, &lock);  // 获取文件锁

    std::cout<<"m_write_pos:"<<*m_write_pos<<std::endl;
    std::cout<<"m_read_pos:"<<*m_read_pos<<std::endl;
    std::cout<<"m_size:"<<m_size<<std::endl;

    if(*m_write_pos + 4 + len < m_size)
    {
        if(len <= (m_size + *m_read_pos - *m_write_pos - 1 - 4) % m_size)
        {
            memcpy(m_buffer + *m_write_pos,&len,4);
            memcpy(m_buffer + *m_write_pos + 4,data,len);
            *m_write_pos = (*m_write_pos + len + 4) % m_size;

            lock.l_type = F_UNLCK;
            fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
            return true;
        }
        else
        {
            lock.l_type = F_UNLCK;
            fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
            return false;
        }
    }
    else
    {
        if(len <= (m_size + *m_read_pos - *m_write_pos - 1 - 4) % m_size)
        {
            memcpy(m_buffer + *m_write_pos,&len,4);
            memcpy(m_buffer + *m_write_pos + 4,data,m_size - (*m_write_pos + 4));
            memcpy(m_buffer,data + (m_size - *m_write_pos - 4),len - (m_size - *m_write_pos -4));
            *m_write_pos = (*m_write_pos + len + 4) % m_size;

            lock.l_type = F_UNLCK;
            fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
            return true;
        }
        else
        {
            lock.l_type = F_UNLCK;
            fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
            return false;
        }
    }
}

int ShmBuffer::read(char ** data)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // 从锁定位置到文件末尾
    fcntl(lock_fd, F_SETLKW, &lock);  // 获取文件锁

    int len = *(reinterpret_cast<int *>(m_buffer + *m_read_pos));
    std::cout<<"len:"<<len<<std::endl;

    if(len == 0)
    {
        lock.l_type = F_UNLCK;
        fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
        return false;
    }

    if(len <= m_size - *m_read_pos - 4)
    {
        memcpy(*data,reinterpret_cast<char *>(m_buffer + *m_read_pos + 4),len);
        memset(reinterpret_cast<char *>(m_buffer + *m_read_pos),0,len + 4);
        *m_read_pos = (*m_read_pos + m_size + len + 4) % m_size;

        lock.l_type = F_UNLCK;
        fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
        return len;
    }
    else
    {
        memcpy(*data,reinterpret_cast<char *>(m_buffer + *m_read_pos + 4),m_size - *m_read_pos - 4);
        memcpy(*data + m_size - *m_read_pos - 4,reinterpret_cast<char *>(m_buffer),len - m_size + *m_read_pos + 4);
        memset(reinterpret_cast<char *>(m_buffer + *m_read_pos),0,m_size - *m_read_pos);
        memset(reinterpret_cast<char *>(m_buffer),0,len + 4 - m_size + *m_read_pos);
        *m_read_pos = (*m_read_pos + m_size + len + 4) % m_size;

        lock.l_type = F_UNLCK;
        fcntl(lock_fd, F_SETLK, &lock);  // 释放文件锁
        return len;
    }
}