/* ########################################################################

   PICSimLab - Programmable IC Simulator Laboratory

   ########################################################################

   Copyright (c) : 2019-2024  Luis Claudio Gambôa Lopes <lcgamboa@yahoo.com>

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

#include "input_keypad.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

/* outputs */
enum {
    O_K1,  // Key 1, line 1
    O_K2,  //     2
    O_K3,  //     3
    O_KA,  //     A
    O_K4,  //     4, line 2
    O_K5,  //     5
    O_K6,  //     6
    O_KB,  //     B
    O_K7,  //     7, line 3
    O_K8,  //     8
    O_K9,  //     9
    O_KC,  //     C, line 4
    O_Ka,  //     *
    O_K0,  //     0
    O_KT,  //     ¤
    O_KD,  //     D
    O_L1,
    O_L2,
    O_L3,
    O_L4,
    O_C1,
    O_C2,
    O_C3,
    O_C4,
    O_C5
};

/* inputs */
enum { I_K1, I_K2, I_K3, I_KA, I_K4, I_K5, I_K6, I_KB, I_K7, I_K8, I_K9, I_KC, I_Ka, I_K0, I_KT, I_KD };

/* types */
enum { KT4x4 = 1, KT4x3, KT2x5, KT4x4_NG, KT4x3_NG, KT2x5_NG };

static PCWProp pcwprop[13+24] = {{PCW_COMBO, "P1 -L1"}, // also L1C1a
                              {PCW_COMBO, "P2 - L2"},    // also L1C1b
                              {PCW_COMBO, "P3 - L3"},    // also L1C2a
                              {PCW_COMBO, "P4 - L4"},    // also L1C2b
                              {PCW_COMBO, "P5 - C1"},    // also L1C3a
                              {PCW_COMBO, "P6 - C2"},    // also L1C3b
                              {PCW_COMBO, "P7 - C3"},    // also L1C4a
                              {PCW_COMBO, "P8 - C4"},    // also L1C4b

                              {PCW_LABEL, "P10-VCC,+5V"},
                              {PCW_LABEL, "P11-GND,GND"},
                              {PCW_COMBO, "Pull"},
                              {PCW_COMBO, "Type"},

                              // combo13...combo36
                              {PCW_COMBO, "P12 L2C1a"},
                              {PCW_COMBO, "P13 L2C1b"},
                              {PCW_COMBO, "P14 L2C2a"},
                              {PCW_COMBO, "P15 L2C2b"},
                              {PCW_COMBO, "P16 L2C3a"},
                              {PCW_COMBO, "P17 L2C3b"},
                              {PCW_COMBO, "P18 L2C4a"},
                              {PCW_COMBO, "P19 L2C4b"},
                              {PCW_COMBO, "P20 L3C1a"},
                              {PCW_COMBO, "P21 L3C1b"},
                              {PCW_COMBO, "P22 L3C2a"},
                              {PCW_COMBO, "P23 L3C2b"},
                              {PCW_COMBO, "P24 L3C3a"},
                              {PCW_COMBO, "P25 L3C3b"},
                              {PCW_COMBO, "P26 L3C4a"},
                              {PCW_COMBO, "P27 L3C4b"},
                              {PCW_COMBO, "P28 L4C1a"},
                              {PCW_COMBO, "P29 L4C1b"},
                              {PCW_COMBO, "P30 L4C2a"},
                              {PCW_COMBO, "P31 L4C2b"},
                              {PCW_COMBO, "P32 L4C3a"},
                              {PCW_COMBO, "P33 L4C3b"},
                              {PCW_COMBO, "P34 L4C4a"},
                              {PCW_COMBO, "P35 L4C4b"},

                              {PCW_END, ""}};

std::string cpart_keypad::GetPictureFileName(void) {
    switch (type) {
        case KT4x4:
        case KT4x4_NG:
            return "Keypad/keypad_4x4.svg";
            break;
        case KT4x3:
        case KT4x3_NG:
            return "Keypad/keypad_4x3.svg";
            break;
        case KT2x5:
        case KT2x5_NG:
            return "Keypad/keypad_2x5.svg";
            break;
    }
    return "Keypad/keypad_4x4.svg";
}

std::string cpart_keypad::GetMapFile(void) {
    switch (type) {
        case KT4x4:
        case KT4x4_NG:
            return "Keypad/keypad_4x4.map";
            break;
        case KT4x3:
        case KT4x3_NG:
            return "Keypad/keypad_4x3.map";
            break;
        case KT2x5:
        case KT2x5_NG:
            return "Keypad/keypad_2x5.map";
            break;
    }
    return "Keypad/keypad_4x4.map";
}

cpart_keypad::cpart_keypad(const unsigned x, const unsigned y, const char* name, const char* type, board* pboard_,
                           const int id_)
    : part(x, y, name, type, pboard_, id_, 9) {
    always_update = 1;
    pull = 0;
    type = 0;

    memset(output_pins, 0, 32); // sizeof(output_pins) * sizeof(char));

    memset(keys, 0, 16);
    memset(keys2, 0, 10);

    refresh = 0;

    ChangeType(KT4x4);

    SetPCWProperties(pcwprop);

    PinCount = sizeof(output_pins); // kjc: FIXME: WAIT WITH THIS!
    Pins = output_pins;
}

