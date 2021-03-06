/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <CloudGemFramework/HttpRequestJob.h>

#include <aws/core/http/HttpRequest.h>
#include <aws/core/http/HttpResponse.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/utils/stream/ResponseStream.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/auth/AWSAuthSigner.h>

#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/make_shared.h>

namespace CloudGemFramework
{
    namespace
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Mappings from HttpRequestJob nested types to Aws types
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        struct HttpMethodInfo
        {
            Aws::Http::HttpMethod m_awsMethod;
            AZStd::string m_name;
        };

// This will run the code fed to the macro, and then assign 0 to a static int (note the ,0 at the end)
#define CLOUD_CANVAS_ONCE_PASTE(x) (x)
#define CLOUD_CANVAS_ONCE(x) static int AZ_JOIN(init, __LINE__)((CLOUD_CANVAS_ONCE_PASTE(x), 0))

#define CLOUD_CANVAS_HTTP_METHOD_ENTRY(x)   { HttpRequestJob::HttpMethod::HTTP_##x, HttpMethodInfo{ Aws::Http::HttpMethod::HTTP_##x, #x } }

        using MethodLookup = AZStd::unordered_map<HttpRequestJob::HttpMethod, HttpMethodInfo>;

        AZ::EnvironmentVariable<MethodLookup> s_methodLookup = nullptr;
        const MethodLookup& GetMethodLookup()
        {
            CLOUD_CANVAS_ONCE(
                s_methodLookup = AZ::Environment::CreateVariable<MethodLookup>("methodlookup.httprequestjob.cloudcanvas", MethodLookup
                {
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(GET),
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(POST),
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(DELETE),
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(PUT),
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(HEAD),
                    CLOUD_CANVAS_HTTP_METHOD_ENTRY(PATCH),
                })
            );
            
            return *s_methodLookup;
        }

#undef CLOUD_CANVAS_HTTP_METHOD_ENTRY

#define CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(x)    { Aws::Http::HttpMethod::HTTP_##x, HttpRequestJob::HttpMethod::HTTP_##x }

        using MethodAwsReverseLookup = AZStd::unordered_map<Aws::Http::HttpMethod, HttpRequestJob::HttpMethod>;

        AZ::EnvironmentVariable<MethodAwsReverseLookup> s_methodAwsReverseLookup = nullptr;
        const MethodAwsReverseLookup& GetMethodAwsReverseLookup()
        {
            CLOUD_CANVAS_ONCE(s_methodAwsReverseLookup = AZ::Environment::CreateVariable<MethodAwsReverseLookup>("methodawsreverselookup.httprequestjob.cloudcanvas", MethodAwsReverseLookup
                {
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(GET),
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(POST),
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(DELETE),
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(PUT),
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(HEAD),
                    CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY(PATCH),
                })
            );

            return *s_methodAwsReverseLookup;
        }

#undef CLOUD_CANVAS_HTTP_METHOD_AWS_ENTRY

#define CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(x) { #x, HttpRequestJob::HttpMethod::HTTP_##x }

        using MethodStringReverseLookup = AZStd::unordered_map<AZStd::string, HttpRequestJob::HttpMethod>;

        AZ::EnvironmentVariable<MethodStringReverseLookup> s_methodStringReverseLookup = nullptr;
        const MethodStringReverseLookup& GetMethodStringReverseLookup()
        {
            CLOUD_CANVAS_ONCE(s_methodStringReverseLookup = AZ::Environment::CreateVariable<MethodStringReverseLookup>("methodstringreverselookup.httprequestjob.cloudcanvas", MethodStringReverseLookup
                {
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(GET),
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(POST),
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(DELETE),
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(PUT),
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(HEAD),
                    CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY(PATCH),
                })
            );

            return *s_methodStringReverseLookup;
        }

#undef CLOUD_CANVAS_HTTP_METHOD_STRING_ENTRY

#define CLOUD_CANVAS_HEADER_FIELD_ENTRY(x)  { HttpRequestJob::HeaderField::x, Aws::Http::x##_HEADER }

        using HeaderLookup = AZStd::unordered_map<HttpRequestJob::HeaderField, AZStd::string>;

        AZ::EnvironmentVariable<HeaderLookup> s_headerLookup = nullptr;
        const HeaderLookup& GetHeaderLookup()
        {
            CLOUD_CANVAS_ONCE(s_headerLookup = AZ::Environment::CreateVariable<HeaderLookup>("headerlookup.httprequestjob.cloudcanvas", HeaderLookup
                {
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(DATE),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(AWS_DATE),
                    { HttpRequestJob::HeaderField::AWS_SECURITY_TOKEN, Aws::Http::AWS_SECURITY_TOKEN },
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(ACCEPT),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(ACCEPT_CHAR_SET),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(ACCEPT_ENCODING),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(AUTHORIZATION),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(AWS_AUTHORIZATION),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(COOKIE),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(CONTENT_LENGTH),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(CONTENT_TYPE),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(USER_AGENT),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(VIA),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(HOST),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(AMZ_TARGET),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(X_AMZ_EXPIRES),
                    CLOUD_CANVAS_HEADER_FIELD_ENTRY(CONTENT_MD5),
                })
            );

            return *s_headerLookup;
        }

#undef CLOUD_CANVAS_HEADER_FIELD_ENTRY
#undef CLOUD_CANVAS_ONCE_PASTE
#undef CLOUD_CANVAS_ONCE

