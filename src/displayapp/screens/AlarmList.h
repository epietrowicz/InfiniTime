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
#pragma once

#include "displayapp/apps/Apps.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class AlarmList : public Screen {
      public:
        explicit AlarmList(Controllers::AlarmController& alarmController,
                           Controllers::Settings::ClockType clockType,
                           System::SystemTask& systemTask,
                           Controllers::MotorController& motorController);
        ~AlarmList() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void StopAlerting();

      private:
        Controllers::AlarmController& alarmController;
        System::WakeLock wakeLock;
        Controllers::MotorController& motorController;

        lv_obj_t *btnStop, *txtStop;
        lv_task_t* taskStopAlarm = nullptr;
        lv_obj_t* alarmList = nullptr;
        std::array<lv_obj_t*, Controllers::AlarmController::MaxAlarms> alarmItems;
        std::array<lv_obj_t*, Controllers::AlarmController::MaxAlarms> alarmSwitches;
        std::array<lv_obj_t*, Controllers::AlarmController::MaxAlarms> alarmLabels;

        void CreateAlarmItem(uint8_t alarmIndex);
        void UpdateAlarmItem(uint8_t alarmIndex);
        void UpdateAllAlarmItems();
        void OnAlarmToggle(uint8_t alarmIndex, bool enabled);
        void FormatTimeString(char* buffer, size_t bufferSize, uint8_t hours, uint8_t minutes, Controllers::Settings::ClockType clockType);
        void FormatRecurrenceString(char* buffer, size_t bufferSize, Controllers::AlarmController::RecurType recurrence);
      };
    }

    template <>
    struct AppTraits<Apps::AlarmList> {
      static constexpr Apps app = Apps::AlarmList;
      static constexpr const char* icon = Screens::Symbols::bell;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::AlarmList(controllers.alarmController,
                                      controllers.settingsController.GetClockType(),
                                      *controllers.systemTask,
                                      controllers.motorController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
