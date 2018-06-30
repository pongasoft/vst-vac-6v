#include <src/cpp/CircularBuffer.h>
#include <gtest/gtest.h>

namespace pongasoft {
namespace VST {
namespace Common {
namespace Test {

// CircularBuffer - copyToBuffer)
TEST(CircularBuffer, copyToBuffer)
{
  CircularBuffer<int> cb(5);
  cb.init(0);

  // underlying buffer will be [6,7,3,4,5]
  //                                ^ head
  for(int i = 1; i <= 7; i++)
    cb.push(i);

  // make sure the "head" is pointing at the right location
  ASSERT_EQ(cb.getAt(0), 3);

  int buf1[1];
  cb.copyToBuffer(0, buf1, 1);
  ASSERT_EQ(buf1[0], 3);

  int buf3[3];
  cb.copyToBuffer(0, buf3, 3);
  ASSERT_EQ(buf3[0], 3);
  ASSERT_EQ(buf3[1], 4);
  ASSERT_EQ(buf3[2], 5);

  cb.copyToBuffer(-1, buf3, 3);
  ASSERT_EQ(buf3[0], 7);
  ASSERT_EQ(buf3[1], 3);
  ASSERT_EQ(buf3[2], 4);

  cb.copyToBuffer(1, buf3, 3);
  ASSERT_EQ(buf3[0], 4);
  ASSERT_EQ(buf3[1], 5);
  ASSERT_EQ(buf3[2], 6);

  int buf7[7];
  cb.copyToBuffer(1, buf7, 7);
  ASSERT_EQ(buf7[0], 4);
  ASSERT_EQ(buf7[1], 5);
  ASSERT_EQ(buf7[2], 6);
  ASSERT_EQ(buf7[3], 7);
  ASSERT_EQ(buf7[4], 3);
  ASSERT_EQ(buf7[5], 4);
  ASSERT_EQ(buf7[6], 5);

}

}
}
}
}