void cpart_keypad::RegisterRemoteControl(void) {
    if ((type == KT4x3) || (type == KT4x3_NG) || (type == KT4x4) || (type == KT4x4_NG)) {
        input_ids[I_K1]->status = &keys[0][0];
        input_ids[I_K1]->update = &output_ids[O_K1]->update;
        input_ids[I_K2]->status = &keys[0][1];
        input_ids[I_K2]->update = &output_ids[O_K2]->update;
        input_ids[I_K3]->status = &keys[0][2];
        input_ids[I_K3]->update = &output_ids[O_K3]->update;
        input_ids[I_K4]->status = &keys[1][0];
        input_ids[I_K4]->update = &output_ids[O_K4]->update;
        input_ids[I_K5]->status = &keys[1][1];
        input_ids[I_K5]->update = &output_ids[O_K5]->update;
        input_ids[I_K6]->status = &keys[1][2];
        input_ids[I_K6]->update = &output_ids[O_K6]->update;
        input_ids[I_K7]->status = &keys[2][0];
        input_ids[I_K7]->update = &output_ids[O_K7]->update;
        input_ids[I_K8]->status = &keys[2][1];
        input_ids[I_K8]->update = &output_ids[O_K8]->update;
        input_ids[I_K9]->status = &keys[2][2];
        input_ids[I_K9]->update = &output_ids[O_K9]->update;
        input_ids[I_Ka]->status = &keys[3][0];
        input_ids[I_Ka]->update = &output_ids[O_Ka]->update;
        input_ids[I_K0]->status = &keys[3][1];
        input_ids[I_K0]->update = &output_ids[O_K0]->update;
        input_ids[I_KT]->status = &keys[3][2];
        input_ids[I_KT]->update = &output_ids[O_KT]->update;
        input_ids[I_KA]->status = &keys[0][3];
        input_ids[I_KA]->update = &output_ids[O_KA]->update;
        input_ids[I_KB]->status = &keys[1][3];
        input_ids[I_KB]->update = &output_ids[O_KB]->update;
        input_ids[I_KC]->status = &keys[2][3];
        input_ids[I_KC]->update = &output_ids[O_KC]->update;
        input_ids[I_KD]->status = &keys[3][3];
        input_ids[I_KD]->update = &output_ids[O_KD]->update;
    } else if (type == KT2x5 || type == KT2x5_NG) {
        input_ids[I_K1]->status = &keys2[0][0];
        input_ids[I_K1]->update = &output_ids[O_K1]->update;
        input_ids[I_K2]->status = &keys2[0][1];
        input_ids[I_K2]->update = &output_ids[O_K2]->update;
        input_ids[I_K3]->status = &keys2[0][2];
        input_ids[I_K3]->update = &output_ids[O_K3]->update;
        input_ids[I_K4]->status = &keys2[0][3];
        input_ids[I_K4]->update = &output_ids[O_K4]->update;
        input_ids[I_K5]->status = &keys2[0][4];
        input_ids[I_K5]->update = &output_ids[O_K5]->update;
        input_ids[I_K6]->status = &keys2[1][0];
        input_ids[I_K6]->update = &output_ids[O_K6]->update;
        input_ids[I_K7]->status = &keys2[1][1];
        input_ids[I_K7]->update = &output_ids[O_K7]->update;
        input_ids[I_K8]->status = &keys2[1][2];
        input_ids[I_K8]->update = &output_ids[O_K8]->update;
        input_ids[I_K9]->status = &keys2[1][3];
        input_ids[I_K9]->update = &output_ids[O_K9]->update;
        input_ids[I_K0]->status = &keys2[1][4];
        input_ids[I_K0]->update = &output_ids[O_K0]->update;
    }
}

cpart_keypad::~cpart_keypad(void) {
    SpareParts.SetPartOnDraw(id);
    SpareParts.CanvasCmd({.cmd = CC_FREEBITMAP, .FreeBitmap{BitmapId}});
    SpareParts.CanvasCmd({.cmd = CC_DESTROY});
}

void cpart_keypad::ChangeType(unsigned char tp) {
    if (!tp)
        tp = KT4x4;
    if (tp > KT2x5_NG)
        tp = KT4x4;

    // if same
    if (tp == type)
        return;

    if (BitmapId >= 0) {
        SpareParts.SetPartOnDraw(id);
        SpareParts.CanvasCmd({.cmd = CC_FREEBITMAP, .FreeBitmap{BitmapId}});
        BitmapId = -1;
    }

    type = tp;

    ReadMaps();

    LoadPartImage();
}

void cpart_keypad::DrawOutput(const unsigned int i) {
    // draw connections name
    switch (output[i].id) {
        case O_L1:
        case O_L2:
        case O_L3:
        case O_L4:
        case O_C1:
        case O_C2:
        case O_C3:
        case O_C4:
        case O_C5: {
            SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{49, 61, 99}});
            SpareParts.CanvasCmd(
                {.cmd = CC_RECTANGLE,
                 .Rectangle{1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1, output[i].y2 - output[i].y1}});
            SpareParts.CanvasCmd({.cmd = CC_SETFGCOLOR, .SetFgColor{255, 255, 255}});

            int id = output[i].id - O_L1;
            if ((type == KT2x5) && (id > 1)) {
                id -= 2;
            }
            if (output_pins[id] == 0)
                SpareParts.CanvasCmd({.cmd = CC_ROTATEDTEXT, .RotatedText{"NC", output[i].x1, output[i].y2, 90.0}});
            else
                SpareParts.CanvasCmd(
                    {.cmd = CC_ROTATEDTEXT,
                     .RotatedText{SpareParts.GetPinName(output_pins[id]).c_str(), output[i].x1, output[i].y2, 90.0}});
        } break;
        // draw rectangle when key is pressed
        case O_K1 ... O_KD:
            if (output[i].value) {
                SpareParts.CanvasCmd({.cmd = CC_SETLINEWIDTH, .SetLineWidth{4}});
                SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{255, 255, 0}});
            } else {
                SpareParts.CanvasCmd({.cmd = CC_SETLINEWIDTH, .SetLineWidth{6}});
                if ((output[i].id == O_Ka) || (output[i].id == O_KT) || !((output[i].id + 1) % 4)) {
                    SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{190, 46, 37}});
                } else {
                    SpareParts.CanvasCmd({.cmd = CC_SETCOLOR, .SetColor{50, 118, 179}});
                }
            }

            SpareParts.CanvasCmd({.cmd = CC_RECTANGLE,
                                  .Rectangle{0, output[i].x1 + 5, output[i].y1 + 5, output[i].x2 - output[i].x1 - 10,
                                             output[i].y2 - output[i].y1 - 10}});
            SpareParts.CanvasCmd({.cmd = CC_SETLINEWIDTH, .SetLineWidth{1}});
            break;
    }
}

