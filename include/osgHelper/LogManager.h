#pragma once

#include <osg/Referenced>
#include <osg/ref_ptr>

#include <memory>
#include <string>

#define OSGH_LOG(sev, msg) osgHelper::LogManager::getInstance()->log(sev, msg)

#ifdef _DEBUG
#define OSGH_LOG_DEBUG(msg) OSGH_LOG(osgHelper::LogManager::DEBUG, msg)
#else
#define OSGH_LOG_DEBUG(msg)
#endif

#define OSGH_LOG_INFO(msg) OSGH_LOG(osgHelper::LogManager::INFO, msg)
#define OSGH_LOG_WARN(msg) OSGH_LOG(osgHelper::LogManager::WARNING, msg)
#define OSGH_LOG_FATAL(msg) OSGH_LOG(osgHelper::LogManager::FATAL, msg)

namespace osgHelper
{
  class Logger : public osg::Referenced
  {
  public:
    virtual void log(const std::string& text) = 0;
  };

  class StdOutLogger : public Logger
  {
  public:
    void log(const std::string& text) override;
  };

  class LogManager : public osg::Referenced
  {
  public:
    enum Severety
    {
      DEBUG = 0,
      INFO = 1,
      WARNING = 2,
      FATAL = 3
    };

    typedef osg::ref_ptr<LogManager> Ptr;

    static Ptr getInstance();
    static void clearInstance();

    void setMinSeverity(Severety severity);
    void log(Severety severety, const std::string& message);

    void addLogger(osg::ref_ptr<Logger> logger);

  private:
    LogManager();
    ~LogManager();

    struct Impl;
    std::unique_ptr<Impl> m;

    static Ptr m_instance;
  };
}