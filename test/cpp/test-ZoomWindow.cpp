#include <src/cpp/ZoomWindow.h>
#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <chrono>
#include <pongasoft/Utils/Lerp.h>

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
  ASSERT_EQ(-1, zoom.getZoomPointIndexFromOffset(-1));

  TSample s = -1.0;

  for(int i = 0; i < 15; i++)
  {
    // when no zoom, there is no accumulation
    ASSERT_TRUE(accumulator.accumulate(static_cast<TSample>(i), s));
    ASSERT_EQ(static_cast<TSample>(i), s);
  }

  zoom.getAccumulatorFromIndex(-73, offset);
  ASSERT_EQ(-73, offset);
  ASSERT_EQ(-73, zoom.getZoomPointIndexFromOffset(-73));
}

template <int batchSize, int batchSizeInSamples>
void testZoom(double zoomFactor, int const *iExpectedBatchSizes, int const *iExpectedOffSets)
{
  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0,1.0);

  Zoom<batchSize> zoom;

  zoom.setZoomFactor(zoomFactor);

  ASSERT_EQ(batchSize, zoom.getBatchSize());
  ASSERT_EQ(batchSizeInSamples, zoom.getBatchSizeInSamples());

  // we test that offset is properly computed
  int index = -1;
  int batchSizeIndex = batchSize - 1;
  int offset = 0;
  int previousOffet = 0;
  for(int k = 0; k < 5; k++)
  {
    for(int i = 0; i < batchSize; i++)
    {
      int offsetComputed = 0;
      auto accumulator = zoom.getAccumulatorFromIndex(index, offsetComputed);
      ASSERT_EQ(batchSizeIndex, accumulator.getBatchSizeIdx());
      ASSERT_EQ(offsetComputed, iExpectedOffSets[batchSizeIndex] - offset);
      ASSERT_EQ(index, zoom.getZoomPointIndexFromOffset(offsetComputed));

      // we make sure that all intermediaries also give the same result
      for(int m = offsetComputed; m < previousOffet; m++)
      {
        ASSERT_EQ(index, zoom.getZoomPointIndexFromOffset(m));
      }

      previousOffet = offsetComputed;

      index--;
      batchSizeIndex--;
      if(batchSizeIndex < 0)
        batchSizeIndex = batchSize - 1;
    }
    offset += zoom.getBatchSizeInSamples();
  }

  index = -1;
  for(int m = 0; m < 5; m++)
  {
    for(int i = 0; i < batchSize; i++)
    {
      int computedOffset = 0;

      // this tests that no matter where it starts, the accumulator behaves properly
      // i.e. that ZoomAccumulator::fBatchSizeIdx is properly set and increment/wraps around properly
      auto accumulator = zoom.getAccumulatorFromIndex(index--, computedOffset);

      // creating a random array of elements
      double elements[batchSizeInSamples];
      for(int k = 0; k < batchSizeInSamples; k++)
        elements[k] = distribution(generator);

      // batchIndex represents ZoomAccumulator::fBatchSizeIdx
      int batchIndex = accumulator.getBatchSizeIdx();

      // size represents ZoomAccumulator::fBatchSizes[ZoomAccumulator::fBatchSizeIdx]
      int size = iExpectedBatchSizes[batchIndex];
      double max = 0;

      // accumulating all the elements in the array
      for(int j = 0; j < batchSizeInSamples; j++)
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
          if(batchIndex == batchSize)
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

  int expectedBatchSizes[10] = {  2,   2,  2,    2,   2,   2,  2,  2,  2,  2};
  int expectedOffSets[10]    = {-20, -18, -16, -14, -12, -10, -8, -6, -4, -2};

  testZoom<10, 20>(2.0, expectedBatchSizes, expectedOffSets);
}


// ZoomTest - Zoom1Point3x (1.3x)
TEST(ZoomTest, Zoom1Point3x)
{
  int expectedBatchSizes[10] = {  1,   1,   1,   2,  1,  1,  2,  1,  1,  2};
  int expectedOffSets[10]    = {-13, -12, -11, -10, -8, -7, -6, -4, -3, -2};

  testZoom<10, 13>(1.3, expectedBatchSizes, expectedOffSets);
}

// ZoomTest - Zoom3Point4x (3.4x)
TEST(ZoomTest, Zoom3Point4x)
{
  int expectedBatchSizes[10] = {  3,   3,   4,   3,   4,   3,   3,   4,  3,  4};
  int expectedOffSets[10]    = {-34, -31, -28, -24, -21, -17, -14, -11, -7, -4};

  testZoom<10, 34>(3.4, expectedBatchSizes, expectedOffSets);
}

class ZoomWindowTest : public ::testing::Test
{
public:
  static constexpr auto VISIBLE_WINDOW_SIZE = 25;
  static constexpr auto BUFFER_SIZE = 104;
  static constexpr int MAX_ZOOM_FACTOR = BUFFER_SIZE / VISIBLE_WINDOW_SIZE;

  void SetUp() override
  {
    fWindow = new ZoomWindow(VISIBLE_WINDOW_SIZE, BUFFER_SIZE);
    fBuffer.init(0);
    fZoomBuffer.init(0);

    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(0.0,1.0);

    for(int i = 0; i < BUFFER_SIZE; i++)
    {
      auto sample = distribution(generator);
      fBuffer.push(sample);
      fReferenceBuffer[i] = sample;
    }
  }

  CircularBuffer<TSample> computeReferenceBuffer(double iZoomFactor)
  {
    ZoomWindow z(VISIBLE_WINDOW_SIZE, BUFFER_SIZE);
    z.__setRawZoomFactor(iZoomFactor);

    int idx = z.__getMinWindowIdx();

    TZoom zoom(iZoomFactor);

    int offset;
    auto accumulator = zoom.getAccumulatorFromIndex(idx, offset);

    std::vector<TSample> buffer{};

    while(offset < 0)
    {
      TSample value;
      if(accumulator.accumulate(fBuffer.getAt(offset), value))
        buffer.push_back(value);
      offset++;
    }

    CircularBuffer<TSample> res(static_cast<int>(buffer.size()));
    for(double i : buffer)
    {
      res.push(i);
    }
    return res;
  }


  void checkBuffer()
  {
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
      ASSERT_EQ(fReferenceBuffer[i], fBuffer.getAt(i)) << " at index " << i;
    }
  }

  void checkZoomBuffer(TSample iExpectedZoomBuffer[VISIBLE_WINDOW_SIZE])
  {
    for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
    {
      ASSERT_EQ(iExpectedZoomBuffer[i], fZoomBuffer.getAt(i)) << " at index " << i;
    }
  }

  void displayInfo()
  {
    std::cout << "maxZoomFactor=" << fWindow->__getMaxZoomFactor()
              << "; minWindowOffset=" << fWindow->__getMinWindowOffset()
              << "; minWindowIdx=" << fWindow->__getMinWindowIdx()
              << "; windowOffset=" << fWindow->__getWindowOffset()
              << std::endl;
  }


  void TearDown() override
  {
    delete fWindow;
  }

  ZoomWindow *fWindow{nullptr};
  CircularBuffer<TSample> fBuffer{BUFFER_SIZE};
  CircularBuffer<TSample> fZoomBuffer{VISIBLE_WINDOW_SIZE};
  TSample fReferenceBuffer[BUFFER_SIZE]{};
};

