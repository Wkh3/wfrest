#include "workflow/WFTaskFactory.h"

namespace wfrest
{

namespace detail
{

// In order to reduce the use of generic programming, add some redundant code
template<typename Tuple>
WFGoTask *aop_process(const Handler &handler,
                      const HttpReq *req,
                      HttpResp *resp,
                      Tuple *tp)
{
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        return nullptr;
    }
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr;
    }
    handler(req, resp);
    HttpServerTask *server_task = task_of(resp);
    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        aop_after(req, resp, *tp);
        for(auto asp : global_aspect->aspect_list)
        {
            asp->after(req, resp);
        }
        delete tp;
    });

    return nullptr;
}

template<typename Tuple>
WFGoTask *aop_process(const SeriesHandler &handler,
                      const HttpReq *req,
                      HttpResp *resp,
                      SeriesWork *series,
                      Tuple *tp)
{
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        return nullptr;
    }
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr;
    }
    handler(req, resp, series);

    HttpServerTask *server_task = task_of(resp);

    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        aop_after(req, resp, *tp);
        for(auto asp : global_aspect->aspect_list)
        {
            asp->after(req, resp);
        }
        delete tp;
    });

    return nullptr;
}

template<typename Tuple>
WFGoTask *aop_compute_process(const Handler &handler,
                              int compute_queue_id,
                              const HttpReq *req,
                              HttpResp *resp,
                              Tuple *tp)
{
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        return nullptr;
    }
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr;
    }
    WFGoTask *go_task = WFTaskFactory::create_go_task(
            "wfrest" + std::to_string(compute_queue_id),
            handler,
            req,
            resp);

    HttpServerTask *server_task = task_of(resp);

    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        aop_after(req, resp, *tp);
        for(auto asp : global_aspect->aspect_list)
        {
            asp->after(req, resp);
        }
        delete tp;
    });

    return go_task;
}


template<typename Tuple>
WFGoTask *aop_compute_process(const SeriesHandler &handler,
                              int compute_queue_id,
                              const HttpReq *req,
                              HttpResp *resp,
                              SeriesWork *series,
                              Tuple *tp)
{
    bool ret = aop_before(req, resp, *tp);
    if (!ret)
    {
        return nullptr;
    }
    GlobalAspect *global_aspect = GlobalAspect::get_instance();
    for(auto asp : global_aspect->aspect_list)
    {
        ret = asp->before(req, resp);
        if(!ret) return nullptr;
    }
    WFGoTask *go_task = WFTaskFactory::create_go_task(
            "wfrest" + std::to_string(compute_queue_id),
            handler,
            req,
            resp,
            series);

    HttpServerTask *server_task = task_of(resp);

    server_task->add_callback([req, resp, tp, global_aspect](HttpTask *) 
    {
        aop_after(req, resp, *tp);
        for(auto asp : global_aspect->aspect_list)
        {
            asp->after(req, resp);
        }
        delete tp;
    });
    
    return go_task;
}

}  // namespace detail

template<typename... AP>
void BluePrint::Handle(const char *route, const Handler &handler, 
            Verb verb, const AP &... ap)
{
    WrapHandler wrap_handler =
            [handler, this, ap...](const HttpReq *req,
                             HttpResp *resp,
                             SeriesWork *) -> WFGoTask *
            {
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                WFGoTask *go_task = detail::aop_process(handler,
                                                        req,
                                                        resp,
                                                        tp);
                return go_task;
            };

    router_.handle(route, -1, wrap_handler, verb);
}

template<typename... AP>
void BluePrint::Handle(const char *route, int compute_queue_id, 
            const Handler &handler, Verb verb, const AP &... ap)
{
    WrapHandler wrap_handler =
            [handler, compute_queue_id, this, ap...](HttpReq *req,
                                               HttpResp *resp,
                                               SeriesWork *) -> WFGoTask *
            {
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                WFGoTask *go_task = detail::aop_compute_process(handler,
                                                                compute_queue_id,
                                                                req,
                                                                resp,
                                                                tp);
                return go_task;
            };

    router_.handle(route, compute_queue_id, wrap_handler, verb);
}

template<typename... AP>
void BluePrint::Handle(const char *route, const Handler &handler, 
            const std::vector<std::string> &methods, const AP &... ap)
{
    for(const auto &method : methods)
    {
        this->Handle(route, handler, str_to_verb(method), ap...);
    }
}

template<typename... AP>
void BluePrint::Handle(const char *route, int compute_queue_id, 
            const Handler &handler, 
            const std::vector<std::string> &methods, const AP &... ap)
{
    for(const auto &method : methods)
    {
        this->Handle(route, compute_queue_id, handler, str_to_verb(method), ap...);
    } 
}

