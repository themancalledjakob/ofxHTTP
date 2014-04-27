//
// Twitter.cpp
//
// $Id: //poco/1.4/Net/samples/TwitterClient/src/Twitter.cpp#2 $
//
// A C++ implementation of a Twitter client based on the POCO Net library.
//
// Copyright (c) 2009-2013, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "Twitter.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPBasicCredentials.h"
//#include "Poco/Util/JSONConfiguration.h"
#include "Poco/URI.h"
#include "Poco/SHA1Engine.h"
#include "Poco/HMACEngine.h"
#include "Poco/Base64Encoder.h"
#include "Poco/RandomStream.h"
#include "Poco/Timestamp.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Format.h"
#include "Poco/StreamCopier.h"
#include <sstream>
#include <map>

#include "ofx/HTTP/Client/Context.h"

Twitter::Twitter()
{
}


Twitter::~Twitter()
{
}


void Twitter::login(const std::string& consumerKey,
                    const std::string& consumerSecret,
                    const std::string& token,
                    const std::string& tokenSecret)
{
	_consumerKey    = consumerKey;
	_consumerSecret = consumerSecret;
	_token          = token;
	_tokenSecret    = tokenSecret;
}


void Twitter::processRequest(Poco::Net::HTTPRequest& request,
                             ofx::HTTP::Context& context)
{
    Poco::URI uri(request.getURI());

    Poco::Net::NameValueCollection nvc;

    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
    {
        nvc = ofx::HTTP::Utils::getQueryMap(uri);
    }

    Poco::URI cleanPath(uri.getScheme(), uri.getAuthority(), uri.getPath());

    sign(request, nvc, cleanPath.toString());
}

void Twitter::sign(Poco::Net::HTTPRequest& request, const Poco::Net::NameValueCollection& params, const std::string& uri) const
{
	std::string nonce(createNonce());
	std::string timestamp(Poco::NumberFormatter::format(Poco::Timestamp().epochTime()));
	std::string signature(createSignature(request, params, uri, nonce, timestamp));
	std::string authorization(
                              Poco::format(
                                           "OAuth"
                                           " oauth_consumer_key=\"%s\","
                                           " oauth_nonce=\"%s\","
                                           " oauth_signature=\"%s\","
                                           " oauth_signature_method=\"HMAC-SHA1\","
                                           " oauth_timestamp=\"%s\","
                                           " oauth_token=\"%s\","
                                           " oauth_version=\"1.0\"",
                                           percentEncode(_consumerKey),
                                           percentEncode(nonce),
                                           percentEncode(signature),
                                           timestamp,
                                           percentEncode(_token)
                                           )
                              );
	request.set("Authorization", authorization);
}


std::string Twitter::createNonce() const
{
	std::ostringstream base64Nonce;
	Poco::Base64Encoder base64Encoder(base64Nonce);
	Poco::RandomInputStream randomStream;
	for (int i = 0; i < 32; i++)
	{
		base64Encoder.put(randomStream.get());
	}
	base64Encoder.close();
	std::string nonce = base64Nonce.str();
	return Poco::translate(nonce, "+/=", "");
}


std::string Twitter::createSignature(Poco::Net::HTTPRequest& request,
                                     const Poco::Net::NameValueCollection& params,
                                     const std::string& uri,
                                     const std::string& nonce,
                                     const std::string& timestamp) const
{

	std::map<std::string, std::string> paramsMap;
	paramsMap["oauth_consumer_key"]     = percentEncode(_consumerKey);
	paramsMap["oauth_nonce"]            = percentEncode(nonce);
	paramsMap["oauth_signature_method"] = "HMAC-SHA1";
	paramsMap["oauth_timestamp"]        = timestamp;
	paramsMap["oauth_token"]            = percentEncode(_token);
	paramsMap["oauth_version"]          = "1.0";

	for (Poco::Net::HTMLForm::ConstIterator it = params.begin(); it != params.end(); ++it)
	{
		paramsMap[percentEncode(it->first)] = percentEncode(it->second);
	}

	std::string paramsString;
	for (std::map<std::string, std::string>::const_iterator it = paramsMap.begin(); it != paramsMap.end(); ++it)
	{
		if (it != paramsMap.begin()) paramsString += '&';
		paramsString += it->first;
		paramsString += "=";
		paramsString += it->second;
	}

	std::string signatureBase = request.getMethod();
	signatureBase += '&';
	signatureBase += percentEncode(uri);
	signatureBase += '&';
	signatureBase += percentEncode(paramsString);

	std::string signingKey;
	signingKey += percentEncode(_consumerSecret);
	signingKey += '&';
	signingKey += percentEncode(_tokenSecret);
	
	Poco::HMACEngine<Poco::SHA1Engine> hmacEngine(signingKey);
	hmacEngine.update(signatureBase);
	Poco::DigestEngine::Digest digest = hmacEngine.digest();
	std::ostringstream digestBase64;
	Poco::Base64Encoder base64Encoder(digestBase64);
	base64Encoder.write(reinterpret_cast<char*>(&digest[0]), digest.size());
	base64Encoder.close();
	return digestBase64.str();
}


std::string Twitter::percentEncode(const std::string& str)
{
	std::string encoded;
	Poco::URI::encode(str, "!?#/'\",;:$&()[]*+=@", encoded);
	return encoded;
}