/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "midas.hpp"

#include "detail.hpp"

#include <hellofitty.hpp>

#include <fmt/core.h>

#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TLatex.h>
#include <TList.h>
#include <TSystem.h>

namespace midas
{

const Option_t h1opts[] = "h,E1";

const TString flags_fit_a = "B,Q,0";
const TString flags_fit_b = "";

// distribution::distribution() : basic_distribution(), ctx(context()), cells(nullptr) {}

distribution::distribution(const context& context, pandora::pandora* pbox)
    : basic_distribution(context, pbox), ctx(context), objects_fits(nullptr)
{
}

distribution::~distribution() { gSystem->ProcessEvents(); }

auto distribution::operator==(const distribution& fac) -> bool
{
    if (ctx == fac.ctx) return true;
    return false;
}

/*
DifferentialFactory& DifferentialFactory::operator=(const DifferentialFactory& fa)
{
    if (this == &fa) return *this;

    // 	nthis->objectsFits = new TObjArray();
    // 	nthis->objectsFits->SetName(ctx.name + "Fits");

    static_cast<distribution>(*this) = static_cast<distribution>(fa);
    if (!cells) return *this;

    for (int i = 0; i < cells->nhists; ++i)
        detail::copyHistogram((*fa.cells)[i], (*cells)[i]);

    return *this;
}*/

auto distribution::prepare() -> void
{
    basic_distribution::prepare();
    objects_fits = new TObjArray();
    objects_fits->SetName(ctx.name + "_fits");

    cells = std::unique_ptr<observable>(
        new observable(ctx.dim, ctx.name.Data(), get_signal_hist(), ctx.v, "{dir}/cells/%c_{analysis}", box));
}

auto distribution::reinit() -> void
{
    // basic_distribution::ctx = static_cast<basic_context>(ctx);
    basic_distribution::reinit();

    // if (cells)
    // {
    //     cells->prefix_name = ctx.hist_name;
    //     cells->rename(ctx.hist_name);
    //     cells->chdir(ctx.dir_name);
    // }
}

// auto DifferentialFactory::getDiffs(bool with_canvases)-> void
// {
// 	Int_t can_width = 800, can_height = 600;
// TString hname, htitle, cname;
//
// 	if (ctx.useDiff())
// 	{
// 		objectsDiffs = new TObjArray();
// 		objectsDiffs->SetName(ctx.histPrefix + "Diffs");
//
// 		// Lambda: differential plots
// 		hDiscreteXYDiff = new TH1D**[ctx.cx.bins];
// 		if (with_canvases)
// 			c_Diffs = new TCanvas*[ctx.cx.bins];
//
// 		for (uint i = 0; i < ctx.cx.bins; ++i)
// 		{
// 			hDiscreteXYDiff[i] = new TH1D*[ctx.cy.bins];
//
// 			for (uint j = 0; j < ctx.cy.bins; ++j)
// 			{
// 				hname = TString::Format("@@@d/Diffs/h_@@@a_LambdaDiff_%s%02d_%s%02d", "X", i, "Y",
// j); 				htitle = TString::Format(
// 					"#Lambda: %s[%d]=%.1f-%.1f, %s[%d]=%.0f-%.0f;M [MeV/c^{2}];Stat",
// 					ctx.cx.label.Data(), i,
// 					ctx.cx.min+ctx.cx.delta*i,
// 					ctx.cx.min+ctx.cx.delta*(i+1),
// 					ctx.cy.label.Data(), j,
// 					ctx.cy.min+ctx.cy.delta*j,
// 					ctx.cy.min+ctx.cy.delta*(j+1));
//
// 					hDiscreteXYDiff[i][j] = RegTH1<TH1D>(hname, htitle, ctx.v.bins, ctx.v.min,
// ctx.v.max);
//
// 					objectsDiffs->AddLast(hDiscreteXYDiff[i][j]);
// 			}
//
// 			if (with_canvases)
// 			{
// 				cname = TString::Format("@@@d/Diffs/c_@@@a_LambdaDiff_%s%02d", "X", i);
// 				c_Diffs[i] = RegCanvas(cname, "test", can_width, can_height, ctx.cy.bins);
// 			}
// 		}
// 	}
// }

auto distribution::fill(Float_t weight) -> void
{
    // basic_distribution::fill(weight);
    if (dimension::DIM3 == ctx.dim)
        cells->fill_3d(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.z.get_var(), *ctx.v.get_var(), weight);
    else if (dimension::DIM2 == ctx.dim)
        cells->fill_2d(*ctx.x.get_var(), *ctx.y.get_var(), *ctx.v.get_var(), weight);
    else if (dimension::DIM1 == ctx.dim)
        cells->fill_1d(*ctx.x.get_var(), *ctx.v.get_var(), weight);
}

auto distribution::finalize(const char* draw_opts) -> void
{
    basic_distribution::finalize(draw_opts);
    prepare_cells_canvas();
}

auto distribution::transform_v(std::function<void(TH1* h)> transform_function) -> void
{
    if (cells)
    {
        for (int i = 0; i < cells->get_hists_number(); ++i)
        {
            transform_function(dynamic_cast<TH1*>(cells->get_hist_by_index(i)));
        }
    }
}

auto distribution::transform_v(std::function<void(TCanvas* h)> transform_function) -> void
{
    if (cells)
    {
        fmt::print("N of canvases = {}\n", cells->get_canvas_number());
        for (int i = 0; i < cells->get_canvas_number(); ++i)
        {
            transform_function(dynamic_cast<TCanvas*>(cells->get_canvas_by_index(i)));
        }
    }
}

auto distribution::save(const char* filename, save_mode mode, bool verbose) -> bool
{
    auto f = TFile::Open(filename, mode == save_mode::update ? "UPDATE" : "RECREATE");
    auto res = save(f, verbose);
    f->Close();
    return res;
}

auto distribution::save(TFile* f, bool verbose) -> bool
{
    if (box)
    {
        box->reg_object(&ctx, ctx.GetName());
        if (verbose) box->list_registered_objects();
        return box->export_structure(f, verbose);
    }
    return false;
}

auto distribution::prepare_cells_canvas() -> void
{
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.07f);

