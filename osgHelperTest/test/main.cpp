#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

#ifdef _DEBUG
  getchar();
#endif

  return result;
}