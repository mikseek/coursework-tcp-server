#pragma once
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <cstdint>
namespace course {
class Socket {
    int fd_;
public:
    explicit Socket(int fd):fd_(fd){if(fd<0)throw std::runtime_error(std::strerror(errno));}
    ~Socket(){::close(fd_);}
    Socket(const Socket&)=delete;Socket& operator=(const Socket&)=delete;
    int get()const{return fd_;}
};
inline void transfer(int fd,void* data,size_t size,bool writing) {
    auto* p=static_cast<char*>(data);
    while(size){ssize_t n=writing ? ::send(fd,p,size,0) : ::recv(fd,p,size,0);
        if(n<0&&errno==EINTR)continue;
        if(n<=0)throw std::runtime_error(n==0?"peer disconnected":std::strerror(errno));
        p+=n;size-=size_t(n);
    }
}
inline void send_text(int fd,const std::string& text){transfer(fd,const_cast<char*>(text.data()),text.size(),true);}
inline std::string read_line(int fd,size_t limit=184) {
    std::string line;char c;
    for(;;){transfer(fd,&c,1,false);if(c=='\n')return line;if(line.size()>=limit)throw std::runtime_error("text line too long");line+=c;}
}
inline uint32_t read_number(int fd,unsigned bytes) {
    uint8_t b[4]{};transfer(fd,b,bytes,false);uint32_t v=0;for(unsigned i=0;i<bytes;++i)v|=uint32_t(b[i])<<(8*i);return v;
}
inline void write_number(int fd,uint32_t v,unsigned bytes) {
    uint8_t b[4]{};for(unsigned i=0;i<bytes;++i)b[i]=uint8_t(v>>(8*i));transfer(fd,b,bytes,true);
}
inline void set_timeout(int fd,int seconds) {
    timeval t{seconds,0};
    if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof(t))<0||setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&t,sizeof(t))<0)
        throw std::runtime_error("cannot set socket timeout");
}
inline uint32_t parse_number(const std::string& s,uint32_t max) {
    if(s.empty()||s.find_first_not_of("0123456789")!=std::string::npos)throw std::runtime_error("invalid number: "+s);
    auto n=std::stoull(s);if(n>max)throw std::runtime_error("number out of range: "+s);return uint32_t(n);
}
}
