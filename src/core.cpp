#include "core.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
namespace course {
namespace {
uint32_t rol(uint32_t x, unsigned n) { return (x << n) | (x >> (32-n)); }
bool hex(const std::string& s) {
    return std::all_of(s.begin(),s.end(),[](char c){return (c>='0'&&c<='9')||(c>='A'&&c<='F');});
}
}
std::string sha1(const std::string& text) {
    std::vector<uint8_t> bytes(text.begin(),text.end());
    const uint64_t bits=uint64_t(bytes.size())*8;
    bytes.push_back(0x80);
    while(bytes.size()%64!=56) bytes.push_back(0);
    for(int i=7;i>=0;--i) bytes.push_back(uint8_t(bits>>(8*i)));
    uint32_t h[5]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
    for(size_t offset=0;offset<bytes.size();offset+=64) {
        uint32_t w[80]{};
        for(int i=0;i<16;++i) for(int j=0;j<4;++j) w[i]=(w[i]<<8)|bytes[offset+4*i+j];
        for(int i=16;i<80;++i) w[i]=rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4];
        for(int i=0;i<80;++i) {
            uint32_t f,k;
            if(i<20){f=(b&c)|(~b&d);k=0x5A827999;}
            else if(i<40){f=b^c^d;k=0x6ED9EBA1;}
            else if(i<60){f=(b&c)|(b&d)|(c&d);k=0x8F1BBCDC;}
            else {f=b^c^d;k=0xCA62C1D6;}
            uint32_t t=rol(a,5)+f+e+k+w[i];e=d;d=c;c=rol(b,30);b=a;a=t;
        }
        h[0]+=a;h[1]+=b;h[2]+=c;h[3]+=d;h[4]+=e;
    }
    std::ostringstream out;out<<std::hex<<std::uppercase<<std::setfill('0');
    for(auto x:h) out<<std::setw(8)<<x;
    return out.str();
}
std::map<std::string,std::string> load_users(const std::string& path) {
    std::ifstream in(path);if(!in) throw std::runtime_error("cannot open clients database: "+path);
    std::map<std::string,std::string> users;std::string line;size_t n=0;
    while(std::getline(in,line)) {
        ++n;if(!line.empty()&&line.back()=='\r')line.pop_back();
        if(line.empty()||line.front()=='#')continue;
        auto pos=line.find(':');
        if(pos==std::string::npos||pos==0||pos>128||pos+1==line.size()||
           line.substr(0,pos).find_first_of(" \t")!=std::string::npos||
           !users.emplace(line.substr(0,pos),line.substr(pos+1)).second)
            throw std::runtime_error("invalid or duplicate database record at line "+std::to_string(n));
    }
    if(in.bad())throw std::runtime_error("database read error");
    if(users.empty())throw std::runtime_error("empty clients database");
    return users;
}
bool authenticate(const std::string& message,const std::map<std::string,std::string>& users) {
    if(message.size()<=56||message.size()>184)return false;
    auto login=message.substr(0,message.size()-56),salt=message.substr(message.size()-56,16),digest=message.substr(message.size()-40);
    if(!hex(salt)||!hex(digest))return false;
    auto it=users.find(login);if(it==users.end())return false;
    auto expected=sha1(salt+it->second);unsigned diff=0;
    for(size_t i=0;i<40;++i)diff|=unsigned(expected[i]^digest[i]);
    return diff==0;
}
uint16_t sum_squares(const std::vector<uint16_t>& values) {
    uint32_t sum=0;
    for(uint16_t v:values){uint32_t square=uint32_t(v)*v;if(square>65535-sum)return 65535;sum+=square;}
    return uint16_t(sum);
}
}
