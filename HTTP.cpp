#include "curl/curl.h"
#include <string>
#include "HTTP.h"
#define THREAD_IMPLEMENTATION
#include "thread.h"
// callbacks for CURL

static size_t HTTPRequest_write_callback(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}
static size_t HTTPRequest_fwrite_callback(void *ptr, size_t size, size_t nmemb, HTTPRequest *request)
{
    if (!request)
        return 0;
    if (!request->stream)
    {
        if (request->m_filePath == "")
        {
            request->response.append((char *)ptr, size * nmemb);
            return size * nmemb;
        }
        request->stream = fopen(request->m_filePath.c_str(), "wb");
        if (!request->stream)
            return 0; // nothing else we can do
    }
    size_t result = fwrite(ptr, size, nmemb, request->stream);
    fflush(request->stream);
    return result;
}
static size_t HTTPRequest_progress_callback(HTTPRequest *req, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    req->m_bytesDownloaded = (double)dlnow;
    req->m_totalSize = (double)dltotal;
    req->m_dlpercentage = (req->m_bytesDownloaded / req->m_totalSize) * 100.0;
    return req->aborted ? 1 : 0;
}
int HTTPRequest_thread(void *rawRequest)
{
    if (!rawRequest)
        return 0;
    HTTPRequest *request = static_cast<HTTPRequest *>(rawRequest);
    if (!request)
        return 0;
    CURL *curl = curl_easy_init();
    if (curl)
    {
        request->complete = false;
        request->noCURL = false;
        request->inProgress = true;
        curl_easy_setopt(curl, CURLOPT_URL, request->url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0l);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, HTTPRequest_progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, request);
        if (request->m_filePath == "")
        {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTPRequest_write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &request->response);
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTPRequest_fwrite_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, request);
        }
        // don't need a callback for headers
        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &request->code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &request->time);
        curl_easy_cleanup(curl);
        curl = NULL;
        request->complete = true;
        request->inProgress = false;
        if (request->stream)
        {
            fclose(request->stream);
            request->stream = NULL;
        }
        return 0;
    }
    request->noCURL = true;
    return 0;
}

HTTPRequest::HTTPRequest(const std::string &url)
{
    this->url = url;
    noCURL = false;
    inProgress = false;
    complete = false;
    m_dlpercentage = 0;
    m_bytesDownloaded = 0;
    m_totalSize = 0;
    m_ulpercentage = 0;
    time = 0;
    response = "";
    m_filePath = "";
    if (!stream)
    {
        fclose(stream);
        stream = NULL;
    }
}
bool HTTPRequest::perform()
{
    if(inProgress) return false;
    if(complete) complete=false;
    if(thread_create(HTTPRequest_thread, this, THREAD_STACK_SIZE_DEFAULT)==NULL) return false;
    return true;
}
bool HTTPRequest::perform(const std::string& url)
{
    if(inProgress) return false;
    this->url=url;
return perform();
}
