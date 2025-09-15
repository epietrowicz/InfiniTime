# InfiniTime Fork Requirements

## Codebase

You can use the [InfiniSim](https://github.com/InfiniTimeOrg/InfiniSim) code base to develop the application easily. Here is the build script I have been using:

```bash
#!/bin/bash
set -e  # stop on first error

# Configure the build
cmake -S . -B build \
 -DInfiniTime_DIR=/home/eric/Documents/smartwatch/InfiniTime \ # <- change this path!
 -DMONITOR_ZOOM=3

# Build with 4 parallel jobs
cmake --build build -j4

# Run the program
./build/infinisim
```

Please work off of the [custom-controller branch in my Github](https://github.com/epietrowicz/InfiniTime/tree/custom-controller).

## Controller

Create a new controller similar to the existing [alarm controller](https://github.com/InfiniTimeOrg/InfiniTime/tree/main/src/components/alarm) that schedules hard coded pre-scheduled alarms (see below). [InfiniTime reference documentation](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/doc/code/Intro.md#controllers). [More documentation](https://docs.infinitime.io/en/latest/developer-documentation/index.html).

## User app

All reminders can be turned on or off in the [“Scheduled Reminders” page](https://github.com/epietrowicz/InfiniTime/blob/main/src/displayapp/screens/ScheduledReminders.cpp).  [InfiniTime reference documentation](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/doc/code/Apps.md).

<img width="5569" height="3500" alt="Frame_1171275590" src="https://github.com/user-attachments/assets/bdf64e57-33a8-4ec3-97a8-04190995008b" />

The app’s icon should be headphones:

<img width="5569" height="3500" alt="Frame_1171275592" src="https://github.com/user-attachments/assets/49e56613-fedf-434f-9f5d-0f7c23d28596" />

## Reminder event UI

<img width="5569" height="3500" alt="Frame_1171275591" src="https://github.com/user-attachments/assets/eb1143c8-d0f3-4e0d-911c-3b3721d13c65" />

## Reminders

### **Daily Reminders**

| Time | Reminder |
| --- | --- |
| **9 AM** | Are you wearing hearing aid, taking your daily meds? |
| **6 PM** | Still wearing hearing aid? Before bed, take off, wipe all parts and store safely.. Take your daily meds |

---

### **Weekend Reminders (in addition to daily)**

One on Saturday, one on Sunday. Selected randomly from the list.

| Time | Reminder |
| --- | --- |
| **12 PM** | Wear in different places to get used to the new sounds

Remove battery (if using) before storing hearing aids

Change battery weekly or sooner

Charge hearing aids (if rechargeable)

Clean excess earwax from ear with FDA listed drops

Check for FDA ‘Red Flag’ conditions (add FDA list)and see an ENT

Learn more from ATLAS Q&A, Blogs, Webinars |

---

### **Monthly Reminders**

On the 1st of each month. Selected randomly from the list

| Time | Reminder |
| --- | --- |
| **3 PM** | Wear hearing aids for at least 8 hours/day for full benefit.

Fill your med prescription regularly.

No water, hair/bodysprays, cleaners on hearing aids.

Do not drop hearing aids.

Very hot or windy places? Remove hearing aid.

Noisy place? Remove hearing and wear ear plugs.

Before MRI/CT scan - remove hearing aids.

Clean reusable earplugs with soap and water after each use.

Check and replace - Dirty eartube, eartips, waxguards.

Remind others to face you and to speak clearly.

Be in a well-lit room when talking.

Stuck eartip? Try removing with fingers or go to emergency. No sharp objects in ear.

Ear pain? Change eartip, try a different size. Use otoscope to check. See ENT.

Check your medicine list for drugs that may harm hearing.

Maintain healthy blood pressure, blood sugar levels - they help with hearing.

Reduce or quit smoking, vaping - they harm hearing

Wear in workplace - for improved communication, job performance, safety.

Fall, head injury? Get prompt medical help to check brain injury, worsening hearing.

Doctor visits, hospital stays? Wear hearing aids to not miss any instruction.

 Be physically active to support hearing health 21. Get 6-7 hours of daily sleep to support hearing health.

Stay hydrated with 9 cups of fluid - avoid sugar, fat in drinks.

Support your social and mental health with daily wear of hearing aids.

 Support your cognitive health with daily wear of hearing aids.

Use public assistive technology like hearing loops, with hearing aid telecoil.

Be prepared for emergencies with special alerting devices.

Check how to wear Personal Protective Equipment with hearing aids at work.

Use vibrating/visual alerts to be prepared for emergencies.

Check your vision and wear eyeglasses along with your hearing aids.

Check how your Quality of Life is improving with hearing aids.

Learn more from ATLAS Q&A, Blogs, Webinars. |
