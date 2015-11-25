
#include "catch.hpp"
#include "railway.h"

TEST_CASE("instantiable", "[railway]") {
//  railway::track("mir.intstl.com", "qotd");
}

#if define(__MINGW32__) || defined(_MSC_VER)
TEST_CASE("doesnt't support unix sockets", "[unix-sockets]") {
  REQUIRE(railway::track::supports_unix_sockets() == false);
}

TEST_CASE("throws when requesting unix sockets anyways", "[unix-sockets]") {
  REQUIRE_THROWS(railway::track("/tmp/socket"));
}
#else
TEST_CASE("supports unix sockets", "[unix-sockets]") {
  REQUIRE(railway::track::supports_unix_sockets() == true);
}

TEST_CASE("throws when path is NULL", "[unix-socket]") {
  REQUIRE_THROWS(railway::track(NULL));
}
#endif

