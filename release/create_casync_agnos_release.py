import argparse
import json
import pathlib
import tempfile
from openpilot.system.hardware.tici.agnos import AGNOS_MANIFEST, StreamingDecompressor
from openpilot.system.updated.casync.common import create_casync_from_file



if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="creates a casync release")
  parser.add_argument("target_dir", type=str, help="target directory to build channel from")
  parser.add_argument("output_dir", type=str, help="output directory for the channel")
  parser.add_argument("version", type=str, help="version of agnos this is")
  args = parser.parse_args()

  output_dir = pathlib.Path(args.output_dir)
  manifest_file = pathlib.Path(args.target_dir) / AGNOS_MANIFEST

  with tempfile.NamedTemporaryFile() as entry_file:
    entry_path = pathlib.Path(entry_file.name)

    with open(manifest_file) as f:
      manifest = json.load(f)

    for entry in manifest:
      print(f"creating casync agnos build from {entry}")
      downloader = StreamingDecompressor(entry['url'])

      CHUNK_SIZE=16*1024*1024 # 16mb

      size = entry["size"]

      cur = 0
      with open(entry_path, "wb") as f:
        while True:
          to_read = min((size - cur), CHUNK_SIZE)

          print(f"{cur/size*100:06.2f}% {to_read}")

          if not to_read:
            break

          f.write(downloader.read(to_read))

          cur += to_read

      print(entry_path.stat().st_size, size)
      assert entry_path.stat().st_size == size

      create_casync_from_file(entry_path, output_dir, f"{args.version}-{entry['name']}")
