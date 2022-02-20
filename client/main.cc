#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>

#include <curl/curl.h>
#include "utils.h"

struct Context
{
  CURL *req;
  std::map<std::string, std::string> _headers;
  struct curl_slist *headers;
};

/*
This function will be called for every header line and it will be terminated
with "\n\r" as received.
Usage: curl_easy_setopt(req, CURLOPT_HEADERFUNCTION, header_callback);
*/
size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  Context *ctx = (Context *)userdata;
  cc::Header *h = cc::parse_header(std::string(buffer));
  if (h != NULL)
  {
    ctx->_headers[h->key] = h->value;
  }

  return size * nitems;
}

void base_request_handle(Context *ctx, std::string host, void *out, long verbose)
{
  CURLcode result;
  ctx->req = curl_easy_init();

  if (ctx->req == NULL)
  {
    fprintf(stderr, "Failed to create a request object\n");
    exit(-1);
  }

  curl_easy_setopt(ctx->req, CURLOPT_URL, host.c_str());
  curl_easy_setopt(ctx->req, CURLOPT_VERBOSE, verbose);
  curl_easy_setopt(ctx->req, CURLOPT_WRITEDATA, out);
  curl_easy_setopt(ctx->req, CURLOPT_HEADERDATA, (void *)ctx);
  curl_easy_setopt(ctx->req, CURLOPT_HEADERFUNCTION, header_callback);

  // struct curl_slist *chunk = NULL;
  // chunk = curl_slist_append(chunk, "User-Agent: curl/7.54.0");
  curl_easy_setopt(ctx->req, CURLOPT_HTTPHEADER, cc::get_user_agent("curl/7.54.0"));

  curl_easy_setopt(ctx->req, CURLOPT_FOLLOWLOCATION, CURLPROTO_ALL);
  curl_easy_setopt(ctx->req, CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST);
  curl_easy_setopt(ctx->req, CURLOPT_SSL_VERIFYHOST, 1);
  curl_easy_setopt(ctx->req, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
  curl_easy_setopt(ctx->req, CURLOPT_CAINFO, "server.crt");
}

void post_request_handle(Context *ctx, std::string host, std::string data,
                         std::map<std::string, std::string> m, void *out, long verbose)
{

  base_request_handle(ctx, host, out, verbose);
  curl_easy_setopt(ctx->req, CURLOPT_POST, 1L);
  curl_easy_setopt(ctx->req, CURLOPT_POSTFIELDSIZE, data.size());

  // Using CURLOPT_COPYPOSTFIELDS to make sure that the local payload offered
  // by this function is copied by curl, otherwise it will be evicted when
  // this function exits and garbage value will be read from the memory.
  curl_easy_setopt(ctx->req, CURLOPT_COPYPOSTFIELDS, data.c_str());

  ctx->headers = cc::get_header_list(m);
  curl_easy_setopt(ctx->req, CURLOPT_HTTPHEADER, ctx->headers);
}

void cleanup_request_handle(Context *ctx)
{
  if (ctx->headers)
    curl_slist_free_all(ctx->headers);
  if (ctx->req)
    curl_easy_cleanup(ctx->req);
}

void http_post(std::string host, std::string payload)
{
  struct Context *ctx = new Context;
  CURLcode result = CURLE_OK;

  FILE *out = fopen("/dev/null", "w");
  std::map<std::string, std::string> h;
  h["Content-Type"] = "application/json";

  post_request_handle(ctx, host, payload, h, stdout, 1);
  result = curl_easy_perform(ctx->req);

  if (result != CURLE_OK)
  {
    fprintf(stderr, "Failed to make a GET request: %d \n", result);
    exit(result);
  }

  fclose(out);
  cleanup_request_handle(ctx);
}

int main(int argc, char **argv)
{
  curl_global_init(CURL_GLOBAL_DEFAULT);

  http_post("https://localhost/hello", "{\"payload\":\"hello world\"}");
  return 0;
}