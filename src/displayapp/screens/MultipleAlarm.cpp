/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "displayapp/screens/MultipleAlarm.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/settings/Settings.h"
#include "components/alarm/MultipleAlarmController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::MultipleAlarmController;

namespace {
  void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<MultipleAlarm*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }
}

static void StopAlarmTaskCallback(lv_task_t* task) {
  auto* screen = static_cast<MultipleAlarm*>(task->user_data);
  screen->StopAlerting();
}

MultipleAlarm::MultipleAlarm(Controllers::MultipleAlarmController& multipleAlarmController,
                             Controllers::Settings::ClockType clockType,
                             System::SystemTask& systemTask,
                             Controllers::MotorController& motorController)
  : multipleAlarmController {multipleAlarmController}, 
    wakeLock(systemTask), 
    motorController {motorController},
    clockType {clockType} {

  CreateAlarmList();

  // Create stop button (hidden initially)
  btnStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnEventHandler);
  lv_obj_set_size(btnStop, 240, 70);
  lv_obj_align(btnStop, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStop, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  txtStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_static(txtStop, Symbols::stop);
  lv_obj_set_hidden(btnStop, true);

  if (multipleAlarmController.IsAlerting()) {
    SetAlerting();
  }
}

MultipleAlarm::~MultipleAlarm() {
  if (multipleAlarmController.IsAlerting()) {
    StopAlerting();
  }
  lv_obj_clean(lv_scr_act());
}

void MultipleAlarm::CreateAlarmList() {
  alarmList = lv_list_create(lv_scr_act(), nullptr);
  lv_obj_set_size(alarmList, 240, 200);
  lv_obj_align(alarmList, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(alarmList, LV_LIST_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);

  for (uint8_t i = 0; i < maxAlarms; i++) {
    // Create list button for each alarm
    alarmItems[i] = lv_list_add_btn(alarmList, nullptr, "");
    alarmItems[i]->user_data = this;
    lv_obj_set_event_cb(alarmItems[i], btnEventHandler);
    lv_obj_set_style_local_bg_color(alarmItems[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_bg_color(alarmItems[i], LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_GRAY);

    // Create switch for enabling/disabling alarm
    alarmSwitches[i] = lv_switch_create(alarmItems[i], nullptr);
    lv_obj_set_size(alarmSwitches[i], 60, 30);
    lv_obj_align(alarmSwitches[i], alarmItems[i], LV_ALIGN_IN_RIGHT_MID, -10, 0);
    lv_obj_set_event_cb(alarmSwitches[i], btnEventHandler);
    alarmSwitches[i]->user_data = this;

    // Create label for alarm name
    alarmLabels[i] = lv_label_create(alarmItems[i], nullptr);
    lv_obj_set_style_local_text_font(alarmLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
    lv_label_set_text_fmt(alarmLabels[i], "%s", multipleAlarmController.GetAlarm(i).name.c_str());
    lv_obj_align(alarmLabels[i], alarmItems[i], LV_ALIGN_IN_LEFT_MID, 10, -15);

    // Create time label
    timeLabels[i] = lv_label_create(alarmItems[i], nullptr);
    lv_obj_set_style_local_text_font(timeLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_20);
    auto alarm = multipleAlarmController.GetAlarm(i);
    lv_label_set_text_fmt(timeLabels[i], "%s", FormatTime(alarm.hours, alarm.minutes).c_str());
    lv_obj_align(timeLabels[i], alarmItems[i], LV_ALIGN_IN_LEFT_MID, 10, 5);

    // Create recurrence label
    recurrenceLabels[i] = lv_label_create(alarmItems[i], nullptr);
    lv_obj_set_style_local_text_font(recurrenceLabels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_12);
    lv_label_set_text_fmt(recurrenceLabels[i], "%s", GetRecurrenceText(alarm.recurrence).c_str());
    lv_obj_align(recurrenceLabels[i], alarmItems[i], LV_ALIGN_IN_LEFT_MID, 10, 20);

    UpdateAlarmItem(i);
  }
}

void MultipleAlarm::UpdateAlarmItem(uint8_t index) {
  if (index >= maxAlarms) return;
  
  auto alarm = multipleAlarmController.GetAlarm(index);
  
  // Update switch state
  if (alarm.isEnabled) {
    lv_switch_on(alarmSwitches[index], LV_ANIM_OFF);
  } else {
    lv_switch_off(alarmSwitches[index], LV_ANIM_OFF);
  }
  
  // Update time label
  lv_label_set_text_fmt(timeLabels[index], "%s", FormatTime(alarm.hours, alarm.minutes).c_str());
  
  // Update recurrence label
  lv_label_set_text_fmt(recurrenceLabels[index], "%s", GetRecurrenceText(alarm.recurrence).c_str());
}

void MultipleAlarm::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnStop) {
      StopAlerting();
      return;
    }
    
    // Check if it's an alarm item button
    for (uint8_t i = 0; i < maxAlarms; i++) {
      if (obj == alarmItems[i]) {
        ToggleAlarm(i);
        return;
      }
      if (obj == alarmSwitches[i]) {
        ToggleAlarm(i);
        return;
      }
    }
  }
}

bool MultipleAlarm::OnButtonPushed() {
  if (multipleAlarmController.IsAlerting()) {
    StopAlerting();
    return true;
  }
  return false;
}

bool MultipleAlarm::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // Don't allow closing the screen by swiping while the alarm is alerting
  return multipleAlarmController.IsAlerting() && event == TouchEvents::SwipeDown;
}

