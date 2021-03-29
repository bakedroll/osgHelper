#pragma once

#include <osg/Referenced>
#include <osg/observer_ptr>

#include <functional>
#include <vector>

namespace osgHelper
{
  /**
   * An Observer<T> is returned when Observable<T> registers a new callback function.
   * It holds the callback function, thus it gets automatically unregistered
   * when Observer<T> dies.
   */
  template<typename T>
  class Observer : public osg::Referenced
  {
  public:
    using Ptr  = osg::ref_ptr<Observer<T>>;
    using Func = std::function<void(T)>;

    Observer(const Func& func)
      : m_func(func)
    {}

    Func m_func;
  };

  template<>
  class Observer<void> : public osg::Referenced
  {
  public:
    using Ptr  = osg::ref_ptr<Observer>;
    using Func = std::function<void()>;

    Observer(const Func& func)
      : m_func(func)
    {}

    Func m_func;
  };

  namespace __ObservableInternals
  {
    /**
     * ObservableBase is the base class of Observable<T> and Signal.
     * It implements all necessities to register and notify callbacks.
     */
    template<typename T>
    class ObservableBase : public osg::Referenced
    {
    public:
      ObservableBase()
        : osg::Referenced()
      {}

      /**
       * Registers a callback function
       * @param func callback function
       */
      typename Observer<T>::Ptr connect(
        const typename Observer<T>::Func& func)
      {
        auto observer = new Observer<T>(func);
        observers().push_back(observer);

        return observer;
      }

      /**
       * Registers and notifies a callback function
       * @param func callback function
       */
      typename Observer<T>::Ptr connectAndNotify(
        const typename Observer<T>::Func& func)
      {
        auto observer = connect(func);
        notify(observer);

        return observer;
      }

    protected:
      using ObserverList = std::vector<osg::observer_ptr<Observer<T>>>;

      /**
       * The Observers are stored as weak pointers.
       * When there is no reference left, it gets removed from the list and will no longer be notified.
       */
      ObserverList& observers()
      {
        auto it = m_observers.begin();
        while (it != m_observers.end())
        {
          typename Observer<T>::Ptr obs;
          if (!it->lock(obs))
          {
            it = m_observers.erase(it);
          }
          else
          {
            ++it;
          }
        }

        return m_observers;
      }

      /**
       * Notify a specific observer. Must be implemented by inheriting classes.
       */
      virtual void notify(const typename Observer<T>::Ptr& obs) = 0;

      /**
       * Notifes all the observers.
       */
      void notifyAll()
      {
        const ObserverList& obs = observers();
        for (const auto& o : obs)
        {
          notify(o.get());
        }
      }

      ObserverList m_observers;
    };
  }

  /**
   * An Observable holds a value of type T which can be accessed via get() and set().
   * If it is set, all the registered observers are notified to execute their callback function
   * with the new value as argument. This is useful to track changes of certain values within the application,
   * eg. to update displays.
   */
  template<typename T>
  class Observable : public __ObservableInternals::ObservableBase<T>
  {
  public:
	  virtual ~Observable() = default;

    typedef osg::ref_ptr<Observable<T>> Ptr;

    Observable()
      : __ObservableInternals::ObservableBase<T>()
    {
    }

    Observable(T initial)
      : __ObservableInternals::ObservableBase<T>()
      , m_value(initial)
    {
    }

    void set(T value)
    {
      m_value = value;
      __ObservableInternals::ObservableBase<T>::notifyAll();
    }

    T get() const
    {
      return m_value;
    }

  protected:
    void notify(const typename Observer<T>::Ptr& obs) override
    {
      obs->m_func(m_value);
    }

  private:
    T m_value;

  };

  /**
   * A Signal is similar to an Observable<T>, but without a value. It can be used to trigger an event
   * and notify all the observers that have registered a callback function.
   */
  class Signal : public __ObservableInternals::ObservableBase<void>
  {
  public:
    Signal()
      : __ObservableInternals::ObservableBase<void>()
    {
    }

    void trigger()
    {
      notifyAll();
    }

  protected:
    void notify(const Observer<void>::Ptr& obs) override
    {
      obs->m_func();
    }

  };

  /**
   * A special Observable<T> that takes the initial value as template argument.
   */
  template <typename T, T init = T()>
  class InitializedObservable : public Observable<T>
  {
  public:
    InitializedObservable()
      : Observable<T>(init)
    {

    }
  };

}
