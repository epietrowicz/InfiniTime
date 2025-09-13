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
#include "components/alarm/MultipleAlarmController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MultipleAlarm : public Screen {
      public:
        explicit MultipleAlarm(Controllers::MultipleAlarmController& multipleAlarmController,
                              Controllers::Settings::ClockType clockType,
                              System::SystemTask& systemTask,
                              Controllers::MotorController& motorController);
        ~MultipleAlarm() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void StopAlerting();

      private:
        Controllers::MultipleAlarmController& multipleAlarmController;
        System::WakeLock wakeLock;
        Controllers::MotorController& motorController;
        Controllers::Settings::ClockType clockType;

        lv_obj_t *btnStop, *txtStop;
        lv_obj_t* alarmList = nullptr;
        lv_obj_t* taskStopAlarm = nullptr;
        
        static constexpr uint8_t maxAlarms = 5;
        lv_obj_t* alarmItems[maxAlarms];
        lv_obj_t* alarmSwitches[maxAlarms];
        lv_obj_t* alarmLabels[maxAlarms];
        lv_obj_t* timeLabels[maxAlarms];
        lv_obj_t* recurrenceLabels[maxAlarms];

        void CreateAlarmList();
        void UpdateAlarmItem(uint8_t index);
        void ToggleAlarm(uint8_t index);
        std::string GetRecurrenceText(Controllers::MultipleAlarmController::HardcodedAlarm::RecurType recurrence) const;
        std::string FormatTime(uint8_t hours, uint8_t minutes) const;
      };
    }

    template <>
    struct AppTraits<Apps::MultipleAlarm> {
      static constexpr Apps app = Apps::MultipleAlarm;
      static constexpr const char* icon = Screens::Symbols::bell;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::MultipleAlarm(controllers.multipleAlarmController,
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