// ZoomWindowTest - SetWindowOffsetNoZoom)
TEST_F(ZoomWindowTest, SetWindowOffsetNoZoom)
{
  checkBuffer();

  // no zoom => we can access the entire buffer
  ASSERT_EQ(fWindow->__getMinWindowIdx(), -BUFFER_SIZE);
  ASSERT_EQ(fWindow->__getMinWindowOffset(), -BUFFER_SIZE + VISIBLE_WINDOW_SIZE - 1);
  ASSERT_EQ(fWindow->__getWindowOffset(), -1);

  TSample expectedZoomBuffer[VISIBLE_WINDOW_SIZE];
  fWindow->computeZoomWindow(fBuffer, fZoomBuffer);

  // calling computeZoomWindow should not affect buffer
  checkBuffer();

  // at this stage the current window represents the last VISIBLE_WINDOW_SIZE of the buffer
  fBuffer.copyToBuffer(-VISIBLE_WINDOW_SIZE, expectedZoomBuffer, VISIBLE_WINDOW_SIZE);
  checkZoomBuffer(expectedZoomBuffer);

  // buffer is a circular buffer and at this time, index 0 represents the oldest sample
  int bufferIndex = 0;
  // this loop will test all possible positions
  for(int windowOffset = fWindow->__getMinWindowOffset(); windowOffset <= -1; windowOffset++, bufferIndex++)
  {
    fWindow->__setRawWindowOffset(windowOffset);
    ASSERT_EQ(fWindow->__getWindowOffset(), windowOffset);

    // we compute the new zoom window
    fWindow->computeZoomWindow(fBuffer, fZoomBuffer);

    // we make sure it is the right one
    fBuffer.copyToBuffer(bufferIndex, expectedZoomBuffer, VISIBLE_WINDOW_SIZE);
    checkZoomBuffer(expectedZoomBuffer);
  }
}

