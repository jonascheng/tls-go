#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>

namespace cc {
  struct Header {
    std::string key;
    std::string value;

    Header() {
      key = "";
      value = "";
    }
  };

  std::string lstrip(std::string s) {
    int p = 0;
    while (s[p] == ' ' || s[p] == '\n' || s[p] == '\r') {
      ++p;
    }
    return s.substr(p, s.size() - p);
  }

  std::string rstrip(std::string s) {
    int p = s.size() - 1;
    while (s[p] == ' ' || s[p] == '\n' || s[p] == '\r') {
      --p;
    }
    return s.substr(0, p + 1);
  }

  std::string strip(std::string s) {
    return lstrip(rstrip(s));
  }

  void to_lower(std::string s) {
      std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) -> unsigned char {
          std::tolower(c); return c;
        }
      );
  }

  Header* parse_header(std::string header) {
    Header *h = new Header;
    int p = header.find_first_of(':', 0);
    if (p >= header.size()) {
      return NULL;
    }
    h->key = strip(header.substr(0, p));
    h->value = strip(header.substr(p + 1, header.size() - p + 1));

    to_lower(h->key);
    return h;
  }

  curl_slist* get_user_agent(std::string user_agent) {
    struct curl_slist *chunk = NULL;
    char *ua = new char[30];
    sprintf(ua, "User-Agent: %s", user_agent.c_str());
    chunk = curl_slist_append(chunk, ua);

    return chunk;
  }

  std::vector<std::string> get_cookie_vector(struct curl_slist *each) {
    std::vector<std::string> v;
    while (each) {
      v.push_back(each->data);
      each = each->next;
    }

    return v;
  }

  curl_slist* get_header_list(std::map<std::string, std::string> h) {
    curl_slist *list = NULL;
    for (auto it : h) {
      char *s = new char[it.first.size() + it.second.size() + 3];
      sprintf(s, "%s: %s", it.first.c_str(), it.second.c_str());
      list = curl_slist_append(list, s);
    }

    return list;
  }

}