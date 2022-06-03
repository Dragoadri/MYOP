#!/usr/bin/env python3
import argparse
import concurrent.futures
import os
import sys
from collections import defaultdict
from tqdm import tqdm
from typing import Any, Dict

from selfdrive.car.car_helpers import interface_names
from selfdrive.test.openpilotci import get_url, upload_file
from selfdrive.test.process_replay.compare_logs import compare_logs, save_log
from selfdrive.test.process_replay.process_replay import CONFIGS, PROC_REPLAY_DIR, FAKEDATA, check_enabled, replay_process
from selfdrive.version import get_commit
from tools.lib.filereader import FileReader
from tools.lib.logreader import LogReader

original_segments = [
  ("BODY", "937ccb7243511b65|2022-05-24--16-03-09--1"),        # COMMA.BODY
  ("HYUNDAI", "02c45f73a2e5c6e9|2021-01-01--19-08-22--1"),     # HYUNDAI.SONATA
  ("TOYOTA", "0982d79ebb0de295|2021-01-04--17-13-21--13"),     # TOYOTA.PRIUS (INDI)
  ("TOYOTA2", "0982d79ebb0de295|2021-01-03--20-03-36--6"),     # TOYOTA.RAV4  (LQR)
  ("TOYOTA3", "f7d7e3538cda1a2a|2021-08-16--08-55-34--6"),     # TOYOTA.COROLLA_TSS2
  ("HONDA", "eb140f119469d9ab|2021-06-12--10-46-24--27"),      # HONDA.CIVIC (NIDEC)
  ("HONDA2", "7d2244f34d1bbcda|2021-06-25--12-25-37--26"),     # HONDA.ACCORD (BOSCH)
  ("CHRYSLER", "4deb27de11bee626|2021-02-20--11-28-55--8"),    # CHRYSLER.PACIFICA
  ("SUBARU", "4d70bc5e608678be|2021-01-15--17-02-04--5"),      # SUBARU.IMPREZA
  ("GM", "0c58b6a25109da2b|2021-02-23--16-35-50--11"),         # GM.VOLT
  ("NISSAN", "35336926920f3571|2021-02-12--18-38-48--46"),     # NISSAN.XTRAIL
  ("VOLKSWAGEN", "de9592456ad7d144|2021-06-29--11-00-15--6"),  # VOLKSWAGEN.GOLF
  ("MAZDA", "bd6a637565e91581|2021-10-30--15-14-53--2"),       # MAZDA.CX9_2021

  # Enable when port is tested and dascamOnly is no longer set
  #("TESLA", "bb50caf5f0945ab1|2021-06-19--17-20-18--3"),      # TESLA.AP2_MODELS
]

segments = [
  ("BODY", "bd6a637565e91581|2022-04-04--22-05-08--0"),
  ("HYUNDAI", "fakedata|2022-01-20--17-49-04--0"),
  ("TOYOTA", "fakedata|2022-04-29--15-57-12--0"),
  ("TOYOTA2", "fakedata|2022-04-29--16-08-01--0"),
  ("TOYOTA3", "fakedata|2022-04-29--16-17-39--0"),
  ("HONDA", "fakedata|2022-01-20--17-56-40--0"),
  ("HONDA2", "fakedata|2022-04-29--16-31-55--0"),
  ("CHRYSLER", "fakedata|2022-01-20--18-00-11--0"),
  ("SUBARU", "fakedata|2022-01-20--18-01-57--0"),
  ("GM", "fakedata|2022-01-20--18-03-41--0"),
  ("NISSAN", "fakedata|2022-01-20--18-05-29--0"),
  ("VOLKSWAGEN", "fakedata|2022-01-20--18-07-15--0"),
  ("MAZDA", "fakedata|2022-01-20--18-09-32--0"),
]

# dashcamOnly makes don't need to be tested until a full port is done
excluded_interfaces = ["mock", "ford", "mazda", "tesla"]

