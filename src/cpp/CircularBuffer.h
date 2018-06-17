#pragma once

#include <cassert>

namespace pongasoft {
namespace VST {
namespace Common {

template<typename T>
class CircularBuffer
{
public:
  explicit CircularBuffer(int iSize) : fSize(iSize), fStart(0)
  {
    assert(fSize > 0);

    fBuf = new T[iSize];
  };

  ~CircularBuffer()
  {
    delete[] fBuf;
  }

  // handle negative offsets as well
  inline int getSize() const
  {
    return fSize;
  }

  inline T getAt(int offset) const
  {
    return fBuf[adjustIndexFromOffset(offset)];
  }

  inline void setAt(int offset, T e)
  {
    fBuf[adjustIndexFromOffset(offset)] = e;
  };

  inline void incrementHead()
  {
    fStart = adjustIndex(fStart + 1);
  }

  inline void push(T e)
  {
    setAt(0, e);
    incrementHead();
  }

  inline void init(T initValue)
  {
    for(int i = 0; i < fSize; ++i)
    {
      fBuf[i] = initValue;
    }
  }

  template<typename U, class BinaryPredicate>
  inline U fold(int startOffset, int endOffset, U initValue, BinaryPredicate &op) const
  {
    if(startOffset == endOffset)
      return initValue;

    int adjStartOffset = adjustIndexFromOffset(startOffset);
    int adjEndOffset = adjustIndexFromOffset(endOffset);

    U resultValue = initValue;

    int i = adjStartOffset;
    while(i != adjEndOffset)
    {
      resultValue = op(resultValue, fBuf[i]);
      if(startOffset < endOffset)
      {
        ++i;
        if(i == fSize)
          i = 0;
      } else
      {
        --i;
        if(i == -1)
          i = fSize - 1;
      }
    }

    return resultValue;
  }

  template<typename U, class BinaryPredicate>
  inline U fold(int endOffset, U initValue, BinaryPredicate &op) const
  {
    return fold(0, endOffset, initValue, op);
  }

private:
  inline int adjustIndexFromOffset(int offset) const
  {
    if(offset == 0)
      return fStart;

    return adjustIndex(fStart + offset);
  }

  inline int adjustIndex(int index) const
  {
    // shortcut since this is a frequent use case
    if(index == fSize)
      return 0;

    while(index < 0)
      index += fSize;

    while(index >= fSize)
      index -= fSize;

    return index;
  }

  int fSize;
  T *fBuf;
  int fStart;
};

}
}
}