#include <gtest/gtest.h>
#include "integral_image.h"

using namespace std;
using namespace Chromaprint;

TEST(IntegralImage, Basic2D) {
	double data[] = {
		1.0, 2.0,
		3.0, 4.0,
	};
	Image image(2, data, data + 4);
	IntegralImage integral_image(&image);
	EXPECT_FLOAT_EQ(1.0, integral_image[0][0]);
	EXPECT_FLOAT_EQ(3.0, integral_image[0][1]);
	EXPECT_FLOAT_EQ(4.0, integral_image[1][0]);
	EXPECT_FLOAT_EQ(10.0, integral_image[1][1]);
}

TEST(IntegralImage, Vertical1D) {
	double data[] = {
		1.0, 2.0, 3.0
	};
	Image image(1, data, data + 3);
	IntegralImage integral_image(&image);
	EXPECT_FLOAT_EQ(1.0, integral_image[0][0]);
	EXPECT_FLOAT_EQ(3.0, integral_image[1][0]);
	EXPECT_FLOAT_EQ(6.0, integral_image[2][0]);
}

TEST(IntegralImage, Horizontal1D) {
	double data[] = {
		1.0, 2.0, 3.0
	};
	Image image(3, data, data + 3);
	IntegralImage integral_image(&image);
	EXPECT_FLOAT_EQ(1.0, integral_image[0][0]);
	EXPECT_FLOAT_EQ(3.0, integral_image[0][1]);
	EXPECT_FLOAT_EQ(6.0, integral_image[0][2]);
}

TEST(IntegralImage, Area) {
	double data[] = {
		1.0, 2.0, 3.0,
		4.0, 5.0, 6.0,
		7.0, 8.0, 9.0,
	};
	Image image(3, data, data + 9);
	IntegralImage integral_image(&image);

	EXPECT_FLOAT_EQ((1.0            ), integral_image.Area(0, 0, 0, 0));
	EXPECT_FLOAT_EQ((1.0 + 4.0      ), integral_image.Area(0, 0, 1, 0));
	EXPECT_FLOAT_EQ((1.0 + 4.0 + 7.0), integral_image.Area(0, 0, 2, 0));

	EXPECT_FLOAT_EQ((1.0            ) + (2.0            ), integral_image.Area(0, 0, 0, 1));
	EXPECT_FLOAT_EQ((1.0 + 4.0      ) + (2.0 + 5.0      ), integral_image.Area(0, 0, 1, 1));
	EXPECT_FLOAT_EQ((1.0 + 4.0 + 7.0) + (2.0 + 5.0 + 8.0), integral_image.Area(0, 0, 2, 1));

	EXPECT_FLOAT_EQ((2.0            ), integral_image.Area(0, 1, 0, 1));
	EXPECT_FLOAT_EQ((2.0 + 5.0      ), integral_image.Area(0, 1, 1, 1));
	EXPECT_FLOAT_EQ((2.0 + 5.0 + 8.0), integral_image.Area(0, 1, 2, 1));
}