BASE_URL = "https://commadataci.blob.core.windows.net/openpilotci/"
REF_COMMIT_FN = os.path.join(PROC_REPLAY_DIR, "ref_commit")


def run_test_process(data):
  segment, cfg, args, cur_log_fn, ref_log_path, lr_dat = data
  res = None
  if not args.upload_only:
    lr = LogReader.from_bytes(lr_dat)
    res, log_msgs = test_process(cfg, lr, ref_log_path, args.ignore_fields, args.ignore_msgs)
    # save logs so we can upload when updating refs
    save_log(cur_log_fn, log_msgs)

  if args.update_refs or args.upload_only:
    print(f'Uploading: {os.path.basename(cur_log_fn)}')
    assert os.path.exists(cur_log_fn), f"Cannot find log to upload: {cur_log_fn}"
    upload_file(cur_log_fn, os.path.basename(cur_log_fn))
    os.remove(cur_log_fn)
  return (segment, cfg.proc_name, res)


def get_log_data(segment):
  r, n = segment.rsplit("--", 1)
  with FileReader(get_url(r, n)) as f:
    return (segment, f.read())


def test_process(cfg, lr, ref_log_path, ignore_fields=None, ignore_msgs=None):
  if ignore_fields is None:
    ignore_fields = []
  if ignore_msgs is None:
    ignore_msgs = []

  ref_log_msgs = list(LogReader(ref_log_path))

  log_msgs = replay_process(cfg, lr)

  # check to make sure openpilot is engaged in the route
  if cfg.proc_name == "controlsd":
    if not check_enabled(log_msgs):
      segment = os.path.basename(ref_log_path).split("/")[-1].split("_")[0]
      raise Exception(f"Route never enabled: {segment}")

  try:
    return compare_logs(ref_log_msgs, log_msgs, ignore_fields + cfg.ignore, ignore_msgs, cfg.tolerance), log_msgs
  except Exception as e:
    return str(e), log_msgs


def format_diff(results, ref_commit):
  diff1, diff2 = "", ""
  diff2 += f"***** tested against commit {ref_commit} *****\n"

  failed = False
  for segment, result in list(results.items()):
    diff1 += f"***** results for segment {segment} *****\n"
    diff2 += f"***** differences for segment {segment} *****\n"

    for proc, diff in list(result.items()):
      diff1 += f"\t{proc}\n"
      diff2 += f"*** process: {proc} ***\n"

      if isinstance(diff, str):
        diff1 += f"\t\t{diff}\n"
        failed = True
      elif len(diff):
        cnt: Dict[str, int] = {}
        for d in diff:
          diff2 += f"\t{str(d)}\n"

          k = str(d[1])
          cnt[k] = 1 if k not in cnt else cnt[k] + 1

        for k, v in sorted(cnt.items()):
          diff1 += f"\t\t{k}: {v}\n"
        failed = True
  return diff1, diff2, failed


