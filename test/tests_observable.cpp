#include <gtest/gtest.h>

#include "detail.hpp"
#include "midas.hpp"

#include <fmt/core.h>

#include <TH2D.h>
#include <TH3D.h>

#include <memory>

TEST(TestObservable, Index)
{
    auto bins = 2;
    auto min = 0;
    auto max = 2;

    midas::axis_config axis;
    axis.set_bins(20, -3, 3);

    auto h2 = std::unique_ptr<TH2D>(new TH2D("h2", "h2", bins, min, max, bins, min, max));
    Int_t found_bin = h2->FindBin(0.5, 0.5);
    ASSERT_EQ(5, found_bin);

    Int_t bx, by, bz;
    h2->GetBinXYZ(found_bin, bx, by, bz);
    ASSERT_EQ(1, bx);
    ASSERT_EQ(1, by);
    ASSERT_EQ(0, bz);

    auto edm2 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM2, "edm2", h2.get(), axis, "edm2"));
    Int_t mapped_bin = edm2->getBin(bx - 1, by - 1, 0);
    ASSERT_EQ(0, mapped_bin);
}

TEST(TestObservable, Fill)
{
    auto bins = 2;
    auto min = 0;
    auto max = 2;

    midas::axis_config axis;
    axis.set_bins(20, -3, 3);

    // 	std::string pattern_string("%%d pattern");
    // 	std::string test_string("test pattern");
    // 	std::string replace_string("test");

    // 	std::string output_string = SmartFactory::placeholder(pattern_string, "%%d",
    // replace_string); 	ASSERT_EQ(output_string, test_string);

    auto h1 = std::unique_ptr<TH1D>(new TH1D("h1", "h1", bins, min, max));
    auto h2 = std::unique_ptr<TH2D>(new TH2D("h2", "h2", bins, min, max, bins, min, max));
    auto h3 = std::unique_ptr<TH3D>(new TH3D("h3", "h3", bins, min, max, bins, min, max, bins, min, max));

    auto edm1 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM1, "edm1", h1.get(), axis, "edm1"));
    auto edm2 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM2, "edm2", h2.get(), axis, "edm2"));
    auto edm3 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM3, "edm3", h3.get(), axis, "edm3"));

    for (int i = 0; i < bins; ++i)
    {
        ASSERT_TRUE(edm1->get(i));
        edm1->get(i)->FillRandom("gaus", 10000);
        ASSERT_TRUE(abs(edm1->get(i)->GetMean()) < 0.05);
        for (int j = 0; j < bins; ++j)
        {
            ASSERT_TRUE(edm2->get(i, j));
            edm2->get(i, j)->FillRandom("gaus", 10000);
            ASSERT_TRUE(abs(edm2->get(j)->GetMean()) < 0.05);
            for (int k = 0; k < bins; ++k)
            {
                ASSERT_TRUE(edm3->get(i, j, k));
                edm3->get(i, j, k)->FillRandom("gaus", 10000);
                ASSERT_TRUE(abs(edm3->get(k)->GetMean()) < 0.05);
            }
        }
    }
}

TEST(TestObservable, Reverse)
{
    auto bins = 2;
    auto min = 0;
    auto max = 2;

    midas::axis_config axis;
    axis.set_bins(20, -3, 3);

    auto h1 = std::unique_ptr<TH1D>(new TH1D("h1", "h1", bins, min, max));
    auto h2 = std::unique_ptr<TH2D>(new TH2D("h2", "h2", bins, min, max, bins, min, max));
    auto h3 = std::unique_ptr<TH3D>(new TH3D("h3", "h3", bins, min, max, bins, min, max, bins, min, max));

    auto edm1 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM1, "edm1", h1.get(), axis, "edm1"));
    auto edm2 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM2, "edm2", h2.get(), axis, "edm2"));
    auto edm3 = std::unique_ptr<midas::observable>(
        new midas::observable(midas::dimension::DIM3, "edm3", h3.get(), axis, "edm3"));

    Int_t x, y, z;

    ASSERT_FALSE(edm1->reverseBin(-1, x));
    ASSERT_TRUE(edm1->reverseBin(0, x));
    ASSERT_TRUE(edm1->reverseBin(1, x));
    ASSERT_FALSE(edm1->reverseBin(2, x));

    for (int i = 0; i < bins; ++i)
    {
        auto bin_x = edm1->getBin(i);
        ASSERT_TRUE(edm1->reverseBin(bin_x, x));
        ASSERT_EQ(i, x);

        for (int j = 0; j < bins; ++j)
        {
            auto bin_xy = edm2->getBin(i, j);
            ASSERT_TRUE(edm2->reverseBin(bin_xy, x, y));
            ASSERT_EQ(i, x);
            ASSERT_EQ(j, y);

            for (int k = 0; k < bins; ++k)
            {
                auto bin_xyz = edm3->getBin(i, j, k);
                ASSERT_TRUE(edm3->reverseBin(bin_xyz, x, y, z));
                ASSERT_EQ(i, x);
                ASSERT_EQ(j, y);
                ASSERT_EQ(k, z);
            }
        }
    }
}
