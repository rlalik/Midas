#include <gtest/gtest.h>

#include <midas.hpp>

TEST(TestsContext, CreationByDim)
{
    auto ctx1 = midas::context("test", midas::dimension::DIM1);
    ctx1.get_x();
    ASSERT_THROW(ctx1.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx1.get_z(), midas::dimension_error);

    auto ctx2 = midas::context("test", midas::dimension::DIM2);
    ctx2.get_x();
    ctx2.get_y();
    ASSERT_THROW(ctx2.get_z(), midas::dimension_error);

    auto ctx3 = midas::context("test", midas::dimension::DIM3);
    ctx3.get_x();
    ctx3.get_y();
    ctx3.get_z();
}

TEST(TestsContext, CreationByDimWithV)
{
    midas::axis_config v;

    auto ctx1 = midas::context("test", midas::dimension::DIM1, v);
    ctx1.get_x();
    ASSERT_THROW(ctx1.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx1.get_z(), midas::dimension_error);

    auto ctx2 = midas::context("test", midas::dimension::DIM2, v);
    ctx2.get_x();
    ctx2.get_y();
    ASSERT_THROW(ctx2.get_z(), midas::dimension_error);

    auto ctx3 = midas::context("test", midas::dimension::DIM3, v);
    ctx3.get_x();
    ctx3.get_y();
    ctx3.get_z();
}

TEST(TestsContext, CreationByAxes)
{
    midas::axis_config x;
    midas::axis_config y;
    midas::axis_config z;
    midas::axis_config v;

    auto ctx1 = midas::context("test", x, v);
    ctx1.get_x();
    ASSERT_THROW(ctx1.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx1.get_z(), midas::dimension_error);
    ctx1.get_v();

    auto ctx2 = midas::context("test", x, y, v);
    ctx2.get_x();
    ctx2.get_y();
    ASSERT_THROW(ctx2.get_z(), midas::dimension_error);
    ctx1.get_v();

    auto ctx3 = midas::context("test", x, y, z, v);
    ctx3.get_x();
    ctx3.get_y();
    ctx3.get_z();
    ctx1.get_v();
}

TEST(TestsContext, Updating)
{
    auto ctx = midas::context("test", midas::dimension::DIM3);
    ctx.get_x().set_label("d2_x").set_unit("mm");
    ctx.get_y().set_label("d2_y").set_unit("");
    ctx.get_z().set_label("d2_z").set_unit("cm");

    ASSERT_EQ(TString(" [mm]"), ctx.get_x().format_unit());
    ASSERT_EQ(TString(""), ctx.get_y().format_unit());
    ASSERT_EQ(TString(" [cm]"), ctx.get_z().format_unit());

    midas::context ctx_2 = ctx;

    ASSERT_EQ(TString(" [cm]"), ctx_2.get_z().format_unit());

    ASSERT_TRUE(ctx == ctx_2);

    ctx_2.get_x().set_bins(13, 0, 0);
    ASSERT_FALSE(ctx == ctx_2);

    ASSERT_TRUE(ctx == ctx);
}

TEST(TestsContext, Extend)
{
    auto ctx = midas::context("test", midas::dimension::NODIM);
    ASSERT_THROW(ctx.get_x(), midas::dimension_error);
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.extend();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_NO_THROW(ctx.get_z());

    ASSERT_THROW(ctx.extend(), midas::dimension_error);
}

TEST(TestsContext, Reduce)
{
    auto ctx = midas::context("test", midas::dimension::DIM3);
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_NO_THROW(ctx.get_z());

    ctx.reduce();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_NO_THROW(ctx.get_y());
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ctx.reduce();
    ASSERT_NO_THROW(ctx.get_x());
    ASSERT_THROW(ctx.get_y(), midas::dimension_error);
    ASSERT_THROW(ctx.get_z(), midas::dimension_error);

    ASSERT_THROW(ctx.reduce(), midas::dimension_error);

    auto ctx2 = midas::context("test", midas::dimension::NODIM);
    ASSERT_THROW(ctx2.reduce(), midas::dimension_error);
}

TEST(TestsContext, ReduceAndExtend)
{
    midas::axis_config x;
    x.set_bins(10, -10, 10);

    midas::axis_config y;
    y.set_bins(20, -20, 20);

    midas::axis_config z;
    z.set_bins(30, -30, 30);

    midas::axis_config v;
    v.set_bins(100, -100, 100);

    auto ctx = midas::context("test", x, y, z, v);
    ctx.reduce(); // to DIM2
    ctx.reduce(); // to DIM1

    ctx.extend(); // to DIM2
    // After extension, the new dimensions should be cleared
    ASSERT_EQ(ctx.get_y().get_bins(), 0);
    ASSERT_EQ(ctx.get_y().get_min(), 0.0f);
    ASSERT_EQ(ctx.get_y().get_max(), 0.0f);

    ctx.extend(); // to DIM3
    // After extension, the new dimensions should be cleared
    ASSERT_EQ(ctx.get_z().get_bins(), 0);
    ASSERT_EQ(ctx.get_z().get_min(), 0.0f);
    ASSERT_EQ(ctx.get_z().get_max(), 0.0f);
}