template<typename... AP>
void BluePrint::GET(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::GET, ap...);
}

template<typename... AP>
void BluePrint::GET(const char *route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::GET, ap...);
}

template<typename... AP>
void BluePrint::POST(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::POST, ap...);
}

template<typename... AP>
void BluePrint::POST(const char *route, int compute_queue_id,
                     const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::POST, ap...);
}

template<typename... AP>
void BluePrint::DELETE(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::DELETE, ap...);
}

template<typename... AP>
void BluePrint::DELETE(const char *route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::DELETE, ap...);
}

template<typename... AP>
void BluePrint::PATCH(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::PATCH, ap...);
}

template<typename... AP>
void BluePrint::PATCH(const char *route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::PATCH, ap...);
}

template<typename... AP>
void BluePrint::PUT(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::PUT, ap...);
}

template<typename... AP>
void BluePrint::PUT(const char *route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::PUT, ap...);
}

template<typename... AP>
void BluePrint::HEAD(const char *route, const Handler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::HEAD, ap...);
}

template<typename... AP>
void BluePrint::HEAD(const char *route, int compute_queue_id,
                    const Handler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::HEAD, ap...);
}

template<typename... AP>
void BluePrint::Handle(const char *route, const SeriesHandler &handler, 
            Verb verb, const AP &... ap)
{
    WrapHandler wrap_handler =
            [handler, this, ap...](const HttpReq *req,
                             HttpResp *resp,
                             SeriesWork *series) -> WFGoTask *
            {
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                WFGoTask *go_task = detail::aop_process(handler,
                                                        req,
                                                        resp,
                                                        series,
                                                        tp);
                return go_task;
            };

    router_.handle(route, -1, wrap_handler, verb);
}

template<typename... AP>
void BluePrint::Handle(const char *route, int compute_queue_id, 
            const SeriesHandler &handler, Verb verb, const AP &... ap)
{
    WrapHandler wrap_handler =
            [handler, compute_queue_id, this, ap...](HttpReq *req,
                                               HttpResp *resp,
                                               SeriesWork *series) -> WFGoTask *
            {
                auto *tp = new std::tuple<AP...>(std::move(ap)...);
                WFGoTask *go_task = detail::aop_compute_process(handler,
                                                                compute_queue_id,
                                                                req,
                                                                resp,
                                                                series,
                                                                tp);
                return go_task;
            };

    router_.handle(route, compute_queue_id, wrap_handler, verb);  
}

template<typename... AP>
void BluePrint::Handle(const char *route, const SeriesHandler &handler, 
            const std::vector<std::string> &methods, const AP &... ap)
{
    for(const auto &method : methods)
    {
        this->Handle(route, handler, str_to_verb(method), ap...);
    } 
}

template<typename... AP>
void BluePrint::Handle(const char *route, int compute_queue_id, 
            const SeriesHandler &handler, 
            const std::vector<std::string> &methods, const AP &... ap)
{
    for(const auto &method : methods)
    {
        this->Handle(route, compute_queue_id, handler, str_to_verb(method), ap...);
    } 
}

template<typename... AP>
void BluePrint::GET(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::GET, ap...);
}

template<typename... AP>
void BluePrint::GET(const char *route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    this->Handle(route, compute_queue_id, handler, Verb::GET, ap...);
}

template<typename... AP>
void BluePrint::POST(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::POST, ap...);
}

template<typename... AP>
void BluePrint::POST(const char *route, int compute_queue_id,
                     const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, compute_queue_id, handler, Verb::POST, ap...);
}

template<typename... AP>
void BluePrint::DELETE(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::DELETE, ap...);
}

template<typename... AP>
void BluePrint::DELETE(const char *route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    this->Handle(route, compute_queue_id, handler, Verb::DELETE, ap...);
}

template<typename... AP>
void BluePrint::PATCH(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::PATCH, ap...);
}

template<typename... AP>
void BluePrint::PATCH(const char *route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    this->Handle(route, compute_queue_id, handler, Verb::PATCH, ap...);
}

template<typename... AP>
void BluePrint::PUT(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::PUT, ap...);
}

template<typename... AP>
void BluePrint::PUT(const char *route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    this->Handle(route, compute_queue_id, handler, Verb::PUT, ap...);
}

template<typename... AP>
void BluePrint::HEAD(const char *route, const SeriesHandler &handler, const AP &... ap)
{
    this->Handle(route, handler, Verb::HEAD, ap...);
}

template<typename... AP>
void BluePrint::HEAD(const char *route, int compute_queue_id,
                    const SeriesHandler &handler, const AP &... ap)
{   
    this->Handle(route, compute_queue_id, handler, Verb::HEAD, ap...);
}

} // namespace wfrest





