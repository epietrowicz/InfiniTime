#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "components/motor/MotorController.h"
#include "Symbols.h"
#include <array>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ScheduledReminders : public Screen {
      public:
        ScheduledReminders(Controllers::ScheduledRemindersController& scheduledRemindersController, System::SystemTask& systemTask, Controllers::MotorController& motorController);
        ~ScheduledReminders() override;
        
        void SetAlerting();
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        void UpdateNextReminderDisplay();
        void UpdateAlertingReminderDisplay();
        void DismissAlertingReminder();
        uint8_t GetAlertingReminderIndex() const;
        bool HasAlertingReminder() const;
        
        Controllers::ScheduledRemindersController& scheduledRemindersController;
        System::WakeLock wakeLock;
        Controllers::MotorController& motorController;
        
        // UI elements - all created in constructor
        lv_obj_t* container;
        lv_obj_t* title;
        lv_obj_t* nextReminderContainer;
        lv_obj_t* nextReminderName;
        lv_obj_t* nextReminderTime;
        lv_obj_t* alertingContainer;
        lv_obj_t* alertingMessage;
        
        static constexpr uint8_t maxReminders = 4;
      };
    }

    template <>
    struct AppTraits<Apps::ScheduledReminders> {
      static constexpr Apps app = Apps::ScheduledReminders;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ScheduledReminders(controllers.scheduledRemindersController,
                                               *controllers.systemTask,
                                               controllers.motorController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}