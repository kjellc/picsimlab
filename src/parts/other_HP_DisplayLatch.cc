/* ########################################################################

   PICSimLab - Programmable IC Simulator Laboratory

   ########################################################################

   Copyright (c) : 2019-2023  Luis Claudio Gambôa Lopes <lcgamboa@yahoo.com>

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

#include "other_HP_DisplayLatch.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

#include <cstring>

// --------------------------------------------------------------------------
//   HP display latch  kjc: 2024-11
//   Anode inputs, a,b,c,d,e,f,g,h (8 inputs) latched when active (8 outputs)
//   STR (1 input) activate the current cathode column, and increments to the next (15 outputs)
//   RCD (1 input) resets the column counter to 0
// --------------------------------------------------------------------------

/* map ids (indexies in the output[] array, also in output[].id ) */
enum {
    I_AA, I_AB, I_AC, I_AD, I_AE, I_AF, I_AH, I_AG,         // input anodes
    I_STR,
    I_RCD,
    O_AA, O_AB, O_AC, O_AD, O_AE, O_AF, O_AH, O_AG,         // output anodes
    O_CC1, O_CC2, O_CC3, O_CC4, O_CC5, O_CC6, O_CC7, O_CC8, // output cathodes
    O_CC9, O_CC10, O_CC11, O_CC12, O_CC13, O_CC14, O_CC15,
    O_IC /* for rendering the IC */
};

const char pin_names[33][5] = {"IAA", "IAB", "IAC", "IAD", "IAE", "IAF", "IAG", "IAH", "STR", "RCD",
                               "OAA", "OAB", "OAC", "OAD", "OAE", "OAF", "OAG", "OAH",
                               "CC1", "CC2", "CC3", "CC4", "CC5", "CC6", "CC7", "CC8", "CC9", "CC10", "CC11", "CC12", "CC13", "CC14", "CC15"};

static PCWProp pcwprop[34] = {{PCW_COMBO, "1-IAA"}, {PCW_COMBO, "2-IAB"}, {PCW_COMBO, "3-IAC"}, {PCW_COMBO, "4-IAD"},
                              {PCW_COMBO, "5-IAE"}, {PCW_COMBO, "6-IAF"}, {PCW_COMBO, "7-IAG"}, {PCW_COMBO, "8-IAH"},
                              {PCW_COMBO, "9-STR"}, {PCW_COMBO, "10-RCD"},
                              {PCW_LABEL, "11-OAA,NC"}, {PCW_LABEL, "12-OAB,NC"}, {PCW_LABEL, "13-OAC,NC"}, {PCW_LABEL, "14-OAD,NC"},
                              {PCW_LABEL, "15-OAE,NC"}, {PCW_LABEL, "16-OAF,NC"}, {PCW_LABEL, "17-OAG,NC"}, {PCW_LABEL, "18-OAH,NC"},
                              {PCW_LABEL, "19-CC1,NC"}, {PCW_LABEL, "20-CC2,NC"}, {PCW_LABEL, "21-CC3,NC"}, {PCW_LABEL, "22-CC4,NC"},
                              {PCW_LABEL, "23-CC5,NC"}, {PCW_LABEL, "24-CC6,NC"}, {PCW_LABEL, "25-CC7,NC"}, {PCW_LABEL, "26-CC8,NC"},
                              {PCW_LABEL, "27-CC9,NC"}, {PCW_LABEL, "28,CC10,NC"},{PCW_LABEL, "29-CC11,NC"},{PCW_LABEL, "30-CC12,NC"},
                              {PCW_LABEL, "31-CC13,NC"},{PCW_LABEL, "32,CC14,NC"},{PCW_LABEL, "33-CC15,NC"},{PCW_END, ""}};

