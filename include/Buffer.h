//
// Created by zhouhf on 2025/4/19.
//

#pragma once
#include <vector>
#include <string>

namespace mymuduo {

    class Buffer {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize=kInitialSize)
            :buffer_(kCheapPrepend+initialSize),
            readerIndex_(kCheapPrepend),
            writerIndex_(kCheapPrepend) {};

        size_t readableBytes() const { return writerIndex_ - readerIndex_; };
        size_t writeableBytes() const { return buffer_.size() - writerIndex_; };
        size_t prependableByte() const {return readerIndex_; };

        const char* peek() const { return begin() + readerIndex_; };

        const char* findSubstr(const char *substr) const;

        std::string retrieveAsString(size_t len);

        void retrieve(size_t len)
        {
            popHead(len);
        }

        void retrieveAll()
        {
            popHeadAll();
        }

        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveUntil(const char* end)
        {
            return retrieveAsString(end - peek());
        }

        void ensureWritableBytes(size_t len);

        void append(const char* data, size_t len);

        // 从fd读取数据
        ssize_t readFd(int fd, int *saveErrno);
        ssize_t writeFd(int fd,int *saveErrno) const;
    private:
        char* beginRead() { return buffer_.data() + readerIndex_; };

        char* beginWrite() { return buffer_.data() + writerIndex_; };

        const char* beginRead() const { return buffer_.data() + readerIndex_; };

        const char* beginWrite() const { return buffer_.data() + writerIndex_; };

        void popHeadAll(){ readerIndex_ = writerIndex_ = kCheapPrepend; }
        void popHead(size_t len);
        char* begin() { return buffer_.data(); };
        const char* begin() const { return buffer_.data(); };

        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
    };

} // mymuduo