void cpart_keypad::PreProcess(void) {
    // pull up/down
    const bool is_grid = (type <= KT2x5);
    const int num_outputs = is_grid ? 8 : sizeof(output_pins);
    // for (int i = 4; i < 8; i++) // kjc: test, set only columns
    for (int i = 0; i < num_outputs; i += is_grid ? 1 : 2)
       SpareParts.SetPin(output_pins[i], !pull);
    SetAlwaysUpdate(1);  // kjc: TEST
}

void cpart_keypad::Process(void) {
    if (refresh > 10) {
        const picpin* ppins = SpareParts.GetPinsValues();
        refresh = 0;

        // // old code: pull up/down
        // const bool is_grid = (type <= KT2x5);
        // const int num_outputs = is_grid ? 8 : sizeof(output_pins);
        // for (int i = 0; i < num_outputs; i += is_grid ? 1 : 2)
        //     SpareParts.SetPin(output_pins[i], !pull);

        // pull up/down
        switch (type) {
            case KT4x4:
                for (int c = 0; c < 4; c++) {
                    bool activated = false;
                    for (int l = 0; !activated && l < 4; l++) {
                        if (keys[l][c]) {
                            //? SpareParts.SetPin(output_pins[l], ppins[output_pins[4 + c] - 1].value);
                            SpareParts.SetPin(output_pins[4 + c], ppins[output_pins[l] - 1].value);
                            activated = true;
                        }
                    }
                    if (!activated)
                        SpareParts.SetPin(output_pins[4 + c], !pull);
                }
                break;

            case KT4x4_NG:
                for (int l = 0; l < 4; l++) {
                    for (int c = 0; c < 4; c++) {
                        const int oi = c * 2 + l * 8;  // line by line, 0,2,4,6,8...31
                        if (keys[l][c]) {
                            SpareParts.SetPin(output_pins[oi], ppins[output_pins[oi + 1] - 1].value);
                            //? SpareParts.SetPin(output_pins[oi + 1], ppins[output_pins[oi] - 1].value);
                            active_key_index = oi;
                        }
                    }
                }
                break;

            case KT4x3:
                for (int c = 0; c < 3; c++) {
                    for (int l = 0; l < 4; l++) {
                        if (keys[l][c]) {
                            SpareParts.SetPin(output_pins[l], ppins[output_pins[4 + c] - 1].value);
                            SpareParts.SetPin(output_pins[4 + c], ppins[output_pins[l] - 1].value);
                        }
                    }
                }
                break;

            case KT4x3_NG:
                for (int l = 0; l < 4; l++) {
                    for (int c = 0; c < 3; c++) {
                        const int oi = c * 2 + l * 8;  // line by line, 0,2,4,6,8...23
                        if (keys[l][c]) {
                            SpareParts.SetPin(output_pins[oi], ppins[output_pins[oi + 1] - 1].value);
                            //? SpareParts.SetPin(output_pins[oi + 1], ppins[output_pins[oi] - 1].value);
                            active_key_index = oi;
                        }
                    }
                }
                break;

            case KT2x5:
                for (int c = 0; c < 5; c++) {
                    bool activated = false;
                    for (int l = 0; !activated && l < 2; l++) {
                        if (keys2[l][c]) {
                            //? SpareParts.SetPin(output_pins[l], ppins[output_pins[2 + c] - 1].value);
                            SpareParts.SetPin(output_pins[2 + c], ppins[output_pins[l] - 1].value);
                            activated = true;
                        }
                    }
                    if (!activated)
                        SpareParts.SetPin(output_pins[2 + c], !pull);
                }
                break;

            case KT2x5_NG:
                for (int l = 0; l < 2; l++) {
                    for (int c = 0; c < 5; c++) {
                        const int oi = c * 2 + l * 10;  // line by line, 0,2,4,6,8, 10,12,14,16,18
                        if (keys2[l][c]) {
                            SpareParts.SetPin(output_pins[oi], ppins[output_pins[oi + 1] - 1].value);
                            //? SpareParts.SetPin(output_pins[oi + 1], ppins[output_pins[oi] - 1].value);
                            active_key_index = oi;
                        }
                    }
                }
                break;
        }
    }
    refresh++;
}

