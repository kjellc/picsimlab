/* ########################################################################

   PICSimLab - Programmable IC Simulator Laboratory

   ########################################################################

   Copyright (c) : 2010-2024  Luis Claudio Gambôa Lopes <lcgamboa@yahoo.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   For e-mail suggestions :  lcgamboa@yahoo.com
   ######################################################################## */

#include "output_7s_Display.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

#define LED_COLOR 1 // R=1, G=2, Y=3, B=4, M=5, C=6, W=7

// RGB: Red
inline unsigned int RgbR(unsigned int v)
{
    switch (LED_COLOR) {
    case 1: return v;
    case 2: return 0;
    case 3: return v;
    case 4: return 0;
    case 5: return v;
    case 6: return 0;
    default: return v;
    }
}

// RGB: Green
inline unsigned int RgbG(unsigned int v)
{
    switch (LED_COLOR) {
    case 1: return 0;
    case 2: return v;
    case 3: return v;
    case 4: return 0;
    case 5: return 0;
    case 6: return v;
    default: return v;
    }
}

// RGB: Blue
inline unsigned int RgbB(unsigned int v)
{
    switch (LED_COLOR) {
    case 1: return 0;
    case 2: return 0;
    case 3: return 0;
    case 4: return v;
    case 5: return v;
    case 6: return v;
    default: return v;
    }
}

/* outputs */
enum {
    O_A1,
    O_B1,
    O_C1,
    O_D1,
    O_E1,
    O_F1,
    O_G1,
    O_P1,
    O_SS1,
    O_A2,
    O_B2,
    O_C2,
    O_D2,
    O_E2,
    O_F2,
    O_G2,
    O_P2,
    O_SS2,
    O_A3,
    O_B3,
    O_C3,
    O_D3,
    O_E3,
    O_F3,
    O_G3,
    O_P3,
    O_SS3,
    O_A4,
    O_B4,
    O_C4,
    O_D4,
    O_E4,
    O_F4,
    O_G4,
    O_P4,
    O_SS4,
    O_SA,
    O_SB,
    O_SC,
    O_SD,
    O_SE,
    O_SF,
    O_SG,
    O_SP,
    O_DISP1,
    O_DISP2,
    O_DISP3,
    O_DISP4,
    O_FX1
};

static PCWProp pcwprop[16] = {{PCW_COMBO, "Seg a"},  {PCW_COMBO, "Seg b"}, {PCW_COMBO, "Seg c"}, {PCW_COMBO, "Seg d"},
                              {PCW_COMBO, "Seg e"},  {PCW_COMBO, "Seg f"}, {PCW_COMBO, "Seg g"}, {PCW_COMBO, "Point"},
                              {PCW_COMBO, "D1"},     {PCW_COMBO, "D2"},    {PCW_COMBO, "D3"},    {PCW_COMBO, "D4"},
                              {PCW_COMBO, "Active"}, {PCW_COMBO, "Type"},  {PCW_END, ""},        {PCW_END, ""}};

cpart_7s_display::cpart_7s_display(const unsigned x, const unsigned y, const char* name, const char* type,
                                   board* pboard_, const int id_)
    : part(x, y, name, type, pboard_, id_) {
    X = x;
    Y = y;
    always_update = 1;

    active = 1;

    ReadMaps();

    LoadPartImage();

    input_pins[0] = 0;
    input_pins[1] = 0;
    input_pins[2] = 0;
    input_pins[3] = 0;
    input_pins[4] = 0;
    input_pins[5] = 0;
    input_pins[6] = 0;
    input_pins[7] = 0;
    input_pins[8] = 0;
    input_pins[9] = 0;
    input_pins[10] = 0;
    input_pins[11] = 0;

    mcount = 0;

    for (int i = 0; i < 8; i++) {
        lm1[i] = 30;
        lm2[i] = 30;
        lm3[i] = 30;
        lm4[i] = 30;
    }

    memset(alm1, 0, 8 * sizeof(unsigned int));
    memset(alm2, 0, 8 * sizeof(unsigned int));
    memset(alm3, 0, 8 * sizeof(unsigned int));
    memset(alm4, 0, 8 * sizeof(unsigned int));

    dtype = 1;  // to force dtype change
    ChangeType(0);

    SetPCWProperties(pcwprop);

    PinCount = 12;
    Pins = input_pins;
}