TEST(TestsContext, Compare)
{
    midas::axis_config x;
    x.set_bins(10, -10, 10);

    midas::axis_config y;
    y.set_bins(20, -20, 20);

    midas::axis_config z;
    z.set_bins(30, -30, 30);

    midas::axis_config v;
    v.set_bins(100, -100, 100);

    auto ctx1 = midas::context("test", x, v);
    auto ctx1_1 = midas::context("test", x, v);
    auto ctx1_2 = midas::context("test", y, v);

    auto ctx2 = midas::context("test", x, y, v);
    auto ctx2_1 = midas::context("test", x, y, v);
    auto ctx2_2 = midas::context("test", y, z, v);

    auto ctx3 = midas::context("test", x, y, z, v);
    auto ctx3_1 = midas::context("test", x, y, z, v);
    auto ctx3_2 = midas::context("test", y, z, x, v);

    ASSERT_TRUE(ctx1 == ctx1);
    ASSERT_TRUE(ctx1 == ctx1_1);
    ASSERT_FALSE(ctx1 != ctx1_1);
    ASSERT_FALSE(ctx1 == ctx1_2);
    ASSERT_TRUE(ctx1 != ctx1_2);
    ASSERT_FALSE(ctx1 == ctx2);
    ASSERT_FALSE(ctx1 == ctx3);

    ASSERT_TRUE(ctx2 == ctx2);
    ASSERT_TRUE(ctx2 == ctx2_1);
    ASSERT_FALSE(ctx2 != ctx2_1);
    ASSERT_FALSE(ctx2 == ctx2_2);
    ASSERT_TRUE(ctx2 != ctx2_2);
    ASSERT_FALSE(ctx2 == ctx3);
    ASSERT_FALSE(ctx2 == ctx1);

    ASSERT_TRUE(ctx3 == ctx3);
    ASSERT_TRUE(ctx3 == ctx3_1);
    ASSERT_FALSE(ctx3 != ctx3_1);
    ASSERT_FALSE(ctx3 == ctx3_2);
    ASSERT_TRUE(ctx3 != ctx3_2);
    ASSERT_FALSE(ctx3 == ctx1);
    ASSERT_FALSE(ctx3 == ctx2);

    auto ctx0 = midas::context("test", midas::dimension::NODIM);
    ASSERT_FALSE(ctx0 == ctx1);
    ASSERT_TRUE(ctx0 != ctx1);
    ASSERT_FALSE(ctx0 == ctx2);
    ASSERT_TRUE(ctx0 != ctx2);
    ASSERT_FALSE(ctx0 == ctx3);
    ASSERT_TRUE(ctx0 != ctx3);
}

TEST(TestsContext, Basic)
{
    Float_t dummy_var;

    auto vctx = midas::context("test", midas::dimension::DIM2);
    vctx.get_x().set_bins(10, 0, 10).set_label("a2_x").set_unit("mm").set_variable(&dummy_var);
    vctx.get_y().set_bins(10, 0, 10).set_label("a2_y").set_variable(&dummy_var);
    vctx.get_v().set_bins(10, 0, 10).set_label("a2_V").set_variable(&dummy_var);
    vctx.prepare();

    ASSERT_EQ(TString(" [mm]"), vctx.get_x().format_unit());
    ASSERT_EQ(TString("d^{2}/da2_xda2_y"), vctx.get_label());

    vctx = midas::context("test", midas::dimension::DIM3);

    vctx.get_x().set_bins(10, 0, 10).set_label("a2_x").set_unit("mm").set_variable(&dummy_var);
    vctx.get_y().set_bins(10, 0, 10).set_label("a2_y").set_variable(&dummy_var);
    vctx.get_z().set_bins(10, 0, 10).set_label("a2_z").set_variable(&dummy_var);
    vctx.get_v().set_bins(10, 0, 10).set_label("a2_V").set_variable(&dummy_var);
    vctx.prepare();

    ASSERT_EQ(TString("d^{3}/da2_xda2_yda2_z"), vctx.get_label());

    midas::context vctx_2 = vctx;
    vctx_2.prepare();

    ASSERT_EQ(TString("d^{3}/da2_xda2_yda2_z"), vctx_2.get_label());

    ASSERT_TRUE(vctx == vctx_2);

    vctx_2.get_x().set_bins(13, 0, 10);
    ASSERT_FALSE(vctx == vctx_2);

    ASSERT_TRUE(vctx == vctx);
}