void cpart_keypad::OnMouseButtonPress(unsigned int inputId, unsigned int button, unsigned int x, unsigned int y,
                                      unsigned int state) {
    switch (inputId) {
        case I_K1:
            keys[0][0] = 1;
            keys2[0][0] = 1;
            output_ids[O_K1]->value = 1;   // key is pressed
            output_ids[O_K1]->update = 1;  // draw update
            break;
        case I_K2:
            keys[0][1] = 1;
            keys2[0][1] = 1;
            output_ids[O_K2]->value = 1;
            output_ids[O_K2]->update = 1;
            break;
        case I_K3:
            keys[0][2] = 1;
            keys2[0][2] = 1;
            output_ids[O_K3]->value = 1;
            output_ids[O_K3]->update = 1;
            break;
        case I_KA:
            keys[0][3] = 1;
            output_ids[O_KA]->value = 1;
            output_ids[O_KA]->update = 1;
            break;
        case I_K4:
            keys[1][0] = 1;
            keys2[0][3] = 1;
            output_ids[O_K4]->value = 1;
            output_ids[O_K4]->update = 1;
            break;
        case I_K5:
            keys[1][1] = 1;
            keys2[0][4] = 1;
            output_ids[O_K5]->value = 1;
            output_ids[O_K5]->update = 1;
            break;
        case I_K6:
            keys[1][2] = 1;
            keys2[1][0] = 1;
            output_ids[O_K6]->value = 1;
            output_ids[O_K6]->update = 1;
            break;
        case I_KB:
            keys[1][3] = 1;
            output_ids[O_KB]->value = 1;
            output_ids[O_KB]->update = 1;
            break;
        case I_K7:
            keys[2][0] = 1;
            keys2[1][1] = 1;
            output_ids[O_K7]->value = 1;
            output_ids[O_K7]->update = 1;
            break;
        case I_K8:
            keys[2][1] = 1;
            keys2[1][2] = 1;
            output_ids[O_K8]->value = 1;
            output_ids[O_K8]->update = 1;
            break;
        case I_K9:
            keys[2][2] = 1;
            keys2[1][3] = 1;
            output_ids[O_K9]->value = 1;
            output_ids[O_K9]->update = 1;
            break;
        case I_KC:
            keys[2][3] = 1;
            output_ids[O_KC]->value = 1;
            output_ids[O_KC]->update = 1;
            break;
        case I_Ka:
            keys[3][0] = 1;
            output_ids[O_Ka]->value = 1;
            output_ids[O_Ka]->update = 1;
            break;
        case I_K0:
            keys[3][1] = 1;
            keys2[1][4] = 1;
            output_ids[O_K0]->value = 1;
            output_ids[O_K0]->update = 1;
            break;
        case I_KT:
            keys[3][2] = 1;
            output_ids[O_KT]->value = 1;
            output_ids[O_KT]->update = 1;
            break;
        case I_KD:
            keys[3][3] = 1;
            output_ids[O_KD]->value = 1;
            output_ids[O_KD]->update = 1;
            break;
    }
}

void cpart_keypad::OnMouseButtonRelease(unsigned int inputId, unsigned int button, unsigned int x, unsigned int y,
                                        unsigned int state) {

    // old code: reset output when key release
    // if (active_key_index >= 0)
    // {
    //     SpareParts.SetPin(output_pins[active_key_index], 1);
    //     active_key_index = -1;
    // }

    // reset output when key release  FIXME: only if NG, use pull?
    switch (inputId) {
        case I_K1:
            keys[0][0] = 0;       // [l][c] i = l * 8 + c * 2
            keys2[0][0] = 0;
            output_ids[O_K1]->value = 0;
            output_ids[O_K1]->update = 1;
            break;
        case I_K2:
            keys[0][1] = 0;
            keys2[0][1] = 0;
            output_ids[O_K2]->value = 0;
            output_ids[O_K2]->update = 1;
            break;
        case I_K3:
            keys[0][2] = 0;
            keys2[0][2] = 0;
            output_ids[O_K3]->value = 0;
            output_ids[O_K3]->update = 1;
            break;
        case I_KA:
            keys[0][3] = 0;
            output_ids[O_KA]->value = 0;
            output_ids[O_KA]->update = 1;
            break;
        case I_K4:
            keys[1][0] = 0;
            keys2[0][3] = 0;
            output_ids[O_K4]->value = 0;
            output_ids[O_K4]->update = 1;
            break;
        case I_K5:
            keys[1][1] = 0;
            keys2[0][4] = 0;
            output_ids[O_K5]->value = 0;
            output_ids[O_K5]->update = 1;
            break;
        case I_K6:
            keys[1][2] = 0;
            keys2[1][0] = 0;
            output_ids[O_K6]->value = 0;
            output_ids[O_K6]->update = 1;
            break;
        case I_KB:
            keys[1][3] = 0;
            output_ids[O_KB]->value = 0;
            output_ids[O_KB]->update = 1;
            break;
        case I_K7:
            keys[2][0] = 0;
            keys2[1][1] = 0;
            output_ids[O_K7]->value = 0;
            output_ids[O_K7]->update = 1;
            break;
        case I_K8:
            keys[2][1] = 0;
            output_ids[O_K8]->value = 0;
            output_ids[O_K8]->update = 1;
            keys2[1][2] = 0;
            break;
        case I_K9:
            keys[2][2] = 0;
            keys2[1][3] = 0;
            output_ids[O_K9]->value = 0;
            output_ids[O_K9]->update = 1;
            break;
        case I_KC:
            keys[2][3] = 0;
            output_ids[O_KC]->value = 0;
            output_ids[O_KC]->update = 1;
            break;
        case I_Ka:
            keys[3][0] = 0;
            output_ids[O_Ka]->value = 0;
            output_ids[O_Ka]->update = 1;
            break;
        case I_K0:
            keys[3][1] = 0;
            keys2[1][4] = 0;
            output_ids[O_K0]->value = 0;
            output_ids[O_K0]->update = 1;
            break;
        case I_KT:
            keys[3][2] = 0;
            output_ids[O_KT]->value = 0;
            output_ids[O_KT]->update = 1;
            break;
        case I_KD:
            keys[3][3] = 0;
            output_ids[O_KD]->value = 0;
            output_ids[O_KD]->update = 1;
            break;
    }
}