// ZoomWindowTest - SetWindowOffset2xZoom) Live View test
TEST_F(ZoomWindowTest, SetWindowOffset2xZoom)
{
  TSample expectedZoomBuffer[VISIBLE_WINDOW_SIZE];

  fWindow->__setRawZoomFactor(2.0);

  CircularBuffer<TSample> referenceBuffer = computeReferenceBuffer(2.0);

  auto zoomedBufferSize = referenceBuffer.getSize();

  // sanity check
  ASSERT_EQ(zoomedBufferSize, BUFFER_SIZE / 2);

  // 2x zoom => we can access the entire buffer
  ASSERT_EQ(fWindow->__getMinWindowIdx(), -zoomedBufferSize);
  ASSERT_EQ(fWindow->__getMinWindowOffset(), -zoomedBufferSize + VISIBLE_WINDOW_SIZE - 1);

  // changing zoom in this mode should not affect window offset
  ASSERT_EQ(fWindow->__getWindowOffset(), -1);

  // we compute the new zoom window
  fWindow->computeZoomWindow(fBuffer, fZoomBuffer);
  referenceBuffer.copyToBuffer(-VISIBLE_WINDOW_SIZE, expectedZoomBuffer, VISIBLE_WINDOW_SIZE);
  checkZoomBuffer(expectedZoomBuffer);

  // buffer is a circular buffer and at this time, index 0 represents the oldest sample
  int bufferIndex = 0;
  // this loop will test all possible positions
  for(int windowOffset = fWindow->__getMinWindowOffset(); windowOffset <= -1; windowOffset++, bufferIndex++)
  {
    fWindow->__setRawWindowOffset(windowOffset);
    ASSERT_EQ(fWindow->__getWindowOffset(), windowOffset);

    // we compute the new zoom window
    fWindow->computeZoomWindow(fBuffer, fZoomBuffer);

    // we make sure it is the right one
    referenceBuffer.copyToBuffer(bufferIndex, expectedZoomBuffer, VISIBLE_WINDOW_SIZE);
    checkZoomBuffer(expectedZoomBuffer);
  }
}

/**
 * This test changes the zoom factor (assuming live mode where fWindow::fWindowOffset is -1) and then check
 * that we can change the offset and be able to read the proper zoomed data back throughout the entire possible
 * offsets
 */
