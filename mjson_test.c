#include <assert.h>
#include <stdio.h>

#include "mjson.h"

static void test_cb(void) {
  {
    const char *s = "{\"a\": true, \"b\": [ null, 3 ]}";
    assert(mjson(s, strlen(s), NULL, NULL) == (int) strlen(s));
  }
  {
    const char *s = "[ 1, 2 ,  null, true,false,\"foo\"  ]";
    assert(mjson(s, strlen(s), NULL, NULL) == (int) strlen(s));
  }
  {
    const char *s = "123";
    assert(mjson(s, strlen(s), NULL, NULL) == (int) strlen(s));
  }
  {
    const char *s = "\"foo\"";
    assert(mjson(s, strlen(s), NULL, NULL) == (int) strlen(s));
  }
  {
    const char *s = "123 ";  // Trailing space
    assert(mjson(s, strlen(s), NULL, NULL) == (int) strlen(s) - 1);
  }
  {
    const char *s = "[[[[[[[[[[[[[[[[[[[[[";
    assert(mjson(s, strlen(s), NULL, NULL) == MJSON_ERROR_TOO_DEEP);
  }

  assert(mjson("\"abc\"", 0, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("\"abc\"", 1, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("\"abc\"", 2, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("\"abc\"", 3, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("\"abc\"", 4, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("\"abc\"", 5, NULL, NULL) == 5);

  assert(mjson("{\"a\":1}", 0, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 1, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 2, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 3, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 4, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 5, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 6, NULL, NULL) == MJSON_ERROR_INVALID_INPUT);
  assert(mjson("{\"a\":1}", 7, NULL, NULL) == 7);

  assert(mjson("{\"a\":[]}", 8, NULL, NULL) == 8);
  assert(mjson("{\"a\":{}}", 8, NULL, NULL) == 8);
  assert(mjson("[]", 2, NULL, NULL) == 2);
  assert(mjson("{}", 2, NULL, NULL) == 2);
  assert(mjson("[[]]", 4, NULL, NULL) == 4);
  assert(mjson("[[],[]]", 7, NULL, NULL) == 7);
  assert(mjson("[{}]", 4, NULL, NULL) == 4);
  assert(mjson("[{},{}]", 7, NULL, NULL) == 7);
  assert(mjson("{\"a\":[{}]}", 10, NULL, NULL) == 10);
}

static void test_find(void) {
  const char *p;
  int n;
  assert(mjson_find("", 0, "", &p, &n) == MJSON_TOK_INVALID);
  assert(mjson_find("", 0, "$", &p, &n) == MJSON_TOK_INVALID);
  assert(mjson_find("123", 3, "$", &p, &n) == MJSON_TOK_NUMBER);
  assert(n == 3 && memcmp(p, "123", 3) == 0);
  assert(mjson_find("{\"a\":true}", 10, "$.a", &p, &n) == MJSON_TOK_TRUE);
  assert(n == 4 && memcmp(p, "true", 4) == 0);
  assert(mjson_find("{\"a\":{\"c\":null},\"c\":2}", 22, "$.c", &p, &n) ==
         MJSON_TOK_NUMBER);
  assert(n == 1 && memcmp(p, "2", 1) == 0);
  assert(mjson_find("{\"a\":{\"c\":null},\"c\":2}", 22, "$.a.c", &p, &n) ==
         MJSON_TOK_NULL);
  assert(n == 4 && memcmp(p, "null", 4) == 0);
  assert(mjson_find("{\"a\":[1,null]}", 15, "$.a", &p, &n) == '[');
  assert(n == 8 && memcmp(p, "[1,null]", 8) == 0);
  assert(mjson_find("{\"a\":{\"b\":7}}", 14, "$.a", &p, &n) == '{');
  assert(n == 7 && memcmp(p, "{\"b\":7}", 7) == 0);
}

static void test_find_number(void) {
  assert(mjson_find_number("", 0, "$", 123) == 123);
  assert(mjson_find_number("234", 3, "$", 123) == 234);
  assert(mjson_find_number("{\"a\":-7}", 8, "$.a", 123) == -7);
  assert(mjson_find_number("{\"a\":1.2e3}", 11, "$.a", 123) == 1.2e3);
  assert(mjson_find_number("[1.23,-43.47,17]", 16, "$", 42) == 42);
  assert(mjson_find_number("[1.23,-43.47,17]", 16, "$[0]", 42) == 1.23);
  assert(mjson_find_number("[1.23,-43.47,17]", 16, "$[1]", 42) == -43.47);
  assert(mjson_find_number("[1.23,-43.47,17]", 16, "$[2]", 42) == 17);
  assert(mjson_find_number("[1.23,-43.47,17]", 16, "$[3]", 42) == 42);
  {
    const char *s = "{\"a1\":[1,2,{\"a2\":4},[],{}],\"a\":3}";
    assert(mjson_find_number(s, strlen(s), "$.a", 0) == 3);
  }
}

static void test_find_bool(void) {
  assert(mjson_find_bool("", 0, "$", 1) == 1);
  assert(mjson_find_bool("", 0, "$", 0) == 0);
  assert(mjson_find_bool("true", 4, "$", 0) == 1);
  assert(mjson_find_bool("false", 5, "$", 1) == 0);
}

static void test_find_string(void) {
  char buf[100];
  {
    const char *s = "{\"a\":\"f\too\"}";
    assert(mjson_find_string(s, strlen(s), "$.a", buf, sizeof(buf)) == 4);
    assert(strcmp(buf, "f\too") == 0);
  }

  {
    const char *s = "{\"ы\":\"превед\"}";
    assert(mjson_find_string(s, strlen(s), "$.ы", buf, sizeof(buf)) == 12);
    assert(strcmp(buf, "превед") == 0);
  }
}

int main() {
  test_cb();
  test_find();
  test_find_number();
  test_find_bool();
  test_find_string();
  return 0;
}