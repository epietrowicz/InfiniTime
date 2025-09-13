#pragma once

#include <lvgl/lvgl.h>
#include <array>
#include <string>

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class HearingAlerts : public Screen {
      public:
        HearingAlerts();
        ~HearingAlerts() override;

        void Refresh() override;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        enum class AlertType { HEARING_AID_CHECK, WIPE_TUBE, BRUSH_TUBE, REPLACE_BATTERY, CLEAN_EAR, EAR_HEALTH, REPLACE_TUBE };

        struct Alert {
          AlertType type;
          const char* message;
          uint32_t intervalHours;
          uint32_t startHour;
          uint32_t endHour;
          bool isWeekly;
          bool isActive;
          uint32_t lastTriggered;
        };

        std::array<Alert, 7> alerts;
        size_t currentAlert;
        bool inHealthChecklist;
        size_t currentHealthQuestion;

        lv_obj_t* container;
        lv_obj_t* alertLabel;
        lv_obj_t* timeLabel;
        lv_obj_t* dismissBtn;
        lv_obj_t* snoozeBtn;
        lv_obj_t* yesBtn;
        lv_obj_t* noBtn;

        void InitializeAlerts();
        void CheckPendingAlerts();
        void ShowAlert(const Alert& alert);
        void ShowHealthChecklist();
        void DismissAlert();
        void SnoozeAlert();
        void NextHealthQuestion();
        uint32_t GetCurrentTimeSeconds();

        static void DismissEventHandler(lv_obj_t* obj, lv_event_t event);
        static void SnoozeEventHandler(lv_obj_t* obj, lv_event_t event);
        static void HealthYesHandler(lv_obj_t* obj, lv_event_t event);
        static void HealthNoHandler(lv_obj_t* obj, lv_event_t event);
      };
    }

    template <>
    struct AppTraits<Apps::HearingAlerts> {
      static constexpr Apps app = Apps::HearingAlerts;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::HearingAlerts();
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}