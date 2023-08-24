#!/usr/bin/env python3
import argparse
import json
import os

from cereal import car, messaging
from openpilot.common.basedir import BASEDIR
from selfdrive.controls.controlsd import CAMERA_PACKETS, CONTROL_PACKETS
from selfdrive.controls.lib.events import EVENTS, Alert

UI_DIR = os.path.join(BASEDIR, "selfdrive", "ui")
TRANSLATIONS_DIR = os.path.join(UI_DIR, "translations")
LANGUAGES_FILE = os.path.join(TRANSLATIONS_DIR, "languages.json")
TRANSLATIONS_INCLUDE_FILE = os.path.join(TRANSLATIONS_DIR, "alerts_generated.h")

def generate_onroad_alert_translations():
  cp = car.CarParams.new_message()
  cs = car.CarState.new_message()
  sm = messaging.SubMaster(CONTROL_PACKETS + CAMERA_PACKETS)
  translated = set()

  for event in EVENTS.values():
    for alert in event.values():
      if not isinstance(alert, Alert):
        alert = alert(cp, cs, sm, False, 0)
      for text in (alert.alert_text_1, alert.alert_text_2):
        text = text.split('|')[0]
        if (text and text not in translated):
          translated.add(text)

  content = '\n// onroad alerts\n'
  for text in sorted(translated):
    content += f'QT_TRANSLATE_NOOP("OnroadAlerts", R"({text})");\n'
  return content

def generate_offroad_alerts_translations():
  content = '\n// offroad alerts\n'
  with open(os.path.join(BASEDIR, "selfdrive/controls/lib/alerts_offroad.json")) as f:
    for alert in json.load(f).values():
      content += f'QT_TRANSLATE_NOOP("OffroadAlert", R"({alert["text"]})");\n'
  return content

def generate_translations_include():
  content = "// THIS IS AN AUTOGENERATED FILE\n"
  content += generate_offroad_alerts_translations()
  content += generate_onroad_alert_translations()

  with open(TRANSLATIONS_INCLUDE_FILE, "w") as f:
    f.write(content)

def update_translations(vanish=False, plural_only=None, translations_dir=TRANSLATIONS_DIR):
  generate_translations_include()

  if plural_only is None:
    plural_only = []

  with open(LANGUAGES_FILE, "r") as f:
    translation_files = json.load(f)

  for file in translation_files.values():
    tr_file = os.path.join(translations_dir, f"{file}.ts")
    args = f"lupdate -locations none -recursive {UI_DIR} -ts {tr_file} -I {BASEDIR}"
    if vanish:
      args += " -no-obsolete"
    if file in plural_only:
      args += " -pluralonly"
    ret = os.system(args)
    assert ret == 0


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Update translation files for UI",
                                   formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("--vanish", action="store_true", help="Remove translations with source text no longer found")
  parser.add_argument("--plural-only", type=str, nargs="*", default=["main_en"],
                      help="Translation codes to only create plural translations for (ie. the base language)")
  args = parser.parse_args()

  update_translations(args.vanish, args.plural_only)
