/* ########################################################################

   PICSimLab - Programmable IC Simulator Laboratory

   ########################################################################

   Copyright (c) : 2021-2023  Luis Claudio Gambôa Lopes <lcgamboa@yahoo.com>

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

#ifndef PART_HP_DISPLAYLATCH_H
#define PART_HP_DISPLAYLATCH_H

//zzz??? #include <lxrad.h>

//zzz #include "../devices/io_74xx573.h"
#include "../lib/part.h"

#define PART_HP_DISPLAYLATCH_Name "HP DisplayLatch"

class cpart_HP_DisplayLatch : public part {
public:
    std::string GetAboutInfo(void) override { return "L.C. Gamboa \n <lcgamboa@yahoo.com>"; };
    cpart_HP_DisplayLatch(const unsigned x, const unsigned y, const char* name, const char* type, board* pboard_, const int id_);
    ~cpart_HP_DisplayLatch(void);
    void DrawOutput(const unsigned int index) override;
    void PreProcess(void) override;
    void Process(void) override;
    void PostProcess(void) override;
    std::string GetPictureFileName(void) override { return "../Common/IC20.svg"; };
    std::string GetMapFile(void) override { return "../Common/IC20.map"; };
    void ConfigurePropertiesWindow(void) override;
    void ReadPropertiesWindow() override;
    std::string WritePreferences(void) override;
    void ReadPreferences(std::string value) override;
    unsigned short GetInputId(char* name) override;
    unsigned short GetOutputId(char* name) override;

private:
    unsigned char input_pins[10];
    unsigned char output_pins[23];
    //zzz unsigned long output_pins_alm[23];
    //zzz long mcount;
    //zzz int JUMPSTEPS_;
    unsigned char prev_strobe = 0;
    unsigned char prev_rcd = 1;
    unsigned char anode_latch = 0;
    int active_column = 0;
    int next_column = 0;
    int mantissa_sign = 0;
    //zzz io_74xx573_t lt8;
    //zzz unsigned short _ret;
    //zzz ???? lxFont font;
};

#endif /* HP_DISPLAYLATCH_H */