cpart_7s_display::~cpart_7s_display(void) {
    SpareParts.SetPartOnDraw(id);
    SpareParts.CanvasCmd({.cmd = CC_FREEBITMAP, .FreeBitmap{BitmapId}});
    SpareParts.CanvasCmd({.cmd = CC_DESTROY});
}

void cpart_7s_display::DrawOutput(const unsigned int i) {
    SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{30, 0, 0}});

    switch (output[i].id) {
        case O_SA:
        case O_SB:
        case O_SC:
        case O_SD:
        case O_SE:
        case O_SF:
        case O_SG:
        case O_SP:
        case O_DISP1:
        case O_DISP2:
        case O_DISP3:
        case O_DISP4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{255, 255, 255}});
            if (input_pins[output[i].id - O_SA] == 0)
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"NC", output[i].x1, output[i].y2, 90.0}});
            else
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT,
                                      .RotatedText{SpareParts.GetPinName(input_pins[output[i].id - O_SA]).c_str(),
                                                   output[i].x1, output[i].y2, 90.0}});
            break;
        case O_FX1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{155, 155, 155}});
            if (active) {
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"GND", output[i].x1, output[i].y2, 90.0}});
            } else {
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"+5V", output[i].x1, output[i].y2, 90.0}});
            }
            break;
        case O_A1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[0]), RgbG(lm1[0]), RgbB(lm1[0])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_B1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[1]), RgbG(lm1[1]), RgbB(lm1[1])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_C1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[2]), RgbG(lm1[2]), RgbB(lm1[2])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_D1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[3]), RgbG(lm1[3]), RgbB(lm1[3])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_E1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[4]), RgbG(lm1[4]), RgbB(lm1[4])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_F1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[5]), RgbG(lm1[5]), RgbB(lm1[5])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_G1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[6]), RgbG(lm1[6]), RgbB(lm1[6])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_P1:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm1[7]), RgbG(lm1[7]), RgbB(lm1[7])}});
            SpareParts.CanvasCmd({.cmd = CC_CIRCLE, .Circle{1, output[i].x1, output[i].y1, output[i].r}});
            break;

        case O_A2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[0]), RgbG(lm2[0]), RgbB(lm2[0])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_B2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[1]), RgbG(lm2[1]), RgbB(lm2[1])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_C2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[2]), RgbG(lm2[2]), RgbB(lm2[2])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_D2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[3]), RgbG(lm2[3]), RgbB(lm2[3])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_E2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[4]), RgbG(lm2[4]), RgbB(lm2[4])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_F2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[5]), RgbG(lm2[5]), RgbB(lm2[5])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_G2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[6]), RgbG(lm2[6]), RgbB(lm2[6])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_P2:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm2[7]), RgbG(lm2[7]), RgbB(lm2[7])}});
            SpareParts.CanvasCmd({.cmd = CC_CIRCLE, .Circle{1, output[i].x1, output[i].y1, output[i].r}});
            break;

        case O_A3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[0]), RgbG(lm3[0]), RgbB(lm3[0])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_B3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[1]), RgbG(lm3[1]), RgbB(lm3[1])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_C3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[2]), RgbG(lm3[2]), RgbB(lm3[2])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_D3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[3]), RgbG(lm3[3]), RgbB(lm3[3])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_E3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[4]), RgbG(lm3[4]), RgbB(lm3[4])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_F3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[5]), RgbG(lm3[5]), RgbB(lm3[5])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_G3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[6]), RgbG(lm3[6]), RgbB(lm3[6])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_P3:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm3[7]), RgbG(lm3[7]), RgbB(lm3[7])}});
            SpareParts.CanvasCmd({.cmd = CC_CIRCLE, .Circle{1, output[i].x1, output[i].y1, output[i].r}});
            break;
            break;
        case O_A4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[0]), RgbG(lm4[0]), RgbB(lm4[0])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_B4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[1]), RgbG(lm4[1]), RgbB(lm4[1])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_C4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[2]), RgbG(lm4[2]), RgbB(lm4[2])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_D4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[3]), RgbG(lm4[3]), RgbB(lm4[3])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_E4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[4]), RgbG(lm4[4]), RgbB(lm4[4])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_F4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[5]), RgbG(lm4[5]), RgbB(lm4[5])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_G4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[6]), RgbG(lm4[6]), RgbB(lm4[6])}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
        case O_P4:
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{RgbR(lm4[7]), RgbG(lm4[7]), RgbB(lm4[7])}});
            SpareParts.CanvasCmd({.cmd = CC_CIRCLE, .Circle{1, output[i].x1, output[i].y1, output[i].r}});
            break;
        case O_SS1:
            output_ids[O_A1]->update = 1;
            output_ids[O_B1]->update = 1;
            output_ids[O_C1]->update = 1;
            output_ids[O_D1]->update = 1;
            output_ids[O_E1]->update = 1;
            output_ids[O_F1]->update = 1;
            output_ids[O_G1]->update = 1;
            output_ids[O_P1]->update = 1;
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{10, 10, 10}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
        case O_SS2:
            output_ids[O_A2]->update = 1;
            output_ids[O_B2]->update = 1;
            output_ids[O_C2]->update = 1;
            output_ids[O_D2]->update = 1;
            output_ids[O_E2]->update = 1;
            output_ids[O_F2]->update = 1;
            output_ids[O_G2]->update = 1;
            output_ids[O_P2]->update = 1;
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{10, 10, 10}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
        case O_SS3:
            output_ids[O_A3]->update = 1;
            output_ids[O_B3]->update = 1;
            output_ids[O_C3]->update = 1;
            output_ids[O_D3]->update = 1;
            output_ids[O_E3]->update = 1;
            output_ids[O_F3]->update = 1;
            output_ids[O_G3]->update = 1;
            output_ids[O_P3]->update = 1;
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{10, 10, 10}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
        case O_SS4:
            output_ids[O_A4]->update = 1;
            output_ids[O_B4]->update = 1;
            output_ids[O_C4]->update = 1;
            output_ids[O_D4]->update = 1;
            output_ids[O_E4]->update = 1;
            output_ids[O_F4]->update = 1;
            output_ids[O_G4]->update = 1;
            output_ids[O_P4]->update = 1;
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{10, 10, 10}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            break;
    }
}