        template<typename MapT>
        inline const typename MapT::mapped_type* FindInMap(const MapT& haystack, const typename MapT::key_type& needle)
        {
            const typename MapT::mapped_type* result = nullptr;
            auto itr = haystack.find(needle);

            if (itr != haystack.end())
            {
                result = &itr->second;
            }

            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // HttpRequestJob methods
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void HttpRequestJob::StaticInit()
    {
        GetMethodLookup();
        GetMethodAwsReverseLookup();
        GetMethodStringReverseLookup();
        GetHeaderLookup();
    }

    void HttpRequestJob::StaticShutdown()
    {
        s_methodLookup.Reset();
        s_methodAwsReverseLookup.Reset();
        s_methodStringReverseLookup.Reset();
        s_headerLookup.Reset();
    }

    void HttpRequestJob::SetUrl(AZStd::string url)
    {
        m_url = url;
    }

    const AZStd::string& HttpRequestJob::GetUrl() const
    {
        return m_url;
    }

    void HttpRequestJob::SetMethod(HttpMethod method)
    {
        m_method = method;
    }

    bool HttpRequestJob::SetMethod(const AZStd::string& method)
    {
        bool result = false;

        if (AZStd::optional<HttpMethod> value = StringToHttpMethod(method))
        {
            SetMethod(*value);
            result = true;
        }

        return result;
    }

    HttpRequestJob::HttpMethod HttpRequestJob::GetMethod() const
    {
        return m_method;
    }

    void HttpRequestJob::SetRequestHeader(AZStd::string key, AZStd::string value)
    {
        m_requestHeaders.emplace(AZStd::move(key), AZStd::move(value));
    }

    bool HttpRequestJob::GetRequestHeader(const AZStd::string& key, AZStd::string* result)
    {
        bool found = false;
        auto itr = m_requestHeaders.find(key);

        if (itr != m_requestHeaders.end())
        {
            found = true;

            if (result)
            {
                *result = itr->second;
            }
        }

        return found;
    }

    void HttpRequestJob::SetRequestHeader(HeaderField field, AZStd::string value)
    {
        if (auto headerString = FindInMap(GetHeaderLookup(), field))
        {
            SetRequestHeader(*headerString, value);
        }
    }

    bool HttpRequestJob::GetRequestHeader(HeaderField field, AZStd::string* result)
    {
        bool found = false;

        if (auto headerString = FindInMap(GetHeaderLookup(), field))
        {
            found = GetRequestHeader(*headerString, result);
        }

        return found;
    }

    HttpRequestJob::StringMap& HttpRequestJob::GetRequestHeaders()
    {
        return m_requestHeaders;
    }

    const HttpRequestJob::StringMap& HttpRequestJob::GetRequestHeaders() const
    {
        return m_requestHeaders;
    }

    void HttpRequestJob::SetAccept(AZStd::string accept)
    {
        SetRequestHeader(HeaderField::ACCEPT, accept);
    }

    void HttpRequestJob::SetAcceptCharSet(AZStd::string acceptCharSet)
    {
        SetRequestHeader(HeaderField::ACCEPT_CHAR_SET, acceptCharSet);
    }

    void HttpRequestJob::SetContentLength(AZStd::string contentLength)
    {
        SetRequestHeader(HeaderField::CONTENT_LENGTH, contentLength);
    }

    void HttpRequestJob::SetContentType(AZStd::string contentType)
    {
        SetRequestHeader(HeaderField::CONTENT_TYPE, contentType);
    }

    void HttpRequestJob::SetAWSAuthSigner(const std::shared_ptr<Aws::Client::AWSAuthSigner>& authSigner)
    {
        m_awsAuthSigner = authSigner;
    }

    const std::shared_ptr<Aws::Client::AWSAuthSigner>& HttpRequestJob::GetAWSAuthSigner() const
    {
        return m_awsAuthSigner;
    }

    void HttpRequestJob::SetBody(AZStd::string body)
    {
        m_requestBody = std::move(body);
    }

    const AZStd::string& HttpRequestJob::GetBody() const
    {
        return m_requestBody;
    }

    AZStd::string& HttpRequestJob::GetBody()
    {
        return m_requestBody;
    }

    const char* HttpRequestJob::HttpMethodToString(HttpMethod method)
    {
        const char* result = nullptr;

        if (auto methodInfo = FindInMap(GetMethodLookup(), method))
        {
            result = methodInfo->m_name.c_str();
        }

        return result;
    }

    const char* HttpRequestJob::HttpMethodToString(Aws::Http::HttpMethod method)
    {
        const char* result = nullptr;

        if (auto convertedMethod = FindInMap(GetMethodAwsReverseLookup(), method))
        {
            result = HttpMethodToString(*convertedMethod);
        }

        return result;
    }

    AZStd::optional<HttpRequestJob::HttpMethod> HttpRequestJob::StringToHttpMethod(const AZStd::string& method)
    {
        AZStd::optional<HttpRequestJob::HttpMethod> result;
        const auto& haystack = GetMethodStringReverseLookup();
        auto itr = haystack.find(method);

        if (itr != haystack.end())
        {
            result = itr->second;
        }

        return result;
    }

    void HttpRequestJob::Process()
    {
        // Someday the AWS Http client may support real async I/O. The 
        // GetRequest and OnResponse methods are designed with that in 
        ///mind. When that feature is available, we can use the AZ::Job 
        // defined IncrementDependentCount method, start the async i/o, 
        // and call WaitForChildren. When the i/o completes, we would call
        // DecrementDependentCount, which would cause WaitForChildren to 
        // return. We would then call OnResponse.

        // Create the request
        std::shared_ptr<Aws::Http::HttpRequest> request = InitializeRequest();
        std::shared_ptr<Aws::Http::HttpResponse> httpResponse;

        if (request)
        {
            // Populate headers
            for (const auto& header : m_requestHeaders)
            {
                request->SetHeaderValue(Util::ToAwsString(header.first), Util::ToAwsString(header.second));
            }

            // Populate the body
            if (!m_requestBody.empty())
            {
                auto body = std::make_shared<Aws::StringStream>();
                body->write(m_requestBody.c_str(), m_requestBody.length());
                request->AddContentBody(body);
            }

            // Allow descendant classes to modify the request if desired
            this->CustomizeRequest(request);

            // Sign the request
            if (m_awsAuthSigner)
            {
                m_awsAuthSigner->SignRequest(*request);
            }

            httpResponse = m_httpClient->MakeRequest(*request.get(), m_readRateLimiter.get(), m_writeRateLimiter.get());
        }

        // Allow descendant classes to process the response
        this->ProcessResponse(httpResponse);

        // Configure and deliver our response
        auto callbackResponse = AZStd::make_shared<Response>();
        callbackResponse->m_response = httpResponse;
        bool failure = true;

        if (httpResponse)
        {
            Aws::IOStream& responseBody = httpResponse->GetResponseBody();
            std::istreambuf_iterator<AZStd::string::value_type> eos;
            callbackResponse->m_responseBody = AZStd::string{ std::istreambuf_iterator<AZStd::string::value_type>(responseBody), eos };
            callbackResponse->m_responseCode = static_cast<int>(httpResponse->GetResponseCode());

            if (callbackResponse->m_responseCode >= 200 && callbackResponse->m_responseCode <= 299)
            {
                failure = false;

                if (m_successCallback)
                {
                    auto callback = AZStd::make_shared<SuccessFn>(AZStd::move(m_successCallback));
                    auto fn = AZStd::function<void()>([callbackResponse, callback]() { (*callback)(callbackResponse); });
                    EBUS_QUEUE_FUNCTION(AZ::TickBus, fn);
                }
            }
        }

        if (failure && m_failureCallback)
        {
            auto callback = AZStd::make_shared<SuccessFn>(AZStd::move(m_failureCallback));
            auto fn = AZStd::function<void()>([callbackResponse, callback]() { (*callback)(callbackResponse); });
            EBUS_QUEUE_FUNCTION(AZ::TickBus, fn);
        }
    }

    std::shared_ptr<Aws::Http::HttpRequest> HttpRequestJob::InitializeRequest()
    {
        std::shared_ptr<Aws::Http::HttpRequest> result;
        auto methodInfo = FindInMap(GetMethodLookup(), m_method);

        if (!m_url.empty() && methodInfo)
        {
            result = Aws::Http::CreateHttpRequest(
                Util::ToAwsString(m_url),
                methodInfo->m_awsMethod,
                &Aws::Utils::Stream::DefaultResponseStreamFactoryMethod
            );
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // HttpRequestJob::Response methods
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const AZStd::string& HttpRequestJob::Response::GetResponseBody() const
    {
        return m_responseBody;
    }

    int HttpRequestJob::Response::GetResponseCode() const
    {
        return m_responseCode;
    }

    const std::shared_ptr<Aws::Http::HttpResponse>& HttpRequestJob::Response::GetUnderlyingResponse() const
    {
        return m_response;
    }


} // namespace CloudGemFramework
