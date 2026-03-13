//
// Created by zhouhf on 2025/5/2.
//

#include "http/HttpContext.h"
#include "Buffer.h"

namespace mymuduo::http {
    HttpContext::HttpContext()
            : state_(EnumHttpState::ExpectRequestLine),
              request_() {

    }

    void HttpContext::parse(Buffer *buf, Timestamp receiveTime) {
        if(state_ == EnumHttpState::ExpectRequestLine)
        {
            const char *crlf = buf->findSubstr("\r\n");
            if(crlf != nullptr)
            {
                std::string requestLine = buf->retrieveUntil(crlf);
                size_t space1 = requestLine.find(' ');
                if(space1==-1)
                {
                    reset();
                    return ;
                }
                std::string method = requestLine.substr(0,space1);
                size_t space2 = requestLine.find(' ',space1+1);
                if(space2==-1)
                {
                    reset();
                    return;
                }
                std::string url = requestLine.substr(space1+1,space2-space1-1);
                std::string version = requestLine.substr(space2+1,requestLine.size()-space2-1-2);
            }
        }
    }


}