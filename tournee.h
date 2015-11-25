/*
 * RAILWAY v0.1
 *
 * Copyright (c) 2015, Jonas Weber
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*/

#ifndef TOURNEE_H
#define TOURNEE_H 

#include <mutex>
#include <functional>
#include "railyway.h"

namespace railway {
  template <typename Track = track, size_t BUFFER_SIZE = 512>
  class tournee {
    private:
      const Track& t;
      std::mutex& t_mut;

    public:
      tournee(const Track& t, std::mutex& t_mut)
        : t(t), t_mut(t_mut) {
      };

      static size_t no_upload_data(char* out_buf, size_t bytes) {
        return 0;
      }

      static size_t discard_data(char* in_buf, size_t bytes) {
        return bytes;
      }

      class HeaderItem {
        private:
          std::string
        public:
          HeaderItem(const std::string& key, const std::string value)
            : key(key), value(value) {
          }


      }

      class Request {
        private:
          const std::string method;
          const std::string path;
          const std::initializer_list<
      };
      
      perform(const std::string& method, const std::initializer_list<std::initializer_list<std::string>>& headers, std::function<size_t(char*,size_t)> reader = no_upload_data, std::function<size_t(char*,size_t)> writer = discard_data) {
        auto lck = std::lock_guard { t_mut };



      }
  };
}

#endif