    TLatex nflatex;
    nflatex.SetNDC();
    nflatex.SetTextSize(0.07f);
    nflatex.SetTextAlign(23);

    auto nhists = cells->get_hists_number();
    for (int i = 0; i < nhists; ++i)
    {
        Int_t bx = 0, by = 0, bz = 0;
        cells->reverse_bin(i, bx, by, bz);

        TVirtualPad* pad = cells->get_pad(bx, by, bz);
        pad->Clear();
        pad->cd();
        TH1* h = cells->get_hist(bx, by, bz);
        h->Draw();
        int pad_number = 0;
        if (ctx.dim == dimension::DIM1)
            pad_number = bx;
        else if (ctx.dim == dimension::DIM2)
            pad_number = by;
        else if (ctx.dim == dimension::DIM3)
            pad_number = bz;

        latex.DrawLatex(0.12, 0.85, TString::Format("%02d", pad_number));

        auto flist = h->GetListOfFunctions();
        auto fs = flist->GetEntries();
        if (fs < 3)
        {
            latex.DrawLatex(0.55, 0.85, TString::Format("E=%g", h->GetEntries()));
            latex.DrawLatex(0.55, 0.80, TString::Format("R=%g", h->GetRMS()));
            latex.DrawLatex(0.55, 0.75, TString::Format("E/R=%g", h->GetEntries() / h->GetRMS()));

            nflatex.DrawLatex(0.5, 0.5, "No fit");
            continue;
        }

        TF1* tfSum = dynamic_cast<TF1*>(flist->At(0));
        TF1* tfSig = dynamic_cast<TF1*>(flist->At(1));
        TF1* tfBkg = dynamic_cast<TF1*>(flist->At(2));

        tfSig->SetLineColor(kBlack);
        tfSig->SetLineWidth(1);
        tfSig->SetFillColor(kGray);
        tfSig->SetFillStyle(3000);

        tfBkg->SetLineColor(kGray + 2);
        tfBkg->SetLineWidth(1);
        // 	tfBkg->SetLineStyle(7);

        tfSum->SetLineColor(kRed);
        tfSum->SetLineWidth(1);

        tfBkg->Draw("same");
        tfSum->Draw("same");

        auto hsigclone = dynamic_cast<TH1*>(h->Clone("hsig"));
        hsigclone->Add(tfBkg, -1);
        hsigclone->Delete();

        h->SetTitle("");
        TLatex loop_latex;
        loop_latex.SetNDC();
        loop_latex.SetTextColor(/*36*/ 1);
        loop_latex.SetTextSize(0.06f);
        auto oldalign = loop_latex.GetTextAlign();
        Short_t centeralign = 23;

        double centerpos = (1.0 - static_cast<double>(pad->GetRightMargin() + pad->GetLeftMargin())) / 2.0;

        loop_latex.SetTextAlign(centeralign);
        if (ctx.dim >= dimension::DIM1)
        {
            auto X_l = ctx.x.get_min() + ctx.x.get_delta() * static_cast<float>(bx);
            auto X_h = ctx.x.get_min() + ctx.x.get_delta() * static_cast<float>(1 + bx);
            loop_latex.DrawLatex(centerpos, 1.01,
                                 fmt::format("{:.2f} < {:s} < {:.2f}", X_l, ctx.x.get_label().Data(), X_h).c_str());
        }
        if (ctx.dim >= dimension::DIM2)
        {
            auto Y_l = ctx.y.get_min() + ctx.y.get_delta() * static_cast<float>(by);
            auto Y_h = ctx.y.get_min() + ctx.y.get_delta() * static_cast<float>(1 + by);
            loop_latex.DrawLatex(centerpos, 0.96,
                                 fmt::format("{:.2f} < {:s} < {:.2f}", Y_l, ctx.y.get_label().Data(), Y_h).c_str());
        }
        if (ctx.dim >= dimension::DIM3)
        {
            auto Z_l = ctx.y.get_min() + ctx.z.get_delta() * static_cast<float>(bz);
            auto Z_h = ctx.y.get_min() + ctx.z.get_delta() * static_cast<float>(1 + bz);
            loop_latex.DrawLatex(centerpos, 0.91,
                                 fmt::format("{:.0f} < {:s} < {:.2f}", Z_l, ctx.z.get_label().Data(), Z_h).c_str());
        }
        loop_latex.SetTextAlign(oldalign);
        loop_latex.SetTextColor(/*36*/ 1);

        int fitnpar = tfSig->GetNpar();
        for (int j = 0; j < fitnpar; ++j)
        {
            loop_latex.DrawLatex(0.5, 0.81 - 0.05 * j,
                                 TString::Format("[%d] %5g#pm%.2g", j, tfSig->GetParameter(j), tfSig->GetParError(j)));
        }
        loop_latex.DrawLatex(0.5, 0.25, TString::Format("#chi^{2}/ndf = %g", tfSum->GetChisquare() / tfSum->GetNDF()));
        loop_latex.DrawLatex(0.5, 0.20, TString::Format(" %.2g/%d", tfSum->GetChisquare(), tfSum->GetNDF()));
    }
}