void testSetWindowOffsetWithZoom(double iZoomFactor, ZoomWindowTest *iTest)
{
  ASSERT_EQ(-1, iTest->fWindow->__getWindowOffset());

  TSample expectedZoomBuffer[ZoomWindowTest::VISIBLE_WINDOW_SIZE];

  iTest->fWindow->__setRawZoomFactor(iZoomFactor);

  CircularBuffer<TSample> referenceBuffer = iTest->computeReferenceBuffer(iZoomFactor);

  auto zoomedBufferSize = referenceBuffer.getSize();

  ASSERT_EQ(iTest->fWindow->__getMinWindowIdx(), -zoomedBufferSize);
  ASSERT_EQ(iTest->fWindow->__getMinWindowOffset(), -zoomedBufferSize + ZoomWindowTest::VISIBLE_WINDOW_SIZE - 1);

  // changing zoom in this mode should not affect window offset
  ASSERT_EQ(iTest->fWindow->__getWindowOffset(), -1);

  // we compute the new zoom window
  iTest->fWindow->computeZoomWindow(iTest->fBuffer, iTest->fZoomBuffer);
  referenceBuffer.copyToBuffer(-ZoomWindowTest::VISIBLE_WINDOW_SIZE, expectedZoomBuffer, ZoomWindowTest::VISIBLE_WINDOW_SIZE);
  iTest->checkZoomBuffer(expectedZoomBuffer);

  // buffer is a circular buffer and at this time, index 0 represents the oldest sample
  int bufferIndex = 0;
  // this loop will test all possible positions
  for(int windowOffset = iTest->fWindow->__getMinWindowOffset(); windowOffset <= -1; windowOffset++, bufferIndex++)
  {
    iTest->fWindow->__setRawWindowOffset(windowOffset);
    ASSERT_EQ(iTest->fWindow->__getWindowOffset(), windowOffset);

    // we compute the new zoom window
    iTest->fWindow->computeZoomWindow(iTest->fBuffer, iTest->fZoomBuffer);

    // we make sure it is the right one
    referenceBuffer.copyToBuffer(bufferIndex, expectedZoomBuffer, ZoomWindowTest::VISIBLE_WINDOW_SIZE);
    iTest->checkZoomBuffer(expectedZoomBuffer);
  }
}

// ZoomWindowTest - SetWindowOffsetWithZoom) Live View test
TEST_F(ZoomWindowTest, SetWindowOffsetWithZoom)
{
  testSetWindowOffsetWithZoom(1.3, this);
  testSetWindowOffsetWithZoom(2.0, this);
  testSetWindowOffsetWithZoom(2.3, this);
  testSetWindowOffsetWithZoom(2.5, this);
  testSetWindowOffsetWithZoom(3.0, this);
  testSetWindowOffsetWithZoom(3.4, this);
  testSetWindowOffsetWithZoom(3.9, this);
  testSetWindowOffsetWithZoom(4.1, this);
  testSetWindowOffsetWithZoom(fWindow->__getMaxZoomFactor(), this);
}

