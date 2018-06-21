#include <src/cpp/ZoomWindow.h>
#include <gtest/gtest.h>
#include <random>

namespace pongasoft {
namespace VST {
namespace Test {

using namespace pongasoft::VST::Common;

///////////////////////////////////////////
// Zoom tests
///////////////////////////////////////////

// ZoomTest - NoZoom
TEST(ZoomTest, NoZoom)
{
  Zoom<10> zoom;

  ASSERT_TRUE(zoom.isNoZoom());

  ASSERT_EQ(10, zoom.getBatchSize());
  ASSERT_EQ(10, zoom.getBatchSizeInSamples());

  int offset = 0;

  auto accumulator = zoom.getAccumulatorFromIndex(-1, offset);

  ASSERT_EQ(-1, offset);

  TSample s = -1.0;

  for(int i = 0; i < 15; i++)
  {
    // when no zoom, there is no accumulation
    ASSERT_TRUE(accumulator.accumulate(static_cast<TSample>(i), s));
    ASSERT_EQ(static_cast<TSample>(i), s);
  }

  zoom.getAccumulatorFromIndex(-73, offset);
  ASSERT_EQ(-73, offset);
}

template <int batchSize = 10>
void testZoom(double zoomFactor, int iBatchSize, int iBatchSizeInSamples, int const *iExpectedBatchSizes, int const *iExpectedOffSets)
{
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0,1.0);

  Zoom<batchSize> zoom;

  zoom.setZoomFactor(zoomFactor);

  ASSERT_EQ(iBatchSize, zoom.getBatchSize());
  ASSERT_EQ(iBatchSizeInSamples, zoom.getBatchSizeInSamples());

  // we test that offset is properly computed
  int index = -1;
  for(int k = 0; k < 5; k++)
  {
    for(int i = 0; i < iBatchSize; i++)
    {
      int offset = 0;
      zoom.getAccumulatorFromIndex(index--, offset);
      ASSERT_EQ(offset, iExpectedOffSets[i] - (iBatchSizeInSamples * k));
    }
  }

  index = -1;
  for(int m = 0; m < 5; m++)
  {
    for(int i = 0; i < iBatchSize; i++)
    {
      int offset = 0;

      // this tests that no matter where it starts, the accumulator behaves properly
      // i.e. that ZoomAccumulator::fBatchSizeIdx is properly set and increment/wraps around properly
      auto accumulator = zoom.getAccumulatorFromIndex(index--, offset);
      ASSERT_EQ(offset, iExpectedOffSets[i] - (iBatchSizeInSamples * m));

      // creating a random array of elements
      double elements[iBatchSizeInSamples];
      for(int k = 0; k < iBatchSizeInSamples; k++)
        elements[k] = distribution(generator);

      // batchIndex represents ZoomAccumulator::fBatchSizeIdx
      int batchIndex = i;

      // size represents ZoomAccumulator::fBatchSizes[ZoomAccumulator::fBatchSizeIdx]
      int size = iExpectedBatchSizes[batchIndex];
      double max = 0;

      // accumulating all the elements in the array
      for(int j = 0; j < iBatchSizeInSamples; j++)
      {
        TSample s = -1.0;
        bool complete = accumulator.accumulate(elements[j], s);
        max = std::max(max, elements[j]);
        size--;

        if(size == 0)
        {
          ASSERT_TRUE(complete);
          ASSERT_EQ(s, max);
          batchIndex++;
          if(batchIndex == iBatchSize)
            batchIndex = 0;
          size = iExpectedBatchSizes[batchIndex];
          max = 0;
        }
        else
        {
          ASSERT_FALSE(complete);
          ASSERT_EQ(s, -1.0);
        }
      }
    }
  }

}

// ZoomTest - Zoom2x
TEST(ZoomTest, Zoom2x)
{
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0,1.0);

  Zoom<10> zoom;

  zoom.setZoomFactor(2.0);

  ASSERT_EQ(10, zoom.getBatchSize());
  ASSERT_EQ(20, zoom.getBatchSizeInSamples());

  int offset = 0;

  auto accumulator = zoom.getAccumulatorFromIndex(-1, offset);

  ASSERT_EQ(-2, offset);

  for(int i = 0; i < 25; i++)
  {
    TSample s = -1.0;
    double s1 = distribution(generator);
    ASSERT_FALSE(accumulator.accumulate(s1, s));
    ASSERT_EQ(-1.0, s); // unchanged

    double s2 = distribution(generator);
    ASSERT_TRUE(accumulator.accumulate(s2, s));

    ASSERT_EQ(std::max(s1,s2), s);
  }

  zoom.getAccumulatorFromIndex(-73, offset);
  ASSERT_EQ(-146, offset);

  int expectedBatchSizes[10] = { 2,  2,  2,  2,   2,   2,   2,   2,   2,   2};
  int expectedOffSets[10]    = {-2, -4, -6, -8, -10, -12, -14, -16, -18, -20};

  testZoom<10>(2.0, 10, 20, expectedBatchSizes, expectedOffSets);
}


// ZoomTest - Zoom1Point3x (1.3x)
TEST(ZoomTest, Zoom1Point3x)
{
  int expectedBatchSizes[10] = { 1,  1,  1,  2,  1,  1,  2,   1,    1,   2};
  int expectedOffSets[10]    = {-1, -2, -3, -5, -6, -7, -9, -10,  -11, -13};

  testZoom<10>(1.3, 10, 13, expectedBatchSizes, expectedOffSets);
}

// ZoomTest - Zoom3Point4x (3.4x)
TEST(ZoomTest, Zoom3Point4x)
{
  int expectedBatchSizes[10] = { 3,  3,   4,   3,   4,   3,   3,   4,   3,   4};
  int expectedOffSets[10]    = {-3, -6, -10, -13, -17, -20, -23, -27, -30, -34};

  testZoom<10>(3.4, 10, 34, expectedBatchSizes, expectedOffSets);
}


}
}
}