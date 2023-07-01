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

#include <Pandora.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TSystem.h>
#include <TVirtualArray.h>

#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

// const Option_t h1opts[] = "h,E1";

// const TString flags_fit_a = "B,Q,0";
// const TString flags_fit_b = "";

namespace midas
{

ExtraDimensionMapper::ExtraDimensionMapper(dimension dim, const std::string& name, TH1* hist, const axis_config& axis,
                                           const std::string& dir_and_name)
    : RT::Pandora(""), dim(dim), axis(axis), prefix_name(dir_and_name), ref_hist(hist)
{
    nbins_x = hist->GetNbinsX();
    nbins_y = hist->GetNbinsY();
    nbins_z = hist->GetNbinsZ();
    nhists = nbins_x * nbins_y * nbins_z;

    histograms = new TH1D*[nhists];
    if (dimension::DIM1 == dim)
        map1D(axis);
    else if (dimension::DIM2 == dim)
        map2D(axis);
    else if (dimension::DIM3 == dim)
        map3D(axis);

    //   objectsFits = new TObjArray();
    // 	objectsFits->SetName(ctx.histPrefix + "Fits");
}

ExtraDimensionMapper::ExtraDimensionMapper(dimension dim, const std::string& name, TH1* hist, const axis_config& axis,
                                           const std::string& dir_and_name, RT::Pandora* sf)
    : RT::Pandora(""), dim(dim), axis(axis), prefix_name(dir_and_name), ref_hist(hist)
{
    nbins_x = hist->GetNbinsX();
    nbins_y = hist->GetNbinsY();
    nbins_z = hist->GetNbinsZ();
    nhists = nbins_x * nbins_y * nbins_z;

    setSource(sf->getSource());
    setSourceName(sf->getSourceName());
    setTarget(sf->getTarget());
    setTargetName(sf->getTargetName());
    chdir(sf->directory_name().c_str());
    rename(sf->objects_name().c_str());

    histograms = new TH1D*[nhists];
    if (dimension::DIM1 == dim)
        map1D(axis);
    else if (dimension::DIM2 == dim)
        map2D(axis);
    else if (dimension::DIM3 == dim)
        map3D(axis);

    //   objectsFits = new TObjArray();
    // 	objectsFits->SetName(ctx.histPrefix + "Fits");
}

ExtraDimensionMapper::~ExtraDimensionMapper()
{
    gSystem->ProcessEvents();
    delete[] histograms;
}

UInt_t ExtraDimensionMapper::getBin(UInt_t x, UInt_t y, UInt_t z) const
{
    return z * (nbins_x * nbins_y) + y * nbins_x + x;
}

bool ExtraDimensionMapper::reverseBin(UInt_t bin, UInt_t& x) const
{
    if (dim != dimension::DIM1) return false;
    if (bin >= nhists) return false;
    x = bin % nbins_x;
    return true;
}

bool ExtraDimensionMapper::reverseBin(UInt_t bin, UInt_t& x, UInt_t& y) const
{
    if (dim < dimension::DIM2)
    {
        y = 0;
        return reverseBin(bin, x);
    }
    if (dim != dimension::DIM2) return false;
    if (bin >= nhists) return false;
    x = bin % nbins_x;
    y = bin / nbins_x;
    return true;
}

bool ExtraDimensionMapper::reverseBin(UInt_t bin, UInt_t& x, UInt_t& y, UInt_t& z) const
{
    if (dim < dimension::DIM3)
    {
        z = 0;
        return reverseBin(bin, x, y);
    }
    if (dim != dimension::DIM3) return false;
    if (bin >= nhists) return false;
    x = bin % nbins_x;
    y = (bin / nbins_x) % nbins_y;
    z = bin / (nbins_x * nbins_y);
    return true;
}

void ExtraDimensionMapper::map1D(const axis_config& axis)
{
    char buff[1024];
    for (UInt_t i = 0; i < nbins_x; ++i)
    {
        formatName(buff, i);
        if (axis.get_bins_array())
        {
            std::cerr << "Histogram bins arrays are not supported yet." << std::endl;
            std::abort();
        }
        else
        {
            histograms[getBin(i)] =
                RegTH1<TH1D>(buff, axis.format_hist_string(buff), axis.get_bins(), axis.get_min(), axis.get_max());
        }
    }

    formatCanvasName(buff, 0);
    canvases = new TCanvas*[1];
    canvases[0] = RegCanvas(buff, buff, 800, 600);
    canvases[0]->DivideSquare(nbins_x);
}

void ExtraDimensionMapper::map2D(const axis_config& axis)
{
    canvases = new TCanvas*[nbins_x];

    char buff[1024];
    for (UInt_t i = 0; i < nbins_x; ++i)
    {
        for (UInt_t j = 0; j < nbins_y; ++j)
        {
            formatName(buff, i, j);
            if (axis.get_bins_array())
            {
                std::cerr << "Histogram bins arrays are not supported yet." << std::endl;
                std::abort();
            }
            else
            {
                histograms[getBin(i, j)] =
                    RegTH1<TH1D>(buff, axis.format_hist_string(buff), axis.get_bins(), axis.get_min(), axis.get_max());
            }
        }
        formatCanvasName(buff, i);
        canvases[i] = RegCanvas(buff, buff, 800, 600);
        canvases[i]->DivideSquare(nbins_y);
    }
}

void ExtraDimensionMapper::map3D(const axis_config& axis)
{
    canvases = new TCanvas*[nbins_x * nbins_y];

    char buff[1024];
    for (UInt_t i = 0; i < nbins_x; ++i)
    {
        for (UInt_t j = 0; j < nbins_y; ++j)
        {
            for (UInt_t k = 0; k < nbins_z; ++k)
            {
                formatName(buff, i, j, k);
                if (axis.get_bins_array())
                {
                    std::cerr << "Histogram bins arrays are not supported yet." << std::endl;
                    std::abort();
                }
                else
                {
                    histograms[getBin(i, j, k)] = RegTH1<TH1D>(buff, axis.format_hist_string(buff), axis.get_bins(),
                                                               axis.get_min(), axis.get_max());
                }
            }
            formatCanvasName(buff, i, j);
            canvases[i + j * nbins_x] = RegCanvas(buff, buff, 800, 600);
            canvases[i + j * nbins_x]->DivideSquare(nbins_z);
        }
    }
}

void ExtraDimensionMapper::formatName(char* buff, UInt_t x, UInt_t y, UInt_t z)
{
    char name[200];
    sprintf(name, prefix_name.c_str(), 'h');

    if (dimension::DIM1 == dim)
        sprintf(buff, "%s_X%02d", name, x);
    else if (dimension::DIM2 == dim)
        sprintf(buff, "%s_X%02d_Y%02d", name, x, y);
    else if (dimension::DIM3 == dim)
        sprintf(buff, "%s_X%02d_Y%02d_Z%02d", name, x, y, z);
}

void ExtraDimensionMapper::formatCanvasName(char* buff, UInt_t x, UInt_t y)
{
    char name[200];
    sprintf(name, prefix_name.c_str(), 'c');
    if (dimension::DIM1 == dim)
        sprintf(buff, "%s", name);
    else if (dimension::DIM2 == dim)
        sprintf(buff, "%s_X%02d", name, x);
    else if (dimension::DIM3 == dim)
        sprintf(buff, "%s_X%02d_Y%02d", name, x, y);
}

// ExtraDimensionMapper & ExtraDimensionMapper::operator=(const ExtraDimensionMapper & edm)
// {
// 	ExtraDimensionMapper * nthis = this;//new ExtraDimensionMapper(fa.ctx);
//
// 	nthis->axis = edm.axis;
// 	nthis->nhists = edm.nhists;
//   objectsFits = new TObjArray();
// nthis->objectsFits->SetName(ctx.histPrefix + "Fits");
// 	for (uint i = 0; i < nthis->nhists; ++i)
// 	{
// 		copyHistogram(edm.histograms[i], nthis->histograms[i]);
// 	}
//
// 	return *nthis;
// }

TH1D* ExtraDimensionMapper::get(UInt_t x, UInt_t y, UInt_t z)
{
    if (x >= nbins_x or y >= nbins_y or z >= nbins_z) return nullptr;

    return histograms[getBin(x, y, z)];
}

TCanvas* ExtraDimensionMapper::getCanvas(UInt_t x, UInt_t y)
{
    if (dimension::DIM3 == dim)
    {
        if (x >= nbins_x or y >= nbins_y) return nullptr;

        return canvases[x + y * nbins_x];
    }
    if (dimension::DIM2 == dim)
    {
        if (x >= nbins_x) return nullptr;

        return canvases[x];
    }

    return canvases[0];
}

TVirtualPad* ExtraDimensionMapper::getPad(UInt_t x, UInt_t y, UInt_t z)
{
    TCanvas* can = getCanvas(x, y);

    if (dimension::DIM3 == dim) return can->GetPad(1 + z);
    if (dimension::DIM2 == dim) return can->GetPad(1 + y);
    if (dimension::DIM1 == dim) return can->GetPad(1 + x);

    return can->GetPad(0);
}

TH1D* ExtraDimensionMapper::find(Double_t x, Double_t y, Double_t z)
{
    UInt_t bin = ref_hist->FindBin(x, y, z);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    return histograms[getBin(bx - 1, by - 1, bz - 1)];
}

void ExtraDimensionMapper::Fill1D(Float_t x, Float_t v, Float_t w)
{
    UInt_t bin = ref_hist->FindBin(x);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= (int)nbins_x) histograms[getBin(bx - 1)]->Fill(v, w);
}

void ExtraDimensionMapper::Fill2D(Float_t x, Float_t y, Float_t v, Float_t w)
{
    UInt_t bin = ref_hist->FindBin(x, y);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= (int)nbins_x && by > 0 && by <= (int)nbins_y) histograms[getBin(bx - 1, by - 1)]->Fill(v, w);
}

void ExtraDimensionMapper::Fill3D(Float_t x, Float_t y, Float_t z, Float_t v, Float_t w)
{
    UInt_t bin = ref_hist->FindBin(x, y, z);
    Int_t bx, by, bz;
    ref_hist->GetBinXYZ(bin, bx, by, bz);
    if (bx > 0 && bx <= (int)nbins_x && by > 0 && by <= (int)nbins_y && bz > 0 && bz <= (int)nbins_z)
        histograms[getBin(bx - 1, by - 1, bz - 1)]->Fill(v, w);
}

} // namespace midas