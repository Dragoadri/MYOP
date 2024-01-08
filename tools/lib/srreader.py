from openpilot.selfdrive.test.openpilotci import get_url
from openpilot.tools.lib.logreader import LogReader
from openpilot.tools.lib.route import Route, SegmentRange


def parse_start_end(sr: SegmentRange, route=None):
  start = int(sr.start) if sr.start is not None else 0
  end = int(sr.end) if sr.end is not None else None if sr.start is None else start

  if route is None:
    assert start is not None and end is not None, "segment(s) must be provided for non-api sources"

  if end is None:
    end = route.max_seg_number

  return start, end

def comma_api_source(sr: SegmentRange):
  route = Route(sr.route_name)
  start, end = parse_start_end(sr, route)

  log_paths = route.log_paths()

  for seg in range(start, end+1):
    yield LogReader(log_paths[seg])

def internal_source(sr: SegmentRange):
  start, end = parse_start_end(sr)

  for seg in range(start, end+1):
    yield LogReader(f"cd:/{sr.dongle_id}/{sr.timestamp}/{seg}/rlog.bz2")

def openpilotci_source(sr: SegmentRange):
  start, end = parse_start_end(sr)

  for seg in range(start, end+1):
    yield LogReader(get_url(sr.route_name, seg))

class SegmentRangeReader:
  def __init__(self, segment_range: str, source=comma_api_source):
    sr = SegmentRange(segment_range)
    self.lrs = source(sr)

  def __iter__(self):
    for lr in self.lrs:
      for m in lr:
        yield m
