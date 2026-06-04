#!/bin/bash
echo "The script must be run in ESP-IDF Terminal - in VS Code it can be launched in"
echo "View -> Command Palette -> ESP-IDF: Open ESP-IDF Terminal (or Ctrl + E T)"

partition_size=$((16 * 1024 * 1024)) # 16MB

parttool.py -b 1000000 read_partition --partition-name=littlefs --output "data-partition.bin"

pip install littlefs-tools
rm -r data-partition 2>/dev/null
block_size=4096
block_count=$(($partition_size / $block_size))
littlefs_extract -b $block_size -c $block_count -d data-partition --image data-partition.bin

python3 decode_binary_data.py data-partition
echo "Decoded data can be found in .TXT files inside data-partition/ subdirectory"
