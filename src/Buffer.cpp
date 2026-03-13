//
// Created by zhouhf on 2025/4/19.
//

#include <cerrno>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include "Buffer.h"

namespace mymuduo {
    void Buffer::popHead(size_t len) {
        if(len<readableBytes())
        {
            readerIndex_ += len;
        }else {
            popHeadAll();
        }
    }

    std::string Buffer::retrieveAsString(size_t len) {
        std::string result(peek(),len);
        popHead(len);
        return std::move(result);
    }

    void Buffer::ensureWritableBytes(size_t len) {
        if(writeableBytes() < len)
        {
            if(writeableBytes()+prependableByte() < len +kCheapPrepend){
                buffer_.resize(writerIndex_+len);
            }
            else
            {
                const size_t readable = readableBytes();
                std::copy(begin()+readerIndex_,
                          begin()+writerIndex_,
                          begin()+kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
            }
        }
    }

    void Buffer::append(const char *data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data,data+len,begin()+writerIndex_);
        writerIndex_ += len;
    }

    ssize_t Buffer::readFd(int fd, int *saveErrno) {
        char extraBuf[65536] = {0};
        iovec vec[2];
        const size_t writable = writeableBytes();
        vec[0].iov_base = begin()+writerIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuf;
        vec[1].iov_len = sizeof(extraBuf);
        const int iov_cnt = (writable< sizeof(extraBuf))?2:1;
        const ssize_t n = ::readv(fd,vec,iov_cnt);
        if(n<0)
        {
            *saveErrno = errno;
        } else if(n<=writable)
        {
            writerIndex_ += n;
        } else
        {
            writerIndex_ = buffer_.size();
            append(extraBuf, n-writable);
        }
        return n;
    }

    ssize_t Buffer::writeFd(int fd, int *saveErrno) const{
        ssize_t n = write(fd,peek(),readableBytes());
        if(n<0)
        {
            *saveErrno = errno;
        }
        return n;
    }

    const char *Buffer::findSubstr(const char *substr) const{
        const char *res = std::search(beginRead(),
                                      beginWrite(),
                                      substr,
                                      substr+strlen(substr)
                                      );
        return res == beginWrite() ? nullptr : res;
    }


} // mymuduo