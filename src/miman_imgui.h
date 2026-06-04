#pragma once
#ifndef __MIMAN_IMGUI_H_
#define __MIMAN_IMGUI_H_



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
//Custom Headers

#include "miman_coms.h"
#include "miman_config.h"

//Display Modules
#define DISPLAY_WINDWIDTH       1920
#define DISPLAY_WINDHEIGHT      1080
#define DISPLAY_COL1            500
#define DISPLAY_COL2            960
#define DISPLAY_COL2_5          680
#define DISPLAY_COL3            1300
#define MENUPADDING             40
#define DISPLAY_ROW1            355
#define DISPLAY_ROW2            600
#define DISPLAY_ROW3            880

#define DISPLAY_MODEL_W         (DISPLAY_COL3 - DISPLAY_COL1)
#define DISPLAY_MODEL_H         (DISPLAY_ROW2) - MENUPADDING
#define DISPLAY_TRACK_W         (DISPLAY_COL2)
#define DISPLAY_TRACK_H         (DISPLAY_WINDHEIGHT - DISPLAY_ROW3)
#define DISPLAY_FREQ_W          (DISPLAY_COL1)
#define DISPLAY_FREQ_H          (DISPLAY_ROW1) - MENUPADDING
#define DISPLAY_ROTAT_W         (DISPLAY_COL1)
#define DISPLAY_ROTAT_H         (DISPLAY_ROW2 - DISPLAY_ROW1)
#define DISPLAY_CONSOLE_W       (DISPLAY_COL3 - DISPLAY_COL1)
#define DISPLAY_CONSOLE_H       (DISPLAY_ROW1) - MENUPADDING
#define DISPLAY_STATE_W         (DISPLAY_WINDWIDTH - DISPLAY_COL3)
#define DISPLAY_STATE_H         (DISPLAY_WINDHEIGHT) - MENUPADDING
#define DISPLAY_CONTROL_W       (DISPLAY_COL3 - DISPLAY_COL1)
#define DISPLAY_CONTROL_H       (DISPLAY_ROW2 - DISPLAY_ROW1)
#define DISPLAY_COMMAND_W       (DISPLAY_WINDWIDTH - DISPLAY_COL2)
#define DISPLAY_COMMAND_H       (DISPLAY_WINDHEIGHT - 800)

#define MIMAN_FRAMERATE         60
void ImGui_ClearColorBuf(GLFWwindow * window, ImVec4 clear_color);
void ImGui_MainMenu();
void ImGui_ModelWindow(float fontscale);
void ImGui_TrackWindow(float fontscale);
void ImGui_AutoPilotWindow(float fontscale);
void ImGui_FrequencyWindow(float fontscale);
void ImGui_RotatorWindow(float fontscale);
void ImGui_BeaconWindow(float fontscale);
void ImGui_ControlWindow(float fontscale);
void ImGui_CommandWindow(float fontscale);

void ImGui_AutoPilotWindow(float fontscale);

void ImGuiCustomStyle(ImGuiStyle* style);
bool popup_setup(Setup * setup);
bool popup_param_table0();
bool popup_param_table1();
bool popup_param_table5();
void Initialize_CMDLabels();
int CMDDataGenerator(uint32_t msgid, uint16_t fnccode,void *Requested, size_t RequestedSize);
bool popup_cmd();
bool popup_load();
bool popup_tle();
bool popup_fds();
bool popup_satinfo(int index);
bool popup_s_band();

void glfw_error_callback(int error, const char* description);
#endif