#include "displayapp/screens/ScheduledReminders.h"
#include "displayapp/DisplayApp.h"
#include "components/scheduledreminders/ScheduledRemindersController.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::ScheduledRemindersController;

ScheduledReminders::ScheduledReminders(ScheduledRemindersController& scheduledRemindersController,
                                       System::SystemTask& systemTask,
                                       Controllers::MotorController& motorController)
  : scheduledRemindersController {scheduledRemindersController}, wakeLock(systemTask), motorController {motorController} {
  
  // Create container
  container = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(container, 200, 180);
  lv_obj_align(container, lv_scr_act(), LV_ALIGN_CENTER, 0, 10);
  
  // Create title label
  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Hearing Reminders");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);
  
  // Create next reminder container
  nextReminderContainer = lv_obj_create(container, nullptr);
  lv_obj_set_style_local_bg_color(nextReminderContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(nextReminderContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_border_color(nextReminderContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(nextReminderContainer, 180, 120);
  lv_obj_align(nextReminderContainer, container, LV_ALIGN_CENTER, 0, 10);
  
  // Create next reminder name label
  nextReminderName = lv_label_create(nextReminderContainer, nullptr);
  lv_label_set_text_static(nextReminderName, "Next Reminder");
  lv_label_set_long_mode(nextReminderName, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(nextReminderName, 160);
  lv_label_set_align(nextReminderName, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(nextReminderName, nextReminderContainer, LV_ALIGN_IN_TOP_MID, 0, 30);
  
  // Create next reminder time label
  nextReminderTime = lv_label_create(nextReminderContainer, nullptr);
  lv_label_set_align(nextReminderTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(nextReminderTime, nextReminderContainer, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
  
  // Create alerting container
  alertingContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_border_color(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_size(alertingContainer, 220, 200);
  lv_obj_align(alertingContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  
  // Create alerting message label
  alertingMessage = lv_label_create(alertingContainer, nullptr);
  lv_label_set_long_mode(alertingMessage, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(alertingMessage, 200);
  lv_label_set_align(alertingMessage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(alertingMessage, alertingContainer, LV_ALIGN_IN_TOP_MID, 0, 10);
  
  // Update displays and set initial visibility
  if (HasAlertingReminder()) {
    UpdateAlertingReminderDisplay();
    SetAlerting();
  } else {
    UpdateNextReminderDisplay();
    lv_obj_set_hidden(alertingContainer, true);
    lv_obj_set_hidden(title, false);
    lv_obj_set_hidden(nextReminderContainer, false);
  }
}

ScheduledReminders::~ScheduledReminders() {
  if (HasAlertingReminder()) {
    motorController.StopRinging();
    wakeLock.Release();
  }
  lv_obj_clean(lv_scr_act());
}

void ScheduledReminders::UpdateNextReminderDisplay() {
  // Find the next reminder
  uint8_t nextReminder = scheduledRemindersController.FindNextReminder();
  
  // Check if there's a valid next reminder
  if (nextReminder >= scheduledRemindersController.GetReminderCount()) {
    // No enabled reminders - hide the container
    lv_obj_set_hidden(nextReminderContainer, true);
    return;
  }
  
  // Show the container and update reminder time label in 12-hour format
  lv_obj_set_hidden(nextReminderContainer, false);
  
  uint8_t hour = scheduledRemindersController.GetReminderHours(nextReminder);
  uint8_t minute = scheduledRemindersController.GetReminderMinutes(nextReminder);
  const char* ampmStr = "AM";
  
  // Convert to 12-hour format
  if (hour == 0) {
    hour = 12; // Midnight
  } else if (hour == 12) {
    ampmStr = "PM"; // Noon
  } else if (hour > 12) {
    hour = hour - 12;
    ampmStr = "PM";
  }
  
  lv_label_set_text_fmt(nextReminderTime, "%d:%02d %s", hour, minute, ampmStr);
  lv_label_set_align(nextReminderTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(nextReminderTime, nextReminderContainer, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
}


bool ScheduledReminders::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::Tap:
      if (HasAlertingReminder()) {
        // If there's an alerting reminder, dismiss it and show normal interface
        DismissAlertingReminder();
        return true; // Handled the touch event
      }
      // No action for tap when displaying next reminder
      return false;
    default:
      break;
  }
  return false;
}

bool ScheduledReminders::OnButtonPushed() {
  if (HasAlertingReminder()) {
    // If there's an alerting reminder, dismiss it and show normal interface
    DismissAlertingReminder();
    return true; // Handled the button press
  }
  return false; // Let the system handle as back navigation
}

// Alerting reminder functionality
void ScheduledReminders::SetAlerting() {
  if (!HasAlertingReminder()) {
    return;
  }
  
  UpdateAlertingReminderDisplay();
  
  // Hide normal display, show alerting display
  lv_obj_set_hidden(title, true);
  lv_obj_set_hidden(nextReminderContainer, true);
  lv_obj_set_hidden(alertingContainer, false);
  
  motorController.StartRinging();
  wakeLock.Lock();
}

void ScheduledReminders::UpdateAlertingReminderDisplay() {
  if (!HasAlertingReminder()) {
    return;
  }
  
  uint8_t alertingIndex = GetAlertingReminderIndex();
  lv_label_set_text(alertingMessage, scheduledRemindersController.GetReminderName(alertingIndex));
}

bool ScheduledReminders::HasAlertingReminder() const {
  for (uint8_t i = 0; i < scheduledRemindersController.GetReminderCount(); i++) {
    if (scheduledRemindersController.IsReminderAlerting(i)) {
      return true;
    }
  }
  return false;
}

uint8_t ScheduledReminders::GetAlertingReminderIndex() const {
  for (uint8_t i = 0; i < scheduledRemindersController.GetReminderCount(); i++) {
    if (scheduledRemindersController.IsReminderAlerting(i)) {
      return i;
    }
  }
  return 0; // Should not reach here if HasAlertingReminder() is called first
}


void ScheduledReminders::DismissAlertingReminder() {
  if (!HasAlertingReminder()) {
    return; // No alerting reminder to dismiss
  }
  
  // Get the specific alerting reminder index and stop only that one
  scheduledRemindersController.StopAlertingForReminder(GetAlertingReminderIndex());
  motorController.StopRinging();
  wakeLock.Release();
  
  // Hide alerting display, show normal display
  lv_obj_set_hidden(alertingContainer, true);
  lv_obj_set_hidden(title, false);
  UpdateNextReminderDisplay();
  lv_obj_set_hidden(nextReminderContainer, false);
}