unsigned short cpart_7s_display::GetInputId(char* name) {
    printf("Error input '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

unsigned short cpart_7s_display::GetOutputId(char* name) {
    if (strcmp(name, "SS_A1") == 0)
        return O_A1;
    if (strcmp(name, "SS_B1") == 0)
        return O_B1;
    if (strcmp(name, "SS_C1") == 0)
        return O_C1;
    if (strcmp(name, "SS_D1") == 0)
        return O_D1;
    if (strcmp(name, "SS_E1") == 0)
        return O_E1;
    if (strcmp(name, "SS_F1") == 0)
        return O_F1;
    if (strcmp(name, "SS_G1") == 0)
        return O_G1;
    if (strcmp(name, "SS_P1") == 0)
        return O_P1;

    if (strcmp(name, "SS_A2") == 0)
        return O_A2;
    if (strcmp(name, "SS_B2") == 0)
        return O_B2;
    if (strcmp(name, "SS_C2") == 0)
        return O_C2;
    if (strcmp(name, "SS_D2") == 0)
        return O_D2;
    if (strcmp(name, "SS_E2") == 0)
        return O_E2;
    if (strcmp(name, "SS_F2") == 0)
        return O_F2;
    if (strcmp(name, "SS_G2") == 0)
        return O_G2;
    if (strcmp(name, "SS_P2") == 0)
        return O_P2;

    if (strcmp(name, "SS_A3") == 0)
        return O_A3;
    if (strcmp(name, "SS_B3") == 0)
        return O_B3;
    if (strcmp(name, "SS_C3") == 0)
        return O_C3;
    if (strcmp(name, "SS_D3") == 0)
        return O_D3;
    if (strcmp(name, "SS_E3") == 0)
        return O_E3;
    if (strcmp(name, "SS_F3") == 0)
        return O_F3;
    if (strcmp(name, "SS_G3") == 0)
        return O_G3;
    if (strcmp(name, "SS_P3") == 0)
        return O_P3;

    if (strcmp(name, "SS_A4") == 0)
        return O_A4;
    if (strcmp(name, "SS_B4") == 0)
        return O_B4;
    if (strcmp(name, "SS_C4") == 0)
        return O_C4;
    if (strcmp(name, "SS_D4") == 0)
        return O_D4;
    if (strcmp(name, "SS_E4") == 0)
        return O_E4;
    if (strcmp(name, "SS_F4") == 0)
        return O_F4;
    if (strcmp(name, "SS_G4") == 0)
        return O_G4;
    if (strcmp(name, "SS_P4") == 0)
        return O_P4;

    if (strcmp(name, "PN_SA") == 0)
        return O_SA;
    if (strcmp(name, "PN_SB") == 0)
        return O_SB;
    if (strcmp(name, "PN_SC") == 0)
        return O_SC;
    if (strcmp(name, "PN_SD") == 0)
        return O_SD;
    if (strcmp(name, "PN_SE") == 0)
        return O_SE;
    if (strcmp(name, "PN_SF") == 0)
        return O_SF;
    if (strcmp(name, "PN_SG") == 0)
        return O_SG;
    if (strcmp(name, "PN_SP") == 0)
        return O_SP;

    if (strcmp(name, "PN_DISP1") == 0)
        return O_DISP1;
    if (strcmp(name, "PN_DISP2") == 0)
        return O_DISP2;
    if (strcmp(name, "PN_DISP3") == 0)
        return O_DISP3;
    if (strcmp(name, "PN_DISP4") == 0)
        return O_DISP4;

    if (strcmp(name, "PN_FX1") == 0)
        return O_FX1;

    if (strcmp(name, "SS_1") == 0)
        return O_SS1;
    if (strcmp(name, "SS_2") == 0)
        return O_SS2;
    if (strcmp(name, "SS_3") == 0)
        return O_SS3;
    if (strcmp(name, "SS_4") == 0)
        return O_SS4;

    printf("Error output '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

std::string cpart_7s_display::WritePreferences(void) {
    char prefs[256];

    sprintf(prefs, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", input_pins[0],
            input_pins[1], input_pins[2], input_pins[3], input_pins[4], input_pins[5], input_pins[6], input_pins[7],
            input_pins[8], input_pins[9], input_pins[10], input_pins[11], active, dtype);

    return prefs;
}

void cpart_7s_display::ReadPreferences(std::string value) {
    unsigned char dtype_;
    sscanf(value.c_str(), "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &input_pins[0],
           &input_pins[1], &input_pins[2], &input_pins[3], &input_pins[4], &input_pins[5], &input_pins[6],
           &input_pins[7], &input_pins[8], &input_pins[9], &input_pins[10], &input_pins[11], &active, &dtype_);
    ChangeType(dtype_);
}

void cpart_7s_display::RegisterRemoteControl(void) {
    output_ids[O_A1]->status = (void*)&lm1[0];
    output_ids[O_B1]->status = (void*)&lm1[1];
    output_ids[O_C1]->status = (void*)&lm1[2];
    output_ids[O_D1]->status = (void*)&lm1[3];
    output_ids[O_E1]->status = (void*)&lm1[4];
    output_ids[O_F1]->status = (void*)&lm1[5];
    output_ids[O_G1]->status = (void*)&lm1[6];
    output_ids[O_P1]->status = (void*)&lm1[7];
    output_ids[O_A2]->status = (void*)&lm2[0];
    output_ids[O_B2]->status = (void*)&lm2[1];
    output_ids[O_C2]->status = (void*)&lm2[2];
    output_ids[O_D2]->status = (void*)&lm2[3];
    output_ids[O_E2]->status = (void*)&lm2[4];
    output_ids[O_F2]->status = (void*)&lm2[5];
    output_ids[O_G2]->status = (void*)&lm2[6];
    output_ids[O_P2]->status = (void*)&lm2[7];
    output_ids[O_A3]->status = (void*)&lm3[0];
    output_ids[O_B3]->status = (void*)&lm3[1];
    output_ids[O_C3]->status = (void*)&lm3[2];
    output_ids[O_D3]->status = (void*)&lm3[3];
    output_ids[O_E3]->status = (void*)&lm3[4];
    output_ids[O_F3]->status = (void*)&lm3[5];
    output_ids[O_G3]->status = (void*)&lm3[6];
    output_ids[O_P3]->status = (void*)&lm3[7];
    output_ids[O_A4]->status = (void*)&lm4[0];
    output_ids[O_B4]->status = (void*)&lm4[1];
    output_ids[O_C4]->status = (void*)&lm4[2];
    output_ids[O_D4]->status = (void*)&lm4[3];
    output_ids[O_E4]->status = (void*)&lm4[4];
    output_ids[O_F4]->status = (void*)&lm4[5];
    output_ids[O_G4]->status = (void*)&lm4[6];
    output_ids[O_P4]->status = (void*)&lm4[7];
}

void cpart_7s_display::ConfigurePropertiesWindow(void) {
    SetPCWComboWithPinNames("combo1", input_pins[0]);
    SetPCWComboWithPinNames("combo2", input_pins[1]);
    SetPCWComboWithPinNames("combo3", input_pins[2]);
    SetPCWComboWithPinNames("combo4", input_pins[3]);
    SetPCWComboWithPinNames("combo5", input_pins[4]);
    SetPCWComboWithPinNames("combo6", input_pins[5]);
    SetPCWComboWithPinNames("combo7", input_pins[6]);
    SetPCWComboWithPinNames("combo8", input_pins[7]);
    SetPCWComboWithPinNames("combo9", input_pins[8]);
    SetPCWComboWithPinNames("combo10", input_pins[9]);
    SetPCWComboWithPinNames("combo11", input_pins[10]);
    SetPCWComboWithPinNames("combo12", input_pins[11]);

    SpareParts.WPropCmd("combo13", PWA_COMBOSETITEMS, "HIGH,LOW,");
    if (active)
        SpareParts.WPropCmd("combo13", PWA_COMBOSETTEXT, "HIGH");
    else
        SpareParts.WPropCmd("combo13", PWA_COMBOSETTEXT, "LOW ");

    SpareParts.WPropCmd("combo14", PWA_COMBOSETITEMS, "4 Mux.,Single,");
    SpareParts.WPropCmd("combo14", PWA_COMBOPROPEV, "1");
    if (dtype == 0)
        SpareParts.WPropCmd("combo14", PWA_COMBOSETTEXT, "4 Mux.");
    else {
        SpareParts.WPropCmd("combo14", PWA_COMBOSETTEXT, "Single");
    }

    if (dtype == 0) {
        SpareParts.WPropCmd("combo9", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo10", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo11", PWA_SETENABLE, " 1");
        SpareParts.WPropCmd("combo12", PWA_SETENABLE, " 1");
    } else {
        SpareParts.WPropCmd("combo9", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo10", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo11", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo12", PWA_SETENABLE, "0");
    }
}

void cpart_7s_display::ReadPropertiesWindow(void) {
    input_pins[0] = GetPWCComboSelectedPin("combo1");
    input_pins[1] = GetPWCComboSelectedPin("combo2");
    input_pins[2] = GetPWCComboSelectedPin("combo3");
    input_pins[3] = GetPWCComboSelectedPin("combo4");
    input_pins[4] = GetPWCComboSelectedPin("combo5");
    input_pins[5] = GetPWCComboSelectedPin("combo6");
    input_pins[6] = GetPWCComboSelectedPin("combo7");
    input_pins[7] = GetPWCComboSelectedPin("combo8");
    input_pins[8] = GetPWCComboSelectedPin("combo9");
    input_pins[9] = GetPWCComboSelectedPin("combo10");
    input_pins[10] = GetPWCComboSelectedPin("combo11");
    input_pins[11] = GetPWCComboSelectedPin("combo12");

    char buff[64];
    SpareParts.WPropCmd("combo13", PWA_COMBOGETTEXT, NULL, buff);
    active = (strcmp(buff, "HIGH") == 0);

    SpareParts.WPropCmd("combo14", PWA_COMBOGETTEXT, NULL, buff);

    unsigned char dtype_ = (strcmp(buff, "4 Mux.")) != 0;

    ChangeType(dtype_);
}

void cpart_7s_display::PreProcess(void) {
    memset(alm1, 0, 8 * sizeof(unsigned int));

    if (!dtype) {
        memset(alm2, 0, 8 * sizeof(unsigned int));
        memset(alm3, 0, 8 * sizeof(unsigned int));
        memset(alm4, 0, 8 * sizeof(unsigned int));
    }

    JUMPSTEPS_ = PICSimLab.GetJUMPSTEPS() * 4.0 / PICSimLab.GetBoard()->MGetClocksPerInstructions();
    mcount = JUMPSTEPS_;
}

void cpart_7s_display::Process(void) {
    int i;
    const picpin* ppins = SpareParts.GetPinsValues();

    mcount++;

    if (mcount > JUMPSTEPS_) {
        for (i = 0; i < 8; i++) {
            if (input_pins[i]) {
                if (ppins[input_pins[i] - 1].value == active) {
                    if (dtype) {
                        alm1[i]++;
                    } else {
                        if (ppins[input_pins[8] - 1].value == 0)
                            alm1[i]++;
                        if (ppins[input_pins[9] - 1].value == 0)
                            alm2[i]++;
                        if (ppins[input_pins[10] - 1].value == 0)
                            alm3[i]++;
                        if (ppins[input_pins[11] - 1].value == 0)
                            alm4[i]++;
                    }
                }
            }
        }
        mcount = -1;
    }
}

void cpart_7s_display::PostProcess(void) {
    for (int i = 0; i < 8; i++) {
        lm1[i] = alm1[i] ? 255 : 0;
        if (lm1[i] > 255)
            lm1[i] = 255;
        if (!dtype) {
            lm2[i] = alm2[i] ? 255 : 0;
            lm3[i] = alm3[i] ? 255 : 0;
            lm4[i] = alm4[i] ? 255 : 0;
            if (lm2[i] > 255)
                lm2[i] = 255;
            if (lm3[i] > 255)
                lm3[i] = 255;
            if (lm4[i] > 255)
                lm4[i] = 255;
        }
    }

    for (int i = 0; i < 8; i++) {
        // 7s DISP
        if (output_ids[O_A1 + i]->value != lm1[i]) {
            output_ids[O_A1 + i]->value = lm1[i];
            output_ids[O_SS1]->update = 1;
        }
        if (!dtype) {
            if (output_ids[O_A2 + i]->value != lm2[i]) {
                output_ids[O_A2 + i]->value = lm2[i];
                output_ids[O_SS2]->update = 1;
            }
            if (output_ids[O_A3 + i]->value != lm3[i]) {
                output_ids[O_A3 + i]->value = lm3[i];
                output_ids[O_SS3]->update = 1;
            }
            if (output_ids[O_A4 + i]->value != lm4[i]) {
                output_ids[O_A4 + i]->value = lm4[i];
                output_ids[O_SS4]->update = 1;
            }
        }
    }
}

void cpart_7s_display::ComboChange(const char* controlname, std::string value) {
    unsigned char dtype_ = (value.compare("4 Mux.")) != 0;
    ChangeType(dtype_);
}

std::string cpart_7s_display::GetPictureFileName(void) {
    switch (dtype) {
        case 0:
            return "7 Segments Display/part.svg";
            break;
        case 1:
            return "7 Segments Display/part1.svg";
            break;
    }

    return "7 Segments Display/part.svg";
}

std::string cpart_7s_display::GetMapFile(void) {
    switch (dtype) {
        case 0:
            return "7 Segments Display/part.map";
            break;
        case 1:
            return "7 Segments Display/part1.map";
            break;
    }

    return "7 Segments Display/part.map";
}

void cpart_7s_display::ChangeType(unsigned char tp) {
    // if same
    if (tp == dtype)
        return;

    if (BitmapId >= 0) {
        SpareParts.SetPartOnDraw(id);
        SpareParts.CanvasCmd({.cmd = CC_FREEBITMAP, .FreeBitmap{BitmapId}});
        SpareParts.CanvasCmd({.cmd = CC_DESTROY});
        BitmapId = -1;
    }

    dtype = tp;

    ReadMaps();

    LoadPartImage();
}

part_init(PART_7S_DISPLAY_Name, cpart_7s_display, "Output");
