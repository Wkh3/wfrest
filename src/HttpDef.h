//
// Created by Chanchan on 11/5/21.
//

#ifndef _HTTPDEF_H_
#define _HTTPDEF_H_

#include <string>

namespace wfrest
{

// Taken from libhv : https://github.com/ithewei/libhv
// MIME: https://www.iana.org/assignments/media-types/media-types.xhtml
// http_content_type
// XX(name, mime, suffix)
#define HTTP_CONTENT_TYPE_MAP(XX) \
    XX(TEXT_PLAIN,              text/plain,               txt)          \
    XX(TEXT_HTML,               text/html,                html)         \
    XX(TEXT_CSS,                text/css,                 css)          \
    XX(IMAGE_JPEG,              image/jpeg,               jpg)          \
    XX(IMAGE_PNG,               image/png,                png)          \
    XX(IMAGE_GIF,               image/gif,                gif)          \
    XX(IMAGE_BMP,               image/bmp,                bmp)          \
    XX(IMAGE_SVG,               image/svg,                svg)          \
    XX(APPLICATION_OCTET_STREAM,application/octet-stream, bin)          \
    XX(APPLICATION_JAVASCRIPT,  application/javascript,   js)           \
    XX(APPLICATION_XML,         application/xml,          xml)          \
    XX(APPLICATION_JSON,        application/json,         json)         \
    XX(APPLICATION_GRPC,        application/grpc,         grpc)         \
    XX(APPLICATION_URLENCODED,  application/x-www-form-urlencoded, kv)  \
    XX(MULTIPART_FORM_DATA,     multipart/form-data,               mp)  \

#define X_WWW_FORM_URLENCODED   APPLICATION_URLENCODED // for compatibility

    enum http_content_type
    {
#define XX(name, string, suffix)   name,
        CONTENT_TYPE_NONE,
        HTTP_CONTENT_TYPE_MAP(XX)
        CONTENT_TYPE_UNDEFINED
#undef XX
    };

    class ContentType
    {
    public:
        static std::string to_string(enum http_content_type type);

        static enum http_content_type to_enum(const std::string &content_type_str);
    };


} // wfrest



#endif //_HTTPDEF_H_