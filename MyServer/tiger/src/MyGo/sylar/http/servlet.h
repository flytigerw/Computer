#ifndef __SYLAR_HTTP_SERVLET_H__
#define __SYLAR_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../thread.h"
#include "../util.h"

///每一个新的客户端连接都会创建一个servlet为其服务
namespace sylar {
namespace http {

class Servlet {
public:
    typedef std::shared_ptr<Servlet> ptr;
    Servlet(const std::string& name)
        :m_name(name) {}

    virtual ~Servlet() {}

    //为servlet添加url的handler，当命中某个url时，执行servlet
    /**
     * @brief 处理请求
     * @param[in] request HTTP请求
     * @param[in] response HTTP响应
     * @param[in] session HTTP连接
     * @return 是否处理成功
     */
    virtual int32_t handle(sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) = 0;
                   
    
    const std::string& getName() const { return m_name;}
protected:
    /// 名称
    std::string m_name;
};

//函数式servlet:成员为回调函数
class FunctionServlet : public Servlet {
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t (sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session)> callback;


    FunctionServlet(callback cb);
    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) override;
private:
    /// 回调函数
    callback m_cb;
};

//servlet creator:创建出servlet作为自己数据成员，外部通过get获取
class IServletCreator {
public:
    typedef std::shared_ptr<IServletCreator> ptr;
    virtual ~IServletCreator() {}
    virtual Servlet::ptr get() const = 0;
    virtual std::string getName() const = 0;
};

class HoldServletCreator : public IServletCreator {
public:
    typedef std::shared_ptr<HoldServletCreator> ptr;
    HoldServletCreator(Servlet::ptr slt)
        :m_servlet(slt) {
    }

    Servlet::ptr get() const override {
        return m_servlet;
    }

    std::string getName() const override {
        return m_servlet->getName();
    }
private:
    Servlet::ptr m_servlet;
};

template<class T>
class ServletCreator : public IServletCreator {
public:
    typedef std::shared_ptr<ServletCreator> ptr;

    ServletCreator() {
    }

    Servlet::ptr get() const override {
        return Servlet::ptr(new T);
    }

    std::string getName() const override {
        return TypeToName<T>();
    }
};

//维护url和servlet的映射,为每一个连接过来的url选择合适的servlet
class ServletDispatch : public Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<ServletDispatch> ptr;
    /// 读写锁类型定义
    typedef RWMutex RWMutexType;

    /**
     * @brief 构造函数
     */
    ServletDispatch();
    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) override;

    void addServlet(const std::string& uri, Servlet::ptr slt);

    //通过回调函数进行添加
    void addServlet(const std::string& uri, FunctionServlet::callback cb);

    //添加模糊匹配servlet
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);

    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
    void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

    template<class T>
    void addServletCreator(const std::string& uri) {
        addServletCreator(uri, std::make_shared<ServletCreator<T> >());
    }

    template<class T>
    void addGlobServletCreator(const std::string& uri) {
        addGlobServletCreator(uri, std::make_shared<ServletCreator<T> >());
    }

    /**
     * @brief 删除servlet
     * @param[in] uri uri
     */
    void delServlet(const std::string& uri);

    /**
     * @brief 删除模糊匹配servlet
     * @param[in] uri uri
     */
    void delGlobServlet(const std::string& uri);

    /**
     * @brief 返回默认servlet
     */
    Servlet::ptr getDefault() const { return m_default;}

    /**
     * @brief 设置默认servlet
     * @param[in] v servlet
     */
    void setDefault(Servlet::ptr v) { m_default = v;}


    //通过uri获取servlet
    Servlet::ptr getServlet(const std::string& uri);

    /**
     * @brief 通过uri获取模糊匹配servlet
     * @param[in] uri uri
     * @return 返回对应的servlet
     */
    Servlet::ptr getGlobServlet(const std::string& uri);

    /**
     * @brief 通过uri获取servlet
     * @param[in] uri uri
     * @return 优先精准匹配,其次模糊匹配,最后返回默认
     */
    Servlet::ptr getMatchedServlet(const std::string& uri);

    void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
    void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
private:
    /// 读写互斥量
    RWMutexType m_mutex;
    /// 精准匹配servlet MAP
    /// uri(/sylar/xxx) -> servlet
    std::unordered_map<std::string, IServletCreator::ptr> m_datas;
    /// 模糊匹配servlet 数组
    /// uri(/sylar/*) -> servlet
    std::vector<std::pair<std::string, IServletCreator::ptr> > m_globs;
    /// 默认servlet，所有路径都没匹配到时使用
    Servlet::ptr m_default;
};

/**
 * @brief NotFoundServlet(默认返回404)
 */
class NotFoundServlet : public Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<NotFoundServlet> ptr;
    /**
     * @brief 构造函数
     */
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) override;

private:
    std::string m_name;
    std::string m_content;
};

}
}

#endif
