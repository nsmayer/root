//This macro is based on labels1.C by Rene Brun.
//Updated by Timur Pocheptsov to use transparent text.

//Includes for ACLiC (cling does not need them).
#include "TVirtualX.h"
#include "TPaveText.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "Rtypes.h"
#include "TError.h"
#include "TColor.h"
#include "TH1F.h"

//Aux. functions for tutorials/gl.
#include "customcolorgl.h"

void transp_text()
{
   //1. Some sanity check.
   if (!gRandom) {
      Error("transp_text", "gRandom is null");
      return;
   }


   //2. Try to 'allocate' free indices for our custom colors -
   //we can use hard-coded indices like 1001, 1002, 1003 ... but
   //I prefer to find free indices in a ROOT's color table
   //to avoid possible conflicts with other tutorials.
   Int_t indices[2] = {};
   if (ROOT::GLTutorials::FindFreeCustomColorIndices(indices) != 2) {
      Error("transp_text", "failed to create new custom colors");
      return;
   }

   //3. Create special transparent colors for both pavetext fill color and text color.
   const Int_t grayColorIndex = indices[0], blackColorIndex = indices[1];
   new TColor(grayColorIndex, 0.8, 0.8, 0.8, "transparent_gray", 0.85);
   new TColor(blackColorIndex, 0., 0., 0., "transparent_black", 0.5);

   //4. Create a TCanvas.
   gStyle->SetCanvasPreferGL(kTRUE);
   TCanvas * const c1 = new TCanvas("transparent text","transparent text demo", 10, 10, 900, 500);

   const Int_t nx = 20;
   const char *people[nx] = {"Jean","Pierre","Marie","Odile",
      "Sebastien","Fons","Rene","Nicolas","Xavier","Greg",
      "Bjarne","Anton","Otto","Eddy","Peter","Pasha",
      "Philippe","Suzanne","Jeff","Valery"};

   c1->SetGrid();
   c1->SetBottomMargin(0.15);

   TH1F * const h = new TH1F("h4", "test", nx, 0, nx);

   h->SetFillColor(38);
   for (Int_t i = 0; i < 5000; ++i)
      h->Fill(gRandom->Gaus(0.5 * nx, 0.2 * nx));

   h->SetStats(0);
   for (Int_t i = 1; i <= nx; ++i)
      h->GetXaxis()->SetBinLabel(i, people[i - 1]);

   h->Draw();
   
   TPaveText * const pt = new TPaveText(0.3, 0.3, 0.98, 0.98, "brNDC");
   //Transparent 'rectangle' with transparent text.
   pt->SetFillColor(grayColorIndex);
   pt->SetTextColor(blackColorIndex);

   pt->SetTextSize(0.5);
   pt->SetTextAlign(12);
   
   pt->AddText("Hello");
   pt->Draw();
}