void MultipleAlarm::SetAlerting() {
  lv_obj_set_hidden(alarmList, true);
  lv_obj_set_hidden(btnStop, false);
  taskStopAlarm = lv_task_create(StopAlarmTaskCallback, pdMS_TO_TICKS(60 * 1000), LV_TASK_PRIO_MID, this);
  motorController.StartRinging();
  wakeLock.Lock();
}

void MultipleAlarm::StopAlerting() {
  multipleAlarmController.StopAlerting();
  motorController.StopRinging();
  if (taskStopAlarm != nullptr) {
    lv_task_del(taskStopAlarm);
    taskStopAlarm = nullptr;
  }
  wakeLock.Release();
  lv_obj_set_hidden(btnStop, true);
  lv_obj_set_hidden(alarmList, false);
}

void MultipleAlarm::ToggleAlarm(uint8_t index) {
  if (index >= maxAlarms) return;
  
  multipleAlarmController.ToggleAlarm(index);
  UpdateAlarmItem(index);
}

std::string MultipleAlarm::GetRecurrenceText(Controllers::MultipleAlarmController::HardcodedAlarm::RecurType recurrence) const {
  switch (recurrence) {
    case Controllers::MultipleAlarmController::HardcodedAlarm::RecurType::None:
      return "Once";
    case Controllers::MultipleAlarmController::HardcodedAlarm::RecurType::Daily:
      return "Daily";
    case Controllers::MultipleAlarmController::HardcodedAlarm::RecurType::Weekdays:
      return "Mon-Fri";
  }
  return "Unknown";
}

std::string MultipleAlarm::FormatTime(uint8_t hours, uint8_t minutes) const {
  if (clockType == Controllers::Settings::ClockType::H12) {
    bool isPM = hours >= 12;
    uint8_t displayHours = hours;
    if (hours == 0) {
      displayHours = 12;
    } else if (hours > 12) {
      displayHours = hours - 12;
    }
    return std::to_string(displayHours) + ":" + 
           (minutes < 10 ? "0" : "") + std::to_string(minutes) + 
           (isPM ? " PM" : " AM");
  } else {
    return std::to_string(hours) + ":" + 
           (minutes < 10 ? "0" : "") + std::to_string(minutes);
  }
}