unsigned short cpart_keypad::GetInputId(char* name) {
    if (strcmp(name, "KB_1") == 0)
        return I_K1;
    if (strcmp(name, "KB_2") == 0)
        return I_K2;
    if (strcmp(name, "KB_3") == 0)
        return I_K3;
    if (strcmp(name, "KB_A") == 0)
        return I_KA;

    if (strcmp(name, "KB_4") == 0)
        return I_K4;
    if (strcmp(name, "KB_5") == 0)
        return I_K5;
    if (strcmp(name, "KB_6") == 0)
        return I_K6;
    if (strcmp(name, "KB_B") == 0)
        return I_KB;

    if (strcmp(name, "KB_7") == 0)
        return I_K7;
    if (strcmp(name, "KB_8") == 0)
        return I_K8;
    if (strcmp(name, "KB_9") == 0)
        return I_K9;
    if (strcmp(name, "KB_C") == 0)
        return I_KC;

    if (strcmp(name, "KB_a") == 0)
        return I_Ka;
    if (strcmp(name, "KB_0") == 0)
        return I_K0;
    if (strcmp(name, "KB_T") == 0)
        return I_KT;
    if (strcmp(name, "KB_D") == 0)
        return I_KD;

    printf("Error input '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

unsigned short cpart_keypad::GetOutputId(char* name) {
    if (strcmp(name, "PN_L1") == 0)
        return O_L1;
    if (strcmp(name, "PN_L2") == 0)
        return O_L2;
    if (strcmp(name, "PN_L3") == 0)
        return O_L3;
    if (strcmp(name, "PN_L4") == 0)
        return O_L4;
    if (strcmp(name, "PN_C1") == 0)
        return O_C1;
    if (strcmp(name, "PN_C2") == 0)
        return O_C2;
    if (strcmp(name, "PN_C3") == 0)
        return O_C3;
    if (strcmp(name, "PN_C4") == 0)
        return O_C4;
    if (strcmp(name, "PN_C5") == 0)
        return O_C5;

    if (strcmp(name, "KB_1") == 0)
        return O_K1;
    if (strcmp(name, "KB_2") == 0)
        return O_K2;
    if (strcmp(name, "KB_3") == 0)
        return O_K3;
    if (strcmp(name, "KB_A") == 0)
        return O_KA;

    if (strcmp(name, "KB_4") == 0)
        return O_K4;
    if (strcmp(name, "KB_5") == 0)
        return O_K5;
    if (strcmp(name, "KB_6") == 0)
        return O_K6;
    if (strcmp(name, "KB_B") == 0)
        return O_KB;

    if (strcmp(name, "KB_7") == 0)
        return O_K7;
    if (strcmp(name, "KB_8") == 0)
        return O_K8;
    if (strcmp(name, "KB_9") == 0)
        return O_K9;
    if (strcmp(name, "KB_C") == 0)
        return O_KC;

    if (strcmp(name, "KB_a") == 0)
        return O_Ka;
    if (strcmp(name, "KB_0") == 0)
        return O_K0;
    if (strcmp(name, "KB_T") == 0)
        return O_KT;
    if (strcmp(name, "KB_D") == 0)
        return O_KD;

    printf("Error output '%s' don't have a valid id! \n", name);
    return INVALID_ID;
}

std::string cpart_keypad::WritePreferences(void) {
    char prefs[256];

    sprintf(prefs, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",
            output_pins[0], output_pins[1], output_pins[2],
            output_pins[3], output_pins[4], output_pins[5], output_pins[6], output_pins[7], pull, type,
            output_pins[8], output_pins[9], output_pins[10], output_pins[11], output_pins[12], output_pins[13],
            output_pins[14], output_pins[15], output_pins[16], output_pins[17], output_pins[18], output_pins[19],
            output_pins[20], output_pins[21], output_pins[22], output_pins[23], output_pins[24], output_pins[25],
            output_pins[26], output_pins[27], output_pins[28], output_pins[29], output_pins[30], output_pins[31]);

    return prefs;
}

void cpart_keypad::ReadPreferences(std::string value) {
    unsigned char tp;
    sscanf(value.c_str(), "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &output_pins[0], &output_pins[1],
           &output_pins[2], &output_pins[3], &output_pins[4], &output_pins[5], &output_pins[6], &output_pins[7], &pull,
           &tp);
    sscanf(value.c_str(), "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,"
           "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu,%hhu",
           &output_pins[0], &output_pins[1],
           &output_pins[2], &output_pins[3], &output_pins[4], &output_pins[5], &output_pins[6], &output_pins[7], &pull, &tp,
           &output_pins[8], &output_pins[9], &output_pins[10], &output_pins[11], &output_pins[12], &output_pins[13],
           &output_pins[14], &output_pins[15], &output_pins[16], &output_pins[17], &output_pins[18], &output_pins[19],
           &output_pins[20], &output_pins[21], &output_pins[22], &output_pins[23], &output_pins[24], &output_pins[25],
           &output_pins[26], &output_pins[27], &output_pins[28], &output_pins[29], &output_pins[30], &output_pins[31]);

    memset(keys, 0, 16);
    memset(keys2, 0, 10);
    ChangeType(tp);

    // kjc: TEST, set initial pulled value
    const bool is_grid = (type <= KT2x5);
    const int num_outputs = is_grid ? 8 : sizeof(output_pins);
     for (int i = 0; i < num_outputs; i += is_grid ? 1 : 2)
         SpareParts.SetPin(output_pins[i], !pull);
}

void cpart_keypad::ConfigurePropertiesWindow(void) {
    SetPCWComboWithPinNames("combo1", output_pins[0]);
    SetPCWComboWithPinNames("combo2", output_pins[1]);
    SetPCWComboWithPinNames("combo3", output_pins[2]);
    SetPCWComboWithPinNames("combo4", output_pins[3]);
    SetPCWComboWithPinNames("combo5", output_pins[4]);
    SetPCWComboWithPinNames("combo6", output_pins[5]);
    SetPCWComboWithPinNames("combo7", output_pins[6]);
    SetPCWComboWithPinNames("combo8", output_pins[7]);

    // for NoGrid (NG)
    SetPCWComboWithPinNames("combo13", output_pins[8]);
    SetPCWComboWithPinNames("combo14", output_pins[9]);
    SetPCWComboWithPinNames("combo15", output_pins[10]);
    SetPCWComboWithPinNames("combo16", output_pins[11]);
    SetPCWComboWithPinNames("combo17", output_pins[12]);
    SetPCWComboWithPinNames("combo18", output_pins[13]);
    SetPCWComboWithPinNames("combo19", output_pins[14]);
    SetPCWComboWithPinNames("combo20", output_pins[15]);
    SetPCWComboWithPinNames("combo21", output_pins[16]);
    SetPCWComboWithPinNames("combo22", output_pins[17]);
    SetPCWComboWithPinNames("combo23", output_pins[18]);
    SetPCWComboWithPinNames("combo24", output_pins[19]);
    SetPCWComboWithPinNames("combo25", output_pins[20]);
    SetPCWComboWithPinNames("combo26", output_pins[21]);
    SetPCWComboWithPinNames("combo27", output_pins[22]);
    SetPCWComboWithPinNames("combo28", output_pins[23]);
    SetPCWComboWithPinNames("combo29", output_pins[24]);
    SetPCWComboWithPinNames("combo30", output_pins[25]);
    SetPCWComboWithPinNames("combo31", output_pins[26]);
    SetPCWComboWithPinNames("combo32", output_pins[27]);
    SetPCWComboWithPinNames("combo33", output_pins[28]);
    SetPCWComboWithPinNames("combo34", output_pins[29]);
    SetPCWComboWithPinNames("combo35", output_pins[30]);
    SetPCWComboWithPinNames("combo36", output_pins[31]);

    SpareParts.WPropCmd("combo11", PWA_COMBOSETITEMS, "UP,DOWN,");
    if (!pull)
        SpareParts.WPropCmd("combo11", PWA_COMBOSETTEXT, "UP");
    else
        SpareParts.WPropCmd("combo11", PWA_COMBOSETTEXT, "DOWN");

    SpareParts.WPropCmd("combo12", PWA_COMBOSETITEMS, "4x4,4x3,2x5,4x4_NG,4x3_NG,2x5_NG,");
    SpareParts.WPropCmd("combo12", PWA_COMBOPROPEV, "1");

    const bool use_grid = (type == KT4x4 || type == KT4x3 || type == KT2x5);
    if (use_grid)
    {
        SpareParts.WPropCmd("combo13", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo14", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo15", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo16", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo17", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo18", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo19", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo20", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo21", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo22", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo23", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo24", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo25", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo26", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo27", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo28", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo29", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo30", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo31", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo32", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo33", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo34", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo35", PWA_SETENABLE, "0");
        SpareParts.WPropCmd("combo36", PWA_SETENABLE, "0");
    }
    else
    {
        SpareParts.WPropCmd("combo8", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo13", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo14", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo15", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo16", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo17", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo18", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo19", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo20", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo21", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo22", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo23", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo24", PWA_SETENABLE, "1");
        SpareParts.WPropCmd("combo25", PWA_SETENABLE, "1");
    }

    switch (type) {
        case KT4x4:
            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 - L1");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 - L2");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 - L3");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 - L4");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 - C1");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 - C2");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 - C3");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 - C4");
            SpareParts.WPropCmd("combo8", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo12", PWA_COMBOSETTEXT, "4x4");
            break;
        case KT4x3:
            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 - L1");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 - L2");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 - L3");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 - L4");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 - C1");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 - C2");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 - C3");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 - NC");
            SpareParts.WPropCmd("combo8", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo12", PWA_COMBOSETTEXT, "4x3");
            break;
        case KT2x5:
            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 - L1");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 - L2");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 - C1");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 - C2");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 - C3");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 - C4");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 - C5");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 - NC");
            SpareParts.WPropCmd("combo8", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo12", PWA_COMBOSETTEXT, "2x5");
            break;
        case KT4x4_NG:
            SpareParts.WPropCmd("combo12", PWA_COMBOSETTEXT, "4x4_NG");
            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 L1C1a");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 L1C1b");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 L1C2a");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 L1C2b");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 L1C3a");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 L1C3b");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 L1C4a");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 L1C4b");
            SpareParts.WPropCmd("label13", PWA_LABELSETTEXT, "P12 L2C1a");
            SpareParts.WPropCmd("label14", PWA_LABELSETTEXT, "P13 L2C1b");
            SpareParts.WPropCmd("label15", PWA_LABELSETTEXT, "P14 L2C2a");
            SpareParts.WPropCmd("label16", PWA_LABELSETTEXT, "P15 L2C2b");
            SpareParts.WPropCmd("label17", PWA_LABELSETTEXT, "P16 L2C3a");
            SpareParts.WPropCmd("label18", PWA_LABELSETTEXT, "P17 L2C3b");
            SpareParts.WPropCmd("label19", PWA_LABELSETTEXT, "P18 L2C4a");
            SpareParts.WPropCmd("label20", PWA_LABELSETTEXT, "P19 L2C4b");
            SpareParts.WPropCmd("label21", PWA_LABELSETTEXT, "P20 L3C1a");
            SpareParts.WPropCmd("label22", PWA_LABELSETTEXT, "P21 L3C1b");
            SpareParts.WPropCmd("label23", PWA_LABELSETTEXT, "P22 L3C2a");
            SpareParts.WPropCmd("label24", PWA_LABELSETTEXT, "P23 L3C2b");
            SpareParts.WPropCmd("label25", PWA_LABELSETTEXT, "P24 L3C3a");
            SpareParts.WPropCmd("label26", PWA_LABELSETTEXT, "P25 L3C3b");
            SpareParts.WPropCmd("label27", PWA_LABELSETTEXT, "P26 L3C4a");
            SpareParts.WPropCmd("label28", PWA_LABELSETTEXT, "P27 L3C4b");
            SpareParts.WPropCmd("label29", PWA_LABELSETTEXT, "P28 L4C1a");
            SpareParts.WPropCmd("label30", PWA_LABELSETTEXT, "P29 L4C1b");
            SpareParts.WPropCmd("label31", PWA_LABELSETTEXT, "P30 L4C2a");
            SpareParts.WPropCmd("label32", PWA_LABELSETTEXT, "P31 L4C2b");
            SpareParts.WPropCmd("label33", PWA_LABELSETTEXT, "P32 L4C3a");
            SpareParts.WPropCmd("label34", PWA_LABELSETTEXT, "P33 L4C3b");
            SpareParts.WPropCmd("label35", PWA_LABELSETTEXT, "P34 L4C4a");
            SpareParts.WPropCmd("label36", PWA_LABELSETTEXT, "P35 L4C4b");
            //
            SpareParts.WPropCmd("combo25", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo26", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo27", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo28", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo29", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo30", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo31", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo32", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo33", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo34", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo35", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo36", PWA_SETENABLE, "1");
            break;

        case KT4x3_NG:
            SpareParts.WPropCmd("combo12", PWA_LABELSETTEXT, "4x3_NG");

            SpareParts.WPropCmd("label29", PWA_LABELSETTEXT, "P28 L4C1a");
            SpareParts.WPropCmd("label30", PWA_LABELSETTEXT, "P29 L4C1b");
            SpareParts.WPropCmd("label31", PWA_LABELSETTEXT, "P30 L4C2a");
            SpareParts.WPropCmd("label32", PWA_LABELSETTEXT, "P31 L4C2b");
            SpareParts.WPropCmd("label33", PWA_LABELSETTEXT, "P32 L4C3a");
            SpareParts.WPropCmd("label34", PWA_LABELSETTEXT, "P33 L4C3b");
            SpareParts.WPropCmd("label35", PWA_LABELSETTEXT, "P34 L4C4a");
            SpareParts.WPropCmd("label36", PWA_LABELSETTEXT, "P35 L4C4b");

            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 L1C1a");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 L1C1b");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 L1C2a");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 L1C2b");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 L1C3a");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 L1C3b");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 L2C1a");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 L2C1b");

            SpareParts.WPropCmd("label13", PWA_LABELSETTEXT, "P12 L2C2a");
            SpareParts.WPropCmd("label14", PWA_LABELSETTEXT, "P13 L2C2b");
            SpareParts.WPropCmd("label15", PWA_LABELSETTEXT, "P14 L2C3a");
            SpareParts.WPropCmd("label16", PWA_LABELSETTEXT, "P15 L2C3b");
            SpareParts.WPropCmd("label17", PWA_LABELSETTEXT, "P16 L3C1a");
            SpareParts.WPropCmd("label18", PWA_LABELSETTEXT, "P17 L3C1b");
            SpareParts.WPropCmd("label19", PWA_LABELSETTEXT, "P18 L3C2a");
            SpareParts.WPropCmd("label20", PWA_LABELSETTEXT, "P19 L3C2b");
            SpareParts.WPropCmd("label21", PWA_LABELSETTEXT, "P20 L3C3a");
            SpareParts.WPropCmd("label22", PWA_LABELSETTEXT, "P21 L3C3b");
            SpareParts.WPropCmd("label23", PWA_LABELSETTEXT, "P22 L4C1a");
            SpareParts.WPropCmd("label24", PWA_LABELSETTEXT, "P23 L4C1b");
            SpareParts.WPropCmd("label25", PWA_LABELSETTEXT, "P24 L4C2a");
            SpareParts.WPropCmd("label26", PWA_LABELSETTEXT, "P25 L4C2b");
            SpareParts.WPropCmd("label27", PWA_LABELSETTEXT, "P26 L4C3a");
            SpareParts.WPropCmd("label28", PWA_LABELSETTEXT, "P27 L4C3b");
            SpareParts.WPropCmd("label29", PWA_LABELSETTEXT, "P28 NC");
            SpareParts.WPropCmd("label30", PWA_LABELSETTEXT, "P29 NC");
            SpareParts.WPropCmd("label31", PWA_LABELSETTEXT, "P30 NC");
            SpareParts.WPropCmd("label32", PWA_LABELSETTEXT, "P31 NC");
            SpareParts.WPropCmd("label33", PWA_LABELSETTEXT, "P32 NC");
            SpareParts.WPropCmd("label34", PWA_LABELSETTEXT, "P33 NC");
            SpareParts.WPropCmd("label35", PWA_LABELSETTEXT, "P34 NC");
            SpareParts.WPropCmd("label36", PWA_LABELSETTEXT, "P35 NC");
            //
            SpareParts.WPropCmd("combo25", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo26", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo27", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo28", PWA_SETENABLE, "1");
            SpareParts.WPropCmd("combo29", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo30", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo31", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo32", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo33", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo34", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo35", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo36", PWA_SETENABLE, "0");
            break;

        case KT2x5_NG:
            SpareParts.WPropCmd("combo12", PWA_LABELSETTEXT, "2x5_NG");

            SpareParts.WPropCmd("label1", PWA_LABELSETTEXT, "P1 L1C1a");
            SpareParts.WPropCmd("label2", PWA_LABELSETTEXT, "P2 L1C1b");
            SpareParts.WPropCmd("label3", PWA_LABELSETTEXT, "P3 L1C2a");
            SpareParts.WPropCmd("label4", PWA_LABELSETTEXT, "P4 L1C2b");
            SpareParts.WPropCmd("label5", PWA_LABELSETTEXT, "P5 L1C3a");
            SpareParts.WPropCmd("label6", PWA_LABELSETTEXT, "P6 L1C3b");
            SpareParts.WPropCmd("label7", PWA_LABELSETTEXT, "P7 L1C4a");
            SpareParts.WPropCmd("label8", PWA_LABELSETTEXT, "P8 L1C4b");
            SpareParts.WPropCmd("label13", PWA_LABELSETTEXT, "P12 L1C5a");
            SpareParts.WPropCmd("label14", PWA_LABELSETTEXT, "P13 L1C5b");
            SpareParts.WPropCmd("label15", PWA_LABELSETTEXT, "P14 L1C1a");
            SpareParts.WPropCmd("label16", PWA_LABELSETTEXT, "P15 L2C1b");
            SpareParts.WPropCmd("label17", PWA_LABELSETTEXT, "P16 L2C2a");
            SpareParts.WPropCmd("label18", PWA_LABELSETTEXT, "P17 L2C2b");
            SpareParts.WPropCmd("label19", PWA_LABELSETTEXT, "P18 L2C3a");
            SpareParts.WPropCmd("label20", PWA_LABELSETTEXT, "P19 L2C3b");
            SpareParts.WPropCmd("label21", PWA_LABELSETTEXT, "P20 L2C4a");
            SpareParts.WPropCmd("label22", PWA_LABELSETTEXT, "P22 L2C4b");
            SpareParts.WPropCmd("label23", PWA_LABELSETTEXT, "P23 L2C5a");
            SpareParts.WPropCmd("label24", PWA_LABELSETTEXT, "P23 L2C5b");
            SpareParts.WPropCmd("label25", PWA_LABELSETTEXT, "P24 NC");
            SpareParts.WPropCmd("label26", PWA_LABELSETTEXT, "P25 NC");
            SpareParts.WPropCmd("label27", PWA_LABELSETTEXT, "P26 NC");
            SpareParts.WPropCmd("label28", PWA_LABELSETTEXT, "P27 NC");
            SpareParts.WPropCmd("label29", PWA_LABELSETTEXT, "P28 NC");
            SpareParts.WPropCmd("label30", PWA_LABELSETTEXT, "P29 NC");
            SpareParts.WPropCmd("label31", PWA_LABELSETTEXT, "P30 NC");
            SpareParts.WPropCmd("label32", PWA_LABELSETTEXT, "P31 NC");
            SpareParts.WPropCmd("label33", PWA_LABELSETTEXT, "P32 NC");
            SpareParts.WPropCmd("label34", PWA_LABELSETTEXT, "P33 NC");
            SpareParts.WPropCmd("label35", PWA_LABELSETTEXT, "P34 NC");
            SpareParts.WPropCmd("label36", PWA_LABELSETTEXT, "P35 NC");
            //
            SpareParts.WPropCmd("combo25", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo26", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo27", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo28", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo29", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo30", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo31", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo32", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo33", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo34", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo35", PWA_SETENABLE, "0");
            SpareParts.WPropCmd("combo36", PWA_SETENABLE, "0");
            break;
    }
}