cpart_HP_DisplayLatch::cpart_HP_DisplayLatch(const unsigned x, const unsigned y,
                                             const char* name,
                                             const char* type,
                                             board* pboard_,
                                             const int id_)
    : part(x, y, name, type, pboard_, id_)
{
    X = x;
    Y = y;
    always_update = 1;
    ReadMaps();
    BitmapId = -1;

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

    output_pins[0] = SpareParts.RegisterIOpin("OAA");
    output_pins[1] = SpareParts.RegisterIOpin("OAB");
    output_pins[2] = SpareParts.RegisterIOpin("OAC");
    output_pins[3] = SpareParts.RegisterIOpin("OAD");
    output_pins[4] = SpareParts.RegisterIOpin("OAE");
    output_pins[5] = SpareParts.RegisterIOpin("OAF");
    output_pins[6] = SpareParts.RegisterIOpin("OAG");
    output_pins[7] = SpareParts.RegisterIOpin("OAH");

    output_pins[8]  = SpareParts.RegisterIOpin("CC1");
    output_pins[9]  = SpareParts.RegisterIOpin("CC2");
    output_pins[10] = SpareParts.RegisterIOpin("CC3");
    output_pins[11] = SpareParts.RegisterIOpin("CC4");
    output_pins[12] = SpareParts.RegisterIOpin("CC5");
    output_pins[13] = SpareParts.RegisterIOpin("CC6");
    output_pins[14] = SpareParts.RegisterIOpin("CC7");
    output_pins[15] = SpareParts.RegisterIOpin("CC8");
    output_pins[16] = SpareParts.RegisterIOpin("CC9");
    output_pins[17] = SpareParts.RegisterIOpin("CC10");
    output_pins[18] = SpareParts.RegisterIOpin("CC11");
    output_pins[19] = SpareParts.RegisterIOpin("CC12");
    output_pins[20] = SpareParts.RegisterIOpin("CC13");
    output_pins[21] = SpareParts.RegisterIOpin("CC14");
    output_pins[22] = SpareParts.RegisterIOpin("CC15");

    SetPCWProperties(pcwprop);

    PinCount = 10;
    Pins = input_pins;
}

cpart_HP_DisplayLatch::~cpart_HP_DisplayLatch(void)
{

    for (int i = 0; i < 23 /*sizeof(output_pins)*/; i++)
        SpareParts.UnregisterIOpin(output_pins[i]);

    SpareParts.SetPartOnDraw(id);
    SpareParts.CanvasCmd({.cmd = CC_FREEBITMAP, .FreeBitmap{BitmapId}});
    SpareParts.CanvasCmd({.cmd = CC_DESTROY});
}

void cpart_HP_DisplayLatch::DrawOutput(const unsigned int i)
{
    switch (output[i].id)
    {
    case O_IC:
        SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{26, 26, 26}});
        SpareParts.CanvasCmd(
            {.cmd = CC_RECTANGLE,
             .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
        SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{255, 255, 255}});
        SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"HP DispLatch", output[i].x1 + 16, output[i].y2 - 15, 0.0}});
        break;
    default:
        SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{49, 61, 99}});
        SpareParts.CanvasCmd(
            {.cmd = CC_RECTANGLE,
             .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
        SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{255, 255, 255}});
        SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{pin_names[output[i].id - I_AA], output[i].x1, output[i].y2, 90.0}});

        if (i < O_AA) // input
        {
            if (input_pins[i] == 0)
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"NC", output[i].x1, output[i].y2 - 30, 90.0}});
            else
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{SpareParts.GetPinName(input_pins[i]).c_str(), output[i].x1, output[i].y2 - 30, 90.0}});
        } else { // output
            if (output_pins[i - O_AA] == 0)
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"NC", output[i].x1, output[i].y2 - 30, 90.0}});
            else
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{
                    SpareParts.GetPinName(output_pins[i - O_AA]).c_str() /* + lxT (" ") + SpareParts.GetPinName (output_pins[pinv - 4])*/,
                    output[i].x1, output[i].y2 - 30, 90.0}});
        }
        break;
    }
}

unsigned short cpart_HP_DisplayLatch::GetInputId(char* name)
{
    // printf("Error input '%s' don't have a valid id! \n", name);
    return INVALID_ID;
};

unsigned short cpart_HP_DisplayLatch::GetOutputId(char* name)
{
    if (strcmp(name, "IC_20") == 0) // IC rendering
        return O_IC;

    if (strcmp(name, "PN_1") == 0)
        return I_AA;
    if (strcmp(name, "PN_2") == 0)
        return I_AB;
    if (strcmp(name, "PN_3") == 0)
        return I_AC;
    if (strcmp(name, "PN_4") == 0)
        return I_AD;
    if (strcmp(name, "PN_5") == 0)
        return I_AE;
    if (strcmp(name, "PN_6") == 0)
        return I_AF;
    if (strcmp(name, "PN_7") == 0)
        return I_AH;
    if (strcmp(name, "PN_8") == 0)
        return I_AG;
    if (strcmp(name, "PN_9") == 0)
        return I_STR;
    if (strcmp(name, "PN_10") == 0)
        return I_RCD;

    if (strcmp(name, "PN_11") == 0)
        return O_AA;
    if (strcmp(name, "PN_12") == 0)
        return O_AB;
    if (strcmp(name, "PN_13") == 0)
        return O_AC;
    if (strcmp(name, "PN_14") == 0)
        return O_AD;
    if (strcmp(name, "PN_15") == 0)
        return O_AE;
    if (strcmp(name, "PN_16") == 0)
        return O_AF;
    if (strcmp(name, "PN_17") == 0)
        return O_AG;
    if (strcmp(name, "PN_18") == 0)
        return O_AH;

    if (strcmp(name, "PN_19") == 0)
        return O_CC11;
    if (strcmp(name, "PN_20") == 0)
        return O_CC12;

    //printf("Error output '%s' don't have a valid id! \n", name);
    return INVALID_ID;
};

