#include "osgHelper/LogManager.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>

namespace osgHelper
{
  LogManager::Ptr LogManager::m_instance;

  std::string currentTime(const char* format)
  {
    struct tm* now;
    time_t tim;
    time(&tim);
#pragma warning(suppress: 4996)
    now = localtime(&tim);

    char buffer[128];
    strftime(buffer, sizeof(buffer), format, now);

    return std::string(buffer);
  }

  void StdOutLogger::log(const std::string& text)
  {
    std::cout << text.c_str() << std::endl;
  }

  struct LogManager::Impl
  {
    Impl() : minSeverity(Severity::DEBUG) {}

    std::vector<osg::ref_ptr<Logger>> loggerList;
    Severity minSeverity;

    std::mutex mutex;
  };

  LogManager::Ptr LogManager::getInstance()
  {
    if (!m_instance.valid())
      m_instance = new LogManager();

    return m_instance;
  }

  void LogManager::clearInstance()
  {
    if (!m_instance)
    {
      return;
    }

    m_instance->m->loggerList.clear();
    m_instance.release();
  }

  void LogManager::setMinSeverity(Severity severity)
  {
    std::lock_guard<std::mutex> lock(m->mutex);

    m->minSeverity = severity;
  }

  void LogManager::log(Severity severety, const std::string& message)
  {
    std::lock_guard<std::mutex> lock(m->mutex);

    if (severety < m->minSeverity)
      return;

    std::string now = currentTime("%Y-%m-%d %H:%M:%S");
    std::string sev;

    if (severety == Severity::DEBUG)
      sev = "DEBUG";
    else if (severety == Severity::INFO)
      sev = "INFO";
    else if (severety == Severity::WARNING)
      sev = "WARN";
    else if (severety == Severity::FATAL)
      sev = "FATAL";
    else
      assert(false);

    std::string text = "[" + now + " " + sev + "] " + message;

    for (std::vector<osg::ref_ptr<Logger>>::iterator it = m->loggerList.begin(); it != m->loggerList.end(); ++it)
      (*it)->log(text);
  }

  void LogManager::addLogger(osg::ref_ptr<Logger> logger)
  {
    std::lock_guard<std::mutex> lock(m->mutex);
    m->loggerList.push_back(logger);
  }

  LogManager::LogManager()
    : osg::Referenced()
    , m(new Impl())
  {
  }

  LogManager::~LogManager()
  {
  }
}