// ZoomWindowTest - SetZoomFactor)
TEST_F(ZoomWindowTest, SetZoomFactor)
{
//  constexpr auto VISIBLE_WINDOW_SIZE = 25;
//  constexpr auto BUFFER_SIZE = 104;
//
//  constexpr int MAX_ZOOM_FACTOR = BUFFER_SIZE / VISIBLE_WINDOW_SIZE;
//
//  auto copyBuffer = [] (CircularBuffer<TSample> const &iBuffer, TSample oBuffer[VISIBLE_WINDOW_SIZE]) -> void {
//    for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//    {
//      oBuffer[VISIBLE_WINDOW_SIZE - 1 - i] = iBuffer.getAt(-1 - i);
//    }
//  };
//
//  ZoomWindow4Test window(VISIBLE_WINDOW_SIZE, BUFFER_SIZE);
//
//  // no zoom => we can access the entire buffer
//  ASSERT_EQ(window.__getMinWindowIdx(), -BUFFER_SIZE);
//  ASSERT_EQ(window.__getMinWindowOffset(), -BUFFER_SIZE + VISIBLE_WINDOW_SIZE - 1);
//
//  std::cout << "maxZoomFactor=" << window.__getMaxZoomFactor() << "; minWindowOffset=" << window.__getMinWindowOffset() << "; minWindowIdx=" << window.__getMinWindowIdx() << std::endl;
//
//  std::default_random_engine generator;
//  std::uniform_real_distribution<double> distribution(0.0,1.0);
//
//  TSample referenceBuffer[BUFFER_SIZE];
//  TSample resultBuffer[VISIBLE_WINDOW_SIZE];
//
//
//  CircularBuffer<TSample> buffer(BUFFER_SIZE);
//  for(int i = 0; i < BUFFER_SIZE; i++)
//  {
//    auto sample = distribution(generator);
//    sample = i;
//    buffer.push(sample);
//    referenceBuffer[i] = sample;
//  }
//
//  Zoom<10> zoom;
//  auto accumulator = zoom.setZoomFactor(2.0);
//  std::vector<TSample> referenceBuffer2x{};
//
//  TSample max;
//  for(int i = 0; i < BUFFER_SIZE; i++)
//  {
//    if(accumulator.accumulate(referenceBuffer[i], max))
//      referenceBuffer2x.push_back(max);
//  }
//
//  CircularBuffer<TSample> zoomBuffer(VISIBLE_WINDOW_SIZE);
//
//  // we are all the way to the right => should be the end of the buffer
//  window.computeZoomWindow(buffer, zoomBuffer);
//  copyBuffer(zoomBuffer, resultBuffer);
//  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//  {
//    ASSERT_EQ(referenceBuffer[BUFFER_SIZE - VISIBLE_WINDOW_SIZE + i], resultBuffer[i]);
//  }
//
//  // we are all the way to the left => should be the beginning of the buffer
//  window.setWindowOffset(0);
//
//  window.computeZoomWindow(buffer, zoomBuffer);
//  copyBuffer(zoomBuffer, resultBuffer);
//  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//  {
//    ASSERT_EQ(referenceBuffer[i], resultBuffer[i]);
//  }
//
//  // we are all the way to the right again => should be the end of the buffer
//  window.setWindowOffset(1.0);
//
//  window.computeZoomWindow(buffer, zoomBuffer);
//  copyBuffer(zoomBuffer, resultBuffer);
//  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//  {
//    ASSERT_EQ(referenceBuffer[BUFFER_SIZE - VISIBLE_WINDOW_SIZE + i], resultBuffer[i]);
//  }
//
//  double zoom2xPercent = Utils::Lerp<double>(MAX_ZOOM_FACTOR, 1.0).reverse(2.0);
//
//  window.setZoomFactor(zoom2xPercent);
//  window.computeZoomWindow(buffer, zoomBuffer);
//  copyBuffer(zoomBuffer, resultBuffer);
//
//  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//  {
//    ASSERT_EQ(referenceBuffer2x[referenceBuffer2x.size() - VISIBLE_WINDOW_SIZE + i], resultBuffer[i]);
//  }
//
//  // we are all the way to the left => should be the beginning of the buffer
//  window.setWindowOffset(0);
//  window.computeZoomWindow(buffer, zoomBuffer);
//  copyBuffer(zoomBuffer, resultBuffer);
//  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
//  {
//    ASSERT_EQ(referenceBuffer2x[i], resultBuffer[i]);
//  }
//
////  window.setZoomFactor(zoom2xPercent, VISIBLE_WINDOW_SIZE - 1);
////  window.computeZoomWindow(buffer, zoomBuffer);
////
////  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
////  {
////    std::cout << i << ":" << zoomBuffer.getAt(-1 - i) << std::endl;
////    //ASSERT_EQ(referenceBuffer[VISIBLE_WINDOW_SIZE - i - 1], zoomBuffer.getAt(-1 - i));
////  }
////
////  window.setZoomFactor(zoom2xPercent);
////  window.computeZoomWindow(buffer, zoomBuffer);
////
////  for(int i = 0; i < VISIBLE_WINDOW_SIZE; i++)
////  {
////    std::cout << i << ":" << zoomBuffer.getAt(-1 - i) << std::endl;
////    //ASSERT_EQ(referenceBuffer[VISIBLE_WINDOW_SIZE - i - 1], zoomBuffer.getAt(-1 - i));
////  }
//
//
}

}
}
}