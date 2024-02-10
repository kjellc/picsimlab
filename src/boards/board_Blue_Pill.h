/* ########################################################################

   PICSimLab - Programmable IC Simulator Laboratory

   ########################################################################

   Copyright (c) : 2015-2024  Luis Claudio Gambôa Lopes <lcgamboa@yahoo.com>

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

#ifndef BOARD_Blue_Pill_H
#define BOARD_Blue_Pill_H

#include "bsim_qemu.h"

#define BOARD_Blue_Pill_Name "Blue Pill"

// new board class must be derived from board class defined in board.h
class cboard_Blue_Pill : public bsim_qemu {
private:
    void RegisterRemoteControl(void) override;

protected:
    const short int* GetPinMap(void) override;

public:
    // Return the board name
    std::string GetName(void) override { return BOARD_Blue_Pill_Name; };
    std::string GetAboutInfo(void) override { return "L.C. Gamboa \n <lcgamboa@yahoo.com>"; };
    // Constructor called once on board creation
    cboard_Blue_Pill(void);
    // Destructor called once on board destruction
    ~cboard_Blue_Pill(void);
    // Called ever 100ms to draw board
    void Draw(void) override;
    void Run_CPU(void) override{};
    void Run_CPU_ns(uint64_t time) override;
    // Return a list of board supported microcontrollers
    std::string GetSupportedDevices(void) override { return "stm32f103c8t6,"; };
    // Reset board status
    void Reset(void) override;
    // Event on the board
    void EvMouseButtonPress(unsigned int button, unsigned int x, unsigned int y, unsigned int state) override;
    // Event on the board
    void EvMouseButtonRelease(unsigned int button, unsigned int x, unsigned int y, unsigned int state) override;
    // Event on the board
    void EvKeyPress(unsigned int key, unsigned int mask) override;
    // Event on the board
    void EvKeyRelease(unsigned int key, unsigned int mask) override;
    // Called ever 1s to refresh status
    void RefreshStatus(void) override;
    // Called to save board preferences in configuration file
    void WritePreferences(void) override;
    // Called whe configuration file load  preferences
    void ReadPreferences(char* name, char* value) override;
    // return the input ids numbers of names used in input map
    unsigned short GetInputId(char* name) override;
    // return the output ids numbers of names used in output map
    unsigned short GetOutputId(char* name) override;
    // board combo events
    void board_Event(const char* controlname) override;
    void MSetAPin(int pin, float value) override;
    std::string MGetPinName(int pin) override;
    int MGetPinCount(void) override;
    void PinsExtraConfig(int cfg) override;
};

#endif /* BOARD_Blue_Pill_H */