if __name__ == "__main__":
  all_cars = {car for car, _ in segments}
  all_procs = {cfg.proc_name for cfg in CONFIGS}

  parser = argparse.ArgumentParser(description="Regression test to identify changes in a process's output")
  parser.add_argument("--whitelist-procs", type=str, nargs="*", default=all_procs,
                      help="Whitelist given processes from the test (e.g. controlsd)")
  parser.add_argument("--whitelist-cars", type=str, nargs="*", default=all_cars,
                      help="Whitelist given cars from the test (e.g. HONDA)")
  parser.add_argument("--blacklist-procs", type=str, nargs="*", default=[],
                      help="Blacklist given processes from the test (e.g. controlsd)")
  parser.add_argument("--blacklist-cars", type=str, nargs="*", default=[],
                      help="Blacklist given cars from the test (e.g. HONDA)")
  parser.add_argument("--ignore-fields", type=str, nargs="*", default=[],
                      help="Extra fields or msgs to ignore (e.g. carState.events)")
  parser.add_argument("--ignore-msgs", type=str, nargs="*", default=[],
                      help="Msgs to ignore (e.g. carEvents)")
  parser.add_argument("--update-refs", action="store_true",
                      help="Updates reference logs using current commit")
  parser.add_argument("--upload-only", action="store_true",
                      help="Skips testing processes and uploads logs from previous test run")
  parser.add_argument("-j", "--jobs", type=int, default=1)
  args = parser.parse_args()

  tested_procs = set(args.whitelist_procs) - set(args.blacklist_procs)
  tested_cars = set(args.whitelist_cars) - set(args.blacklist_cars)
  tested_cars = {c.upper() for c in tested_cars}

  full_test = (tested_procs == all_procs) and (tested_cars == all_cars) and all(len(x) == 0 for x in (args.ignore_fields, args.ignore_msgs))
  upload = args.update_refs or args.upload_only
  os.makedirs(os.path.dirname(FAKEDATA), exist_ok=True)

  if upload:
    assert full_test, "Need to run full test when updating refs"

  try:
    ref_commit = open(REF_COMMIT_FN).read().strip()
  except FileNotFoundError:
    print("Couldn't find reference commit")
    sys.exit(1)

  cur_commit = get_commit()
  if cur_commit is None:
    raise Exception("Couldn't get current commit")

  print(f"***** testing against commit {ref_commit} *****")

  # check to make sure all car brands are tested
  if full_test:
    untested = (set(interface_names) - set(excluded_interfaces)) - {c.lower() for c in tested_cars}
    assert len(untested) == 0, f"Cars missing routes: {str(untested)}"

  with concurrent.futures.ProcessPoolExecutor(max_workers=args.jobs) as pool:
    if not args.upload_only:
      download_segments = [seg for car, seg in segments if car in tested_cars]
      log_data: Dict[str, LogReader] = {}
      p1 = pool.map(get_log_data, download_segments)
      for segment, lr in tqdm(p1, desc="Getting Logs", total=len(download_segments)):
        log_data[segment] = lr

    pool_args: Any = []
    for car_brand, segment in segments:
      if car_brand not in tested_cars:
        continue

      for cfg in CONFIGS:
        if cfg.proc_name not in tested_procs:
          continue

        cur_log_fn = os.path.join(FAKEDATA, f"{segment}_{cfg.proc_name}_{cur_commit}.bz2")
        if args.update_refs:  # reference logs will not exist if routes were just regenerated
          ref_log_path = get_url(*segment.rsplit("--", 1))
        else:
          ref_log_fn = os.path.join(FAKEDATA, f"{segment}_{cfg.proc_name}_{ref_commit}.bz2")
          ref_log_path = ref_log_fn if os.path.exists(ref_log_fn) else BASE_URL + os.path.basename(ref_log_fn)

        dat = None if args.upload_only else log_data[segment]
        pool_args.append((segment, cfg, args, cur_log_fn, ref_log_path, dat))

    results: Any = defaultdict(dict)
    p2 = pool.map(run_test_process, pool_args)
    for (segment, proc, result) in tqdm(p2, desc="Running Tests", total=len(pool_args)):
      if isinstance(result, list):
        results[segment][proc] = result

  diff1, diff2, failed = format_diff(results, ref_commit)
  if not upload:
    with open(os.path.join(PROC_REPLAY_DIR, "diff.txt"), "w") as f:
      f.write(diff2)
    print(diff1)

    if failed:
      print("TEST FAILED")
      print("\n\nTo push the new reference logs for this commit run:")
      print("./test_processes.py --upload-only")
    else:
      print("TEST SUCCEEDED")

  else:
    with open(REF_COMMIT_FN, "w") as f:
      f.write(cur_commit)
    print(f"\n\nUpdated reference logs for commit: {cur_commit}")

  sys.exit(int(failed))