auto distribution::fit_cells_hists(hf::fitter& hf_fitter, int signal_function_number, FitCallbackF func) -> void
{
    TLatex nofit_text;
    nofit_text.SetTextAlign(23);
    nofit_text.SetNDC();

    auto lx = cells->get_bins_x();
    auto ly = cells->get_bins_y();
    auto lz = cells->get_bins_z();

    for (auto bx = 0; (bx < lx && lx > 0) || bx == 0; ++bx)
        for (auto by = 0; (by < ly && ly > 0) || by == 0; ++by)
            for (auto bz = 0; (bz < lz && lz > 0) || bz == 0; ++bz)
            {
                TCanvas* can = nullptr;

                if (ctx.dim == dimension::DIM3)
                {
                    can = cells->get_canvas(bx, by);
                    can->cd(bz + 1);
                }
                else if (ctx.dim == dimension::DIM2)
                {
                    can = cells->get_canvas(bx);
                    can->cd(by + 1);
                }
                else if (ctx.dim == dimension::DIM1)
                {
                    can = cells->get_canvas(0);
                    can->cd(bx + 1);
                }

                TH1D* hfit = cells->get_hist(bx, by, bz);
                hfit->SetStats(0);
                hfit->Draw();
                enum class fit_search_status
                {
                    notinit,
                    fitted,
                    disabled,
                    ignored
                } info_text = fit_search_status::notinit;

                if (signal_function_number >= 0)
                {
                    auto hfp = hf_fitter.find_fit(hfit);
                    // if (!hfp) continue;

                    if (hfp and hfp->get_flag_disabled())
                    {
                        std::cout << "Fit for " << hfit->GetName() << " is disabled\n";
                        info_text = fit_search_status::disabled;
                    }
                    else

                        if (hfp or (!hfp and hf_fitter.has_generic_entry()))
                    {
                        if ((hfit->GetEntries() / hfit->GetRMS()) < 5)
                        {
                            // to low statistics to fit? Should it be for decision to the user?
                            info_text = fit_search_status::ignored;
                        }
                        else
                        {
                            info_text = fit_search_status::fitted;

                            auto res = fit_cell_hist(hfit, hf_fitter);

                            auto signal_function = res.second->get_function_object(signal_function_number);
                            auto signal_integral =
                                signal_function.Integral(hfp->get_fit_range_min(), hfp->get_fit_range_max());

                            get_signal_hist()->SetBinContent(bx + 1, by + 1, bz + 1, signal_integral);
                            get_signal_hist()->SetBinError(bx + 1, by + 1, bz + 1, sqrt(signal_integral));
                            // TODO make proper errors calculations and set it

                            if (func) func(this, gPad, hfit, res.first, bx, by, bz);
                            // if (fit_callback) fit_callback(this, sigfac, res, hfit, bx, by, bz);

                            // 						FIXME
                            // 						std::cout << "    Signal: " << res.signal << " +/- "
                            // << res.signal_err << std::endl;

                            // 						hSliceXYDiff[i]->SetBinContent(1+j, res.signal);
                            // 						hSliceXYDiff[i]->SetBinError(1+j,
                            // res.signal_err); 						hSignalCounter->SetBinContent(1+bx, 1+by,
                            // 1+bz, res.signal);
                            // hSignalCounter->SetBinError(1+bx, 1+by, 1+bz, res.signal_err);

                            // 						if (res.mean != 0)
                            // 						{
                            // 							hSliceXYFitQA[i]->SetBinContent(1+j,
                            // res.mean); 							hSliceXYFitQA[i]->SetBinError(1+j,
                            // res.sigma); 							hSliceXYChi2NDF[i]->SetBinContent(1+j,
                            // res.chi2/res.ndf);
                            // 						}
                        }
                    }
                }
                else
                {
                    // FIXME should fill the signal histogram with the integral of cell histogram
                    if (func) func(this, gPad, hfit, false, bx, by, bz);
                }

                Double_t hmax = hfit->GetBinContent(hfit->GetMaximumBin());
                hfit->GetYaxis()->SetRangeUser(0, hmax * 1.1);
                hfit->GetYaxis()->SetNdivisions(504, kTRUE);

                switch (info_text)
                {
                    case fit_search_status::fitted:
                        nofit_text.DrawLatex(0.65, 0.65, "No fit");
                        break;
                    case fit_search_status::disabled:
                        nofit_text.DrawLatex(0.65, 0.65, "Fit disabled");
                        break;
                    case fit_search_status::ignored:
                        std::cout << "This one is ignored, dunno why\n";
                        break;
                    default:
                        std::cout << "Something went wrong\n";
                        break;
                }

                // 		cSliceXYDiff->cd(1+i)/*->Draw()*/; FIXME
                // 		hSliceXYDiff[i]->Draw(h1opts); FIXME
                //     cells->getCanvas()
            }

    // 	cDiscreteXYSig->cd(); FIXME
    // 	hDiscreteXYSig->SetMarkerColor(kWhite);

    // 	if (flag_details)
    // 		gStyle->SetPaintTextFormat(".3g");
    // 		hDiscreteXYSig->Draw("colz,text10");
    // 		gStyle->SetPaintTextFormat("g");
    // 	else
    // 		hDiscreteXYSig->Draw("colz");

    // RT::NicePalette(dynamic_cast<TH2*>(sigfac->hSignalCounter.get()), 0.05f); FIXME

    // printf("Raw/fine binning counts:  %f / %f  for %s\n", sigfac->get_signal_hist()->Integral(),
    // sigfac->get_signal_hist()->Integral(), ctx.hist_name.Data());
}