void cpart_keypad::ReadPropertiesWindow(void) {
    output_pins[0] = GetPWCComboSelectedPin("combo1");
    output_pins[1] = GetPWCComboSelectedPin("combo2");
    output_pins[2] = GetPWCComboSelectedPin("combo3");
    output_pins[3] = GetPWCComboSelectedPin("combo4");
    output_pins[4] = GetPWCComboSelectedPin("combo5");
    output_pins[5] = GetPWCComboSelectedPin("combo6");
    output_pins[6] = GetPWCComboSelectedPin("combo7");
    output_pins[7] = GetPWCComboSelectedPin("combo8");

    output_pins[8] = GetPWCComboSelectedPin("combo13");
    output_pins[9] = GetPWCComboSelectedPin("combo14");
    output_pins[10] = GetPWCComboSelectedPin("combo15");
    output_pins[11] = GetPWCComboSelectedPin("combo16");
    output_pins[12] = GetPWCComboSelectedPin("combo17");
    output_pins[13] = GetPWCComboSelectedPin("combo18");
    output_pins[14] = GetPWCComboSelectedPin("combo19");
    output_pins[15] = GetPWCComboSelectedPin("combo20");
    output_pins[16] = GetPWCComboSelectedPin("combo21");
    output_pins[17] = GetPWCComboSelectedPin("combo22");
    output_pins[18] = GetPWCComboSelectedPin("combo23");
    output_pins[19] = GetPWCComboSelectedPin("combo24");
    output_pins[20] = GetPWCComboSelectedPin("combo25");
    output_pins[21] = GetPWCComboSelectedPin("combo26");
    output_pins[22] = GetPWCComboSelectedPin("combo27");
    output_pins[23] = GetPWCComboSelectedPin("combo28");
    output_pins[24] = GetPWCComboSelectedPin("combo29");
    output_pins[25] = GetPWCComboSelectedPin("combo30");
    output_pins[26] = GetPWCComboSelectedPin("combo31");
    output_pins[27] = GetPWCComboSelectedPin("combo32");
    output_pins[28] = GetPWCComboSelectedPin("combo33");
    output_pins[29] = GetPWCComboSelectedPin("combo34");
    output_pins[30] = GetPWCComboSelectedPin("combo35");
    output_pins[31] = GetPWCComboSelectedPin("combo36");

    char buff[64];
    SpareParts.WPropCmd("combo11", PWA_COMBOGETTEXT, NULL, buff);

    pull = !(strcmp(buff, "UP") == 0);

    SpareParts.WPropCmd("combo12", PWA_COMBOGETTEXT, NULL, buff);

    ComboChange(NULL, buff);

    memset(keys, 0, 16);
    memset(keys2, 0, 10);
}

void cpart_keypad::ComboChange(const char* controlname, std::string value) {
    if (!value.compare("4x3")) {
        ChangeType(KT4x3);
    } else if (!value.compare("2x5")) {
        ChangeType(KT2x5);
    } else if (!value.compare("4x4_NG")) {
        ChangeType(KT4x4_NG);
    } else if (!value.compare("4x3_NG")) {
        ChangeType(KT4x3_NG);
    } else if (!value.compare("2x5_NG")) {
        ChangeType(KT2x5_NG);
    } else {
        ChangeType(KT4x4);
    }
}

part_init(PART_KEYPAD_Name, cpart_keypad, "Input");