std::string cpart_HP_DisplayLatch::WritePreferences(void)
{
    char prefs[256];

    sprintf(prefs, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",
            input_pins[0], input_pins[1], input_pins[2], input_pins[3],
            input_pins[4], input_pins[5], input_pins[6], input_pins[7],
            input_pins[8], input_pins[9], output_pins[0]);

    return prefs;
}

void cpart_HP_DisplayLatch::ReadPreferences(std::string value)
{
    unsigned char outp;
    sscanf(value.c_str(), "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",
           &input_pins[0], &input_pins[1], &input_pins[2], &input_pins[3],
           &input_pins[4], &input_pins[5], &input_pins[6], &input_pins[7],
           &input_pins[8], &input_pins[9], &outp);

    // skip if already configured - saves time  FIXME?
    if (output_pins[0] != outp)
    {
        for (int i = 0; i < 23 /*sizeof(output_pins)*/; i++)
            SpareParts.UnregisterIOpin(output_pins[i]);

        output_pins[0] = SpareParts.RegisterIOpin("OAA");
        output_pins[1] = SpareParts.RegisterIOpin("OAB");
        output_pins[2] = SpareParts.RegisterIOpin("OAC");
        output_pins[3] = SpareParts.RegisterIOpin("OAD");
        output_pins[4] = SpareParts.RegisterIOpin("OAE");
        output_pins[5] = SpareParts.RegisterIOpin("OAF");
        output_pins[6] = SpareParts.RegisterIOpin("OAG");
        output_pins[7] = SpareParts.RegisterIOpin("OAH");

        output_pins[8]  = SpareParts.RegisterIOpin("CC1");
        output_pins[9]  = SpareParts.RegisterIOpin("CC2");
        output_pins[10] = SpareParts.RegisterIOpin("CC3");
        output_pins[11] = SpareParts.RegisterIOpin("CC4");
        output_pins[12] = SpareParts.RegisterIOpin("CC5");
        output_pins[13] = SpareParts.RegisterIOpin("CC6");
        output_pins[14] = SpareParts.RegisterIOpin("CC7");
        output_pins[15] = SpareParts.RegisterIOpin("CC8");
        output_pins[16] = SpareParts.RegisterIOpin("CC9");
        output_pins[17] = SpareParts.RegisterIOpin("CC10");
        output_pins[18] = SpareParts.RegisterIOpin("CC11");
        output_pins[19] = SpareParts.RegisterIOpin("CC12");
        output_pins[20] = SpareParts.RegisterIOpin("CC13");
        output_pins[21] = SpareParts.RegisterIOpin("CC14");
        output_pins[22] = SpareParts.RegisterIOpin("CC15");
    }

    Reset();
}

void cpart_HP_DisplayLatch::ConfigurePropertiesWindow(void)
{
    std::string spin;

    // i/o pins
    for (int i = 0; i < 33; i++)
    {
        std::string value = "";

        // set the pin name
        SpareParts.WPropCmd(("label" + std::to_string(i + 1)).c_str(), PWA_LABELSETTEXT,
                            (std::to_string(i + 1) + "-" + pin_names[i]).c_str());

#if 0 // needed?
        // CLabel* label = (CLabel*)WProp->GetChildByName("label_" + itoa(i + 1));
        // if (label) {
        if (true) {  // FIXME
            std::string value = "";
            if (i >= O_AA) // output
            {
                if (output_pins[i - O_AA] == 0)
                    value = "NC";
                else
                    value = std::to_string(output_pins[i - O_AA]);  // + lxT (" ") + SpareParts.GetPinName (output_pins[pinv - 4]);
            }
            SpareParts.WPropCmd(("label_" + std::to_string(i + 1)).c_str(), PWA_LABELSETTEXT, value.c_str());
        }
#endif
    }

    SetPCWComboWithPinNames("combo1", input_pins[0]);  // 1-AA
    SetPCWComboWithPinNames("combo2", input_pins[1]);  // 2-AB
    SetPCWComboWithPinNames("combo3", input_pins[2]);  // 3-AC
    SetPCWComboWithPinNames("combo4", input_pins[3]);  // 4-AD
    SetPCWComboWithPinNames("combo5", input_pins[4]);  // 5-AE
    SetPCWComboWithPinNames("combo6", input_pins[5]);  // 6-AF
    SetPCWComboWithPinNames("combo7", input_pins[6]);  // 7-AG
    SetPCWComboWithPinNames("combo8", input_pins[7]);  // 8-AH
    SetPCWComboWithPinNames("combo9", input_pins[8]);  // 9-STR
    SetPCWComboWithPinNames("combo10", input_pins[9]); // 10-RCD
}