auto distribution::fit_cell_hist(TH1* hist, hf::fitter& hf_fitter, double min_entries)
    -> std::pair<bool, hf::fit_entry*>
{
    auto hfp = hf_fitter.find_fit(hist);

    if (hist->Integral() < min_entries) return {false, hfp};

    // remove all saved function, potentially risky move
    // if has stored other functions than fit functions
    hist->GetListOfFunctions()->Clear();

    // do fit using FitterFactory
    auto res = hf_fitter.fit(hist, "B,Q", "");

    // if fit converged retrieve fit functions from histogram
    // otherwise nothing to do here
    if (!res.first) return res;

    // 	tfSum = dynamic_cast<TF1*<(hist->GetListOfFunctions()->At(0);
    auto tfSig = dynamic_cast<TF1*>(hist->GetListOfFunctions()->At(1));

    // do not draw Sig function in the histogram
    if (tfSig) tfSig->SetBit(TF1::kNotDraw);

    return {true, hfp};
}

// auto distribution::reset() -> void FIXME
// {
//     if (box) box->basic_distribution::reset();
//     // if (cells) cells->reset();
// }

auto distribution::print() const -> void
{
    basic_distribution::print();
    if (cells) cells->print();
}

auto distribution::validate() const -> bool
{
    if (!basic_distribution::validate()) return false;
    if (!ctx.v.get_var()) throw std::runtime_error("The x-axis variable is not set");
    return true;
}

} // namespace midas
