#include "displayapp/screens/HearingAlerts.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>
#include <cmath>
#include <cstdio>
#include <array>
#include <ctime>

using namespace Pinetime::Applications::Screens;

namespace {
  const std::array<const char*, 5> healthQuestions = {"Any pus, blood or fluids from your ear this week?",
                                                      "Has your ear hurt this week?",
                                                      "Been dizzy or spinning this week?",
                                                      "Hearing suddenly changed this week?",
                                                      "Ringing in only one ear this week?"};
}

HearingAlerts::HearingAlerts() : currentAlert(0), inHealthChecklist(false), currentHealthQuestion(0) {

  InitializeAlerts();

  container = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_cont_set_layout(container, LV_LAYOUT_OFF);
  lv_cont_set_fit(container, LV_FIT_NONE);

  // Dark screen background:
  lv_obj_set_style_local_bg_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  // lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

  alertLabel = lv_label_create(container, nullptr);
  lv_obj_set_size(alertLabel, 200, 100);
  lv_obj_align(alertLabel, container, LV_ALIGN_IN_TOP_MID, 0, 20);
  lv_label_set_align(alertLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_long_mode(alertLabel, LV_LABEL_LONG_BREAK);

  timeLabel = lv_label_create(container, nullptr);
  lv_obj_align(timeLabel, alertLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  dismissBtn = lv_btn_create(container, nullptr);
  lv_obj_set_size(dismissBtn, 80, 40);
  lv_obj_align(dismissBtn, container, LV_ALIGN_IN_BOTTOM_LEFT, 20, -20);
  lv_obj_t* dismissLabel = lv_label_create(dismissBtn, nullptr);
  lv_label_set_text(dismissLabel, "Done");
  lv_obj_set_user_data(dismissBtn, this);
  lv_obj_set_event_cb(dismissBtn, DismissEventHandler);

  snoozeBtn = lv_btn_create(container, nullptr);
  lv_obj_set_size(snoozeBtn, 80, 40);
  lv_obj_align(snoozeBtn, container, LV_ALIGN_IN_BOTTOM_RIGHT, -20, -20);
  lv_obj_t* snoozeLabel = lv_label_create(snoozeBtn, nullptr);
  lv_label_set_text(snoozeLabel, "Later");
  lv_obj_set_user_data(snoozeBtn, this);
  lv_obj_set_event_cb(snoozeBtn, SnoozeEventHandler);

  // Health checklist buttons (initially hidden)
  yesBtn = lv_btn_create(container, nullptr);
  lv_obj_set_size(yesBtn, 60, 40);
  lv_obj_align(yesBtn, container, LV_ALIGN_IN_BOTTOM_LEFT, 30, -20);
  lv_obj_t* yesLabel = lv_label_create(yesBtn, nullptr);
  lv_label_set_text(yesLabel, "Yes");
  lv_obj_set_user_data(yesBtn, this);
  lv_obj_set_event_cb(yesBtn, HealthYesHandler);
  lv_obj_set_hidden(yesBtn, true);

  noBtn = lv_btn_create(container, nullptr);
  lv_obj_set_size(noBtn, 60, 40);
  lv_obj_align(noBtn, container, LV_ALIGN_IN_BOTTOM_RIGHT, -30, -20);
  lv_obj_t* noLabel = lv_label_create(noBtn, nullptr);
  lv_label_set_text(noLabel, "No");
  lv_obj_set_user_data(noBtn, this);
  lv_obj_set_event_cb(noBtn, HealthNoHandler);
  lv_obj_set_hidden(noBtn, true);

  CheckPendingAlerts();
}

uint32_t HearingAlerts::GetCurrentTimeSeconds() {
  // Use simple time() function as fallback - this will work for basic testing
  // In a real implementation, you'd access the InfiniTime datetime controller
  return static_cast<uint32_t>(time(nullptr));
}

void HearingAlerts::InitializeAlerts() {
  uint32_t now = GetCurrentTimeSeconds();
  NRF_LOG_INFO("Current time (s): %u", now);

  alerts = {{{AlertType::HEARING_AID_CHECK, "Are you wearing your hearing aid?", 3, 8, 21, false, true, 0},
             {AlertType::WIPE_TUBE, "Wipe down your ear tube.", 24, 21, 22, false, true, 0},
             {AlertType::BRUSH_TUBE, "Use the ear tube brush to clean the tube", 24 * 7, 0, 24, true, true, now},
             {AlertType::REPLACE_BATTERY, "Replace the battery", 24 * 7, 0, 24, true, true, now},
             {AlertType::CLEAN_EAR, "Clean your ear of earwax buildup", 24 * 7, 0, 24, true, true, now},
             {AlertType::EAR_HEALTH, "Weekly ear health check", 24 * 7, 0, 24, true, true, now},
             {AlertType::REPLACE_TUBE, "Replace the ear tube", 24 * 30 * 2, 0, 24, true, true, now}}};
}

void HearingAlerts::CheckPendingAlerts() {
  uint32_t now = GetCurrentTimeSeconds();
  // Get current hour from system time for basic testing
  time_t rawtime = time(nullptr);
  struct tm* timeinfo = localtime(&rawtime);
  uint8_t currentHour = timeinfo->tm_hour;
  uint8_t currentMinute = timeinfo->tm_min;

  for (size_t i = 0; i < alerts.size(); i++) {
    auto& alert = alerts[i];
    if (!alert.isActive)
      continue;

    bool shouldTrigger = false;

    if (alert.type == AlertType::HEARING_AID_CHECK) {
      // Every 3 hours between 8am and 9:30pm
      if (currentHour >= alert.startHour && currentHour <= alert.endHour) {
        uint32_t hoursSinceLastTrigger = (now - alert.lastTriggered) / 3600;
        if (hoursSinceLastTrigger >= alert.intervalHours) {
          shouldTrigger = true;
        }
      }
    } else if (alert.type == AlertType::WIPE_TUBE) {
      // Every night at 9:30pm
      if (currentHour == 21 && currentMinute >= 30) {
        uint32_t daysSinceLast = (now - alert.lastTriggered) / (24 * 3600);
        if (daysSinceLast >= 1) {
          shouldTrigger = true;
        }
      }
    } else if (alert.isWeekly) {
      // Weekly alerts
      uint32_t hoursSinceLast = (now - alert.lastTriggered) / 3600;
      if (hoursSinceLast >= alert.intervalHours) {
        shouldTrigger = true;
      }
    }

    if (shouldTrigger) {
      currentAlert = i;
      ShowAlert(alert);
      return;
    }
  }

  // If no alerts pending, show a status message
  lv_label_set_text(alertLabel, "No pending alerts");
  lv_obj_set_hidden(dismissBtn, false);
  lv_obj_set_hidden(snoozeBtn, true);
  lv_obj_set_hidden(yesBtn, true);
  lv_obj_set_hidden(noBtn, true);
}

void HearingAlerts::ShowAlert(const Alert& alert) {
  lv_label_set_text(alertLabel, alert.message);

  // Get current time for display
  time_t rawtime = time(nullptr);
  struct tm* timeinfo = localtime(&rawtime);
  char timeStr[32];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  lv_label_set_text(timeLabel, timeStr);

  if (alert.type == AlertType::EAR_HEALTH) {
    // Hide normal buttons, show health checklist
    lv_obj_set_hidden(dismissBtn, true);
    lv_obj_set_hidden(snoozeBtn, true);
    ShowHealthChecklist();
  } else {
    // Show normal buttons
    lv_obj_set_hidden(dismissBtn, false);
    lv_obj_set_hidden(snoozeBtn, false);
    lv_obj_set_hidden(yesBtn, true);
    lv_obj_set_hidden(noBtn, true);
    inHealthChecklist = false;
  }
}

void HearingAlerts::ShowHealthChecklist() {
  inHealthChecklist = true;
  currentHealthQuestion = 0;

  lv_label_set_text(alertLabel, healthQuestions[currentHealthQuestion]);
  lv_obj_set_hidden(yesBtn, false);
  lv_obj_set_hidden(noBtn, false);
}

void HearingAlerts::DismissAlert() {
  alerts[currentAlert].lastTriggered = GetCurrentTimeSeconds();
  CheckPendingAlerts();
}

void HearingAlerts::SnoozeAlert() {
  // Snooze for 30 minutes by setting last triggered to 30 mins ago relative to interval
  uint32_t now = GetCurrentTimeSeconds();
  alerts[currentAlert].lastTriggered = now - (alerts[currentAlert].intervalHours * 3600 - 1800); // 30 min snooze
  CheckPendingAlerts();
}

void HearingAlerts::NextHealthQuestion() {
  currentHealthQuestion++;
  if (currentHealthQuestion < healthQuestions.size()) {
    lv_label_set_text(alertLabel, healthQuestions[currentHealthQuestion]);
  } else {
    // End of checklist
    lv_label_set_text(alertLabel, "Health check complete!");
    lv_obj_set_hidden(yesBtn, true);
    lv_obj_set_hidden(noBtn, true);
    lv_obj_set_hidden(dismissBtn, false);
    lv_obj_set_hidden(snoozeBtn, false);
    inHealthChecklist = false;
  }
}

// Event handlers
void HearingAlerts::DismissEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<HearingAlerts*>(lv_obj_get_user_data(obj));
    screen->DismissAlert();
  }
}

void HearingAlerts::SnoozeEventHandler(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<HearingAlerts*>(lv_obj_get_user_data(obj));
    screen->SnoozeAlert();
  }
}

void HearingAlerts::HealthYesHandler(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<HearingAlerts*>(lv_obj_get_user_data(obj));
    // If yes to any health question, show warning
    lv_label_set_text(screen->alertLabel, "If any symptoms persist, please contact your doctor");
    lv_obj_set_hidden(screen->yesBtn, true);
    lv_obj_set_hidden(screen->noBtn, true);
    lv_obj_set_hidden(screen->dismissBtn, false);
    lv_obj_set_hidden(screen->snoozeBtn, false);
    screen->inHealthChecklist = false;
  }
}

void HearingAlerts::HealthNoHandler(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    auto* screen = static_cast<HearingAlerts*>(lv_obj_get_user_data(obj));
    screen->NextHealthQuestion();
  }
}

// Required overrides
void HearingAlerts::Refresh() {
}

bool HearingAlerts::OnButtonPushed() {
  return true;
}

bool HearingAlerts::OnTouchEvent(TouchEvents event) {
  return false;
}

HearingAlerts::~HearingAlerts() {
  lv_obj_clean(lv_scr_act());
}