void cpart_HP_DisplayLatch::ReadPropertiesWindow(void)
{
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
}

void cpart_HP_DisplayLatch::PreProcess(void)
{
    SetAlwaysUpdate(1);
}

//                                                                  0    1    2    3     4    =
const int test_data[15] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0xFc,0x60,0xda,0xf2,0x66,0x92,0};

void cpart_HP_DisplayLatch::Process(void)
{
    const picpin* ppins = SpareParts.GetPinsValues();

    auto str = ppins[input_pins[8] - 1].value;
    auto rcd = ppins[input_pins[9] - 1].value;

    // STRobe raising edge
    if (str == 1 && prev_strobe == 0)
    {
        // output latch and strobe the current column
        if (rcd == 0)
        {
            next_column = 0;            // start over at next strobe
        }

        // disable the active column
        SpareParts.WritePin(output_pins[8 + active_column], 1);
        //SpareParts.SetPin(output_pins[8 + active_column], 1);

        if (next_column == 3)  // exponent sign?
        {
            mantissa_sign = (anode_latch & 0x08) != 0;  // save mantissa sign
            anode_latch &= 0x02;  // keep only exp sign
        }
        else if (next_column == 0)  // mantissa sign? shown last
        {
            anode_latch = mantissa_sign ? 0x02 : 0;
        }

        SpareParts.WritePin(output_pins[0], (anode_latch & 0x80) != 0); // AA
        SpareParts.WritePin(output_pins[1], (anode_latch & 0x40) != 0); // AB
        SpareParts.WritePin(output_pins[2], (anode_latch & 0x20) != 0); // AC
        SpareParts.WritePin(output_pins[3], (anode_latch & 0x10) != 0); // AD
        SpareParts.WritePin(output_pins[4], (anode_latch & 0x08) != 0); // AE
        SpareParts.WritePin(output_pins[5], (anode_latch & 0x04) != 0); // AF
        SpareParts.WritePin(output_pins[6], (anode_latch & 0x02) != 0); // AG
        SpareParts.WritePin(output_pins[7], (anode_latch & 0x01) != 0); // AH

        // activate the next column strobe
        active_column = next_column;
        SpareParts.WritePin(output_pins[8 + active_column], 0);
        //SpareParts.SetPin(output_pins[8 + active_column], 0);
        next_column = (next_column + 1) % 15;

        anode_latch = 0;   // ready for new anode values
        prev_strobe = str;
    }

    // STRobe falling edge
    else if (str == 0 && prev_strobe == 1)
    {
        prev_strobe = str;
    }

#if 0 // NO NEED
    // RCD falling edge
    else if (rcd == 0 && prev_rcd == 1)
    {
        //MOVED next_column = 0;            // start over at next strobe
        prev_rcd = rcd;
    }
    // RCD raising edge - No-op
    else if (rcd == 1 && prev_rcd == 0)
    {
        prev_rcd = rcd;
    }
    // latch anode pins when 0 when no strobe
    else if (str == 0)
#endif
    {
        if (ppins[input_pins[0] - 1].value == 0)
        {
            anode_latch |= 0x80;
        }
        if (ppins[input_pins[1] - 1].value == 0)
        {
            anode_latch |= 0x40;
        }
        if (ppins[input_pins[2] - 1].value == 0)
        {
            anode_latch |= 0x20;
        }
        if (ppins[input_pins[3] - 1].value == 0)
        {
            anode_latch |= 0x10;
        }
        if (ppins[input_pins[4] - 1].value == 0)
        {
            anode_latch |= 0x08;
        }
        if (ppins[input_pins[5] - 1].value == 0)
        {
            anode_latch |= 0x04;
        }
        if (ppins[input_pins[6] - 1].value == 0)
        {
            anode_latch |= 0x02;
        }
        if (ppins[input_pins[7] - 1].value == 0)
        {
            anode_latch |= 0x01;
        }
    }
}

void cpart_HP_DisplayLatch::PostProcess(void) {
}

part_init(PART_HP_DISPLAYLATCH_Name, cpart_HP_DisplayLatch, "Other");
