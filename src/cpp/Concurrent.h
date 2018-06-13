#pragma once

#include <atomic>

namespace pongasoft {
namespace Common {

/**
 * Thread safe and lock free queue containing at most one element. The intended usage is for 1 thread to call
 * "push" while another thread calls "pop". Calling "push" multiple times is safe and simply replaces the value in the
 * queue if there is already one. The type T must have a copy constructor. This class trades locks for memory allocation.
 * At any moment the queue may have 1 or 0 elements ("pop" returns false when no element)
 */
template<typename T>
class SingleElementQueue
{
public:
  SingleElementQueue() : fSingleElement{nullptr}
  {}

  ~SingleElementQueue()
  {
    delete fSingleElement.exchange(nullptr);
  }

  /**
   * Returns the single element in the queue if there is one
   *
   * @param oElement this will be populated with the value of the element in the queue or left untouched if there
   *                 isn't one
   * @return true if there was one element in the queue, false otherwise
   */
  inline bool pop(T &oElement)
  {
    auto element = fSingleElement.exchange(nullptr);
    if(element)
    {
      oElement = *element;
      delete element;
      return true;
    }

    return false;
  };

  /**
   * Pushes one element in the queue. If the queue already had an element it will be replaced.
   * @param iElement the element to push (clearly not modified by the call)
   */
  inline void push(T const &iElement)
  {
    delete fSingleElement.exchange(new T(iElement));
  }


private:
  std::atomic<T *> fSingleElement;
};

/**
 * Thread safe and lock free atomic value. The intended usage is for 1 thread to call "set" while another thread
 * calls "get". The type T must have a copy constructor. This class trades locks for memory allocation.
 *
 * Note that "set" is multi thread safe. "get" is only thread safe in relationship to "set" meaning it is fine to call
 * "get" from one thread while another thread is calling "set", but it is not fine to call "get" from multiple threads.
 */
template<typename T>
class AtomicValue
{
public:
  explicit AtomicValue(T const &iValue) : fValue{iValue}, fNewValue{nullptr}
  {}

  ~AtomicValue()
  {
    delete fNewValue.exchange(nullptr);
  }

  /**
   * Returns the "current" value. Note that this method should be called by one thread at a time (it is ok to call
   * "set" at the same time with another thread).
   *
   * @return the value. Since it is returning the address of the value, you should copy it if you wish to store it.
   */
  inline const T &get()
  {
    auto state = fNewValue.exchange(nullptr);
    if(state)
    {
      fValue = *state;
      delete state;
    }

    return fValue;
  };

  /**
   * Updates the current value with the provided one. This method is safe to be called by multiple threads.
   */
  inline void set(T const &iValue)
  {
    delete fNewValue.exchange(new T(iValue));
  }


private:
  T fValue;
  std::atomic<T *> fNewValue;
};

}
}