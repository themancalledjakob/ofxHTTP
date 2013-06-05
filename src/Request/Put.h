/*==============================================================================
 
 Copyright (c) 2013 - Christopher Baker <http://christopherbaker.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 =============================================================================*/

#pragma once


#include "BaseRequest.h"


namespace ofx {
namespace HTTP {
namespace Request {

class Put : public BaseRequest {
public:
    Put(const Poco::URI& uri);
    Put(const Poco::URI& uri,
        const std::string& httpVersion);
    
    virtual ~Put();
    
    void addFile(const std::string& filename);
    
    void setContentRange(size_t startByte,
                         size_t endByte = std::numeric_limits<size_t>::max());

    void setContentType(const std::string& contentType);
    
protected:
    virtual void prepareRequest(Poco::Net::HTTPRequest& request) const;

    size_t startByte;
    size_t endByte;

    std::string contentType;
    
    ofFile file;

    friend class Client;

};

    
} } }
