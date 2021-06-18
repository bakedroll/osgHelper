#include <gtest/gtest.h>

#include <osgHelper/Observable.h>

TEST(ObservableTest, Observers)
{
  osgHelper::Observable<int> observedNumber(5);

  bool obs1Notified = false;
  osgHelper::Observer<int>::Ptr obs1 = observedNumber.connect([&obs1Notified](int value)
  {
    obs1Notified = true;
  });

  bool obs2Notified = false;
  osgHelper::Observer<int>::Ptr obs2 = observedNumber.connect([&obs2Notified](int value)
  {
    obs2Notified = true;
  });

  bool obs3Notified = false;
  osgHelper::Observer<int>::Ptr obs3 = observedNumber.connectAndNotify([&obs3Notified](int value)
  {
    obs3Notified = true;
  });

  EXPECT_EQ(observedNumber.get(), 5) << "observedNumber should be 5 here";
  EXPECT_FALSE(obs1Notified) << "obs1Notified should not have been notified here";
  EXPECT_FALSE(obs2Notified) << "obs2Notified should not have been notified here";
  EXPECT_TRUE(obs3Notified) << "obs3Notified should have been notified here";

  obs3Notified = false;

  observedNumber.set(3);

  EXPECT_EQ(observedNumber.get(), 3) << "observedNumber should be 3 here";
  EXPECT_TRUE(obs1Notified) << "obs1Notified should have been notified here";
  EXPECT_TRUE(obs2Notified) << "obs2Notified should have been notified here";
  EXPECT_TRUE(obs3Notified) << "obs3Notified should have been notified here";

  obs1Notified = false;
  obs2Notified = false;
  obs3Notified = false;

  obs2.release();

  observedNumber.set(10);

  EXPECT_EQ(observedNumber.get(), 10) << "observedNumber should be 10 here";
  EXPECT_TRUE(obs1Notified) << "obs1Notified should have been notified here";
  EXPECT_FALSE(obs2Notified) << "obs2Notified should not have been notified here";
  EXPECT_TRUE(obs3Notified) << "obs3Notified should have been notified here";
}
