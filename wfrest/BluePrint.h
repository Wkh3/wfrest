#ifndef WFREST_BLUEPRINT_H_
#define WFREST_BLUEPRINT_H_

#include <functional>
#include <utility>
#include "wfrest/Noncopyable.h"
#include "wfrest/Aspect.h"
#include "wfrest/AopUtil.h"

// todo : hide
#include "wfrest/Router.h"
#include "wfrest/HttpServerTask.h" 

class SeriesWork;

namespace wfrest
{
using Handler = std::function<void(const HttpReq *, HttpResp *)>;
using SeriesHandler = std::function<void(const HttpReq *, HttpResp *, SeriesWork *)>;

class BluePrint : public Noncopyable
{
public:
    void GET(const char *route, const Handler &handler);

    void GET(const char *route, int compute_queue_id, const Handler &handler);

    void POST(const char *route, const Handler &handler);

    void POST(const char *route, int compute_queue_id, const Handler &handler);

public:
    template<typename... AP>
    void GET(const char *route, const Handler &handler, const AP &... ap);

    template<typename... AP>
    void GET(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap);

    template<typename... AP>
    void POST(const char *route, const Handler &handler, const AP &... ap);

    template<typename... AP>
    void POST(const char *route, int compute_queue_id,
              const Handler &handler, const AP &... ap);

public:
    void GET(const char *route, const SeriesHandler &handler);

    void GET(const char *route, int compute_queue_id, const SeriesHandler &handler);

    void POST(const char *route, const SeriesHandler &handler);

    void POST(const char *route, int compute_queue_id, const SeriesHandler &handler);  

public:
    template<typename... AP>
    void GET(const char *route, const SeriesHandler &handler, const AP &... ap);

    template<typename... AP>
    void GET(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap);

    template<typename... AP>
    void POST(const char *route, const SeriesHandler &handler, const AP &... ap);

    template<typename... AP>
    void POST(const char *route, int compute_queue_id,
              const SeriesHandler &handler, const AP &... ap);

public:
    const Router &router() const
    { return router_; }

    void add_blueprint(const BluePrint &bp, const std::string &url_prefix);

private:
    template<typename T>
    void Use(T&& t)
    {
        Aspect *asp = new T(std::move(t));
        global_aspect_.push_back(asp);
    }

private:
    Router router_;    // ptr for hiding internel class
    std::vector<Aspect *> global_aspect_;

    friend class HttpServer;
};


} // namespace wfrest

#include "wfrest/BluePrint.inl"

#endif // WFREST_BLUEPRINT_H_