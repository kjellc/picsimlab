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

#include "input_lm35.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

/* outputs */
enum { O_P1, O_F1, O_F2, O_PO1 };

/* inputs */
enum { I_PO1 };

static PCWProp pcwprop[4] = {{PCW_LABEL, "1 - VCC,+5V"},
                             {PCW_COMBO, "2 - OUT"},
                             {PCW_LABEL, "3 - GND ,GND"},
                             {PCW_END, ""}};

cpart_lm35::cpart_lm35(const unsigned x, const unsigned y, const char* name, const char* type, board* pboard_,
                       const int id_)
    : part(x, y, name, type, pboard_, id_) {
    output_pins[0] = 0;

    value = 0;
    active = 0;

    SetPCWProperties(pcwprop);

    PinCount = 1;
    Pins = output_pins;
}

void cpart_lm35::RegisterRemoteControl(void) {
    input_ids[I_PO1]->status = &value;
    input_ids[I_PO1]->update = &output_ids[O_PO1]->update;
}

cpart_lm35::~cpart_lm35(void) {
    SpareParts.SetPartOnDraw(id);
    SpareParts.CanvasCmd({CC_FREEBITMAP, .FreeBitmap{BitmapId}});
    SpareParts.CanvasCmd({CC_DESTROY});
}

void cpart_lm35::DrawOutput(const unsigned int i) {
    char val[10];

    switch (output[i].id) {
        case O_P1:
            SpareParts.CanvasCmd({CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd({CC_RECTANGLE, .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                                           output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({CC_SETFGCOLOR, .SetFgColor{255, 255, 255}});
            if (output_pins[output[i].id - O_P1] == 0)
                SpareParts.CanvasCmd({CC_ROTATEDTEXT, .RotatedText{"NC", output[i].x1, output[i].y2, 90}});
            else
                SpareParts.CanvasCmd(
                    {CC_ROTATEDTEXT, .RotatedText{SpareParts.GetPinName(output_pins[output[i].id - O_P1]).c_str(),
                                                  output[i].x1, output[i].y2, 90}});
            break;
        case O_F1:
            SpareParts.CanvasCmd({CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd({CC_RECTANGLE, .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                                           output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({CC_SETFGCOLOR, .SetFgColor{155, 155, 155}});
            SpareParts.CanvasCmd({CC_ROTATEDTEXT, .RotatedText{"+5V", output[i].x1, output[i].y2, 90}});
            break;
        case O_F2:
            SpareParts.CanvasCmd({CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd({CC_RECTANGLE, .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                                           output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({CC_SETFGCOLOR, .SetFgColor{155, 155, 155}});
            SpareParts.CanvasCmd({CC_ROTATEDTEXT, .RotatedText{"GND", output[i].x1, output[i].y2, 90}});
            break;
        case O_PO1:
            snprintf(val, 10, "%5.1f", (0.74 * (200 - value) + 2));
            DrawSlider(SpareParts.CanvasCmd, &output[i], value, val, 7);
            SpareParts.CanvasCmd({CC_SETFONTSIZE, .SetFontSize{9}});
            break;
    }
}

void cpart_lm35::PostProcess(void) {
    if (output_pins[0]) {
        SpareParts.SetAPin(output_pins[0], (0.0074 * (200 - value) + 0.02));
    }
}

void cpart_lm35::OnMouseButtonPress(uint inputId, uint button, uint x, uint y, uint state) {
    switch (inputId) {
        case I_PO1:
            value = (y - input_ids[I_PO1]->y1) * 1.66;
            if (value > 200)
                value = 200;
            active = 1;
            output_ids[O_PO1]->update = 1;
            break;
    }
}

void cpart_lm35::OnMouseButtonRelease(uint inputId, uint button, uint x, uint y, uint state) {
    switch (inputId) {
        case I_PO1:
            active = 0;
            output_ids[O_PO1]->update = 1;
            break;
    }
}

void cpart_lm35::OnMouseMove(uint inputId, uint button, uint x, uint y, uint state) {
    switch (inputId) {
        case I_PO1:
            if (active) {
                value = (y - input_ids[I_PO1]->y1) * 1.66;
                if (value > 200)
                    value = 200;
                output_ids[O_PO1]->update = 1;
            }
            break;
        default:
            active = 0;
            break;
    }
}

unsigned short cpart_lm35::GetInputId(char* name) {
    if (strcmp(name, "PO_1") == 0)
        return I_PO1;

    printf("Error input '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

unsigned short cpart_lm35::GetOutputId(char* name) {
    if (strcmp(name, "PN_1") == 0)
        return O_P1;

    if (strcmp(name, "PN_F1") == 0)
        return O_F1;
    if (strcmp(name, "PN_F2") == 0)
        return O_F2;

    if (strcmp(name, "PO_1") == 0)
        return O_PO1;

    printf("Error output '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

std::string cpart_lm35::WritePreferences(void) {
    char prefs[256];

    sprintf(prefs, "%hhu,%hhu", output_pins[0], value);

    return prefs;
}

void cpart_lm35::ReadPreferences(std::string value_) {
    sscanf(value_.c_str(), "%hhu,%hhu", &output_pins[0], &value);
}

void cpart_lm35::ConfigurePropertiesWindow(void) {
    SetPCWComboWithPinNames("combo2", output_pins[0]);
}

void cpart_lm35::ReadPropertiesWindow(void) {
    output_pins[0] = GetPWCComboSelectedPin("combo2");
}

part_init(PART_LM35_Name, cpart_lm35, "Input");
