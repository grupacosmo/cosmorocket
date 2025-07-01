# Written by Maximilian Gerhardt <maximilian.gerhardt@rub.de>
# 29th December 2020
# and Christian Baars, Johann Obermeier
# 2023 / 2024
# License: Apache
# Expanded from functionality provided by PlatformIO's espressif32 and espressif8266 platforms, credited below.
# This script provides functions to download the filesystem (LittleFS) from a running ESP32 / ESP8266
# over the serial bootloader using esptool.py, and mklittlefs for extracting.
# run by either using the VSCode task "Custom" -> "Download Filesystem"
# or by doing 'pio run -t downloadfs' (with optional '-e <environment>') from the commandline.
# output will be saved, by default, in the "unpacked_fs" of the project.
# this folder can be changed by writing 'custom_unpack_dir = some_other_dir' in the corresponding platformio.ini
# environment.
import re
import sys
from os.path import isfile, join
from enum import Enum
import os
import subprocess
import shutil
Import("env")
platform = env.PioPlatform()
board = env.BoardConfig()
mcu = board.get("build.mcu", "esp32")


class FSType(Enum):
    LITTLEFS="littlefs"
    FATFS="fatfs"

class FSInfo:
    def __init__(self, fs_type, start, length, page_size, block_size):
        self.fs_type = fs_type
        self.start = start
        self.length = length
        self.page_size = page_size
        self.block_size = block_size
    def __repr__(self):
        return f"FS type {self.fs_type} Start {hex(self.start)} Len {self.length} Page size {self.page_size} Block size {self.block_size}"
    # extract command supposed to be implemented by subclasses
    def get_extract_cmd(self, input_file, output_dir):
        raise NotImplementedError()

class FS_Info(FSInfo):
    def __init__(self, start, length, page_size, block_size):
        self.tool = env["MKFSTOOL"]
        self.tool = join(platform.get_package_dir("tool-mkspiffs"), self.tool)
        super().__init__(FSType.LITTLEFS, start, length, page_size, block_size)
    def __repr__(self):
        return f"{self.fs_type} Start {hex(self.start)} Len {hex(self.length)} Page size {hex(self.page_size)} Block size {hex(self.block_size)}"
    def get_extract_cmd(self, input_file, output_dir):
        return f'"mkspiffs_espressif32_arduino" -b {self.block_size} -s {self.length} -p {self.page_size} --unpack "{output_dir}" "{input_file}"'

# SPIFFS helpers copied from ESP32, https://github.com/platformio/platform-espressif32/blob/develop/builder/main.py
# Copyright 2014-present PlatformIO <contact@platformio.org>
# Licensed under the Apache License, Version 2.0 (the "License");

def _parse_size(value):
    if isinstance(value, int):
        return value
    elif value.isdigit():
        return int(value)
    elif value.startswith("0x"):
        return int(value, 16)
    elif value[-1].upper() in ("K", "M"):
        base = 1024 if value[-1].upper() == "K" else 1024 * 1024
        return int(value[:-1]) * base
    return value

## Script interface functions
def parse_partition_table(content):
    entries = [e for e in content.split(b'\xaaP') if len(e) > 0]
    #print("Partition data:")
    for entry in entries:
        type = entry[1]
        if type in [0x82,0x83]: # SPIFFS or LITTLEFS
            offset = int.from_bytes(entry[2:5], byteorder='little', signed=False)
            size = int.from_bytes(entry[6:9], byteorder='little', signed=False)
            env["FS_START"] = offset
            env["FS_SIZE"] = size
            env["FS_PAGE"] = int("0x100", 16)
            env["FS_BLOCK"] = int("0x1000", 16)

def get_partition_table():
    esptoolpy = join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
    upload_port = join(env.get("UPLOAD_PORT", "none"))
    download_speed = join(str(board.get("download.speed", "115200")))
    if "none" in upload_port:
        env.AutodetectUploadPort()
        upload_port = join(env.get("UPLOAD_PORT", "none"))
    fs_file = join(env["PROJECT_DIR"], "partition_table_from_flash.bin")
    esptoolpy_flags = [
            "--chip", mcu,
            "--port", upload_port,
            "--baud",  download_speed,
            "--before", "default_reset",
            "--after", "hard_reset",
            "read_flash",
            "0x8000",
            "0x1000",
            fs_file
    ]
    esptoolpy_cmd = [env["PYTHONEXE"], esptoolpy] + esptoolpy_flags
    try:
        returncode = subprocess.call(esptoolpy_cmd, shell=False)
    except subprocess.CalledProcessError as exc:
        print("Downloading failed with " + str(exc))
    with open(fs_file, mode="rb") as file:
        content = file.read()
        parse_partition_table(content)

def get_fs_type_start_and_length():
    print(f"Retrieving filesystem info for {mcu}.")
    get_partition_table()
    return FS_Info(env["FS_START"], env["FS_SIZE"], env["FS_PAGE"], env["FS_BLOCK"])

def download_fs(fs_info: FSInfo):
    print(fs_info)
    esptoolpy = join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
    upload_port = join(env.get("UPLOAD_PORT", "none"))
    download_speed = join(str(board.get("download.speed", "115200")))
    if "none" in upload_port:
        env.AutodetectUploadPort()
        upload_port = join(env.get("UPLOAD_PORT", "none"))
    fs_file = join(env.subst("$BUILD_DIR"), f"downloaded_fs_{hex(fs_info.start)}_{hex(fs_info.length)}.bin")
    esptoolpy_flags = [
            "--chip", mcu,
            "--port", upload_port,
            "--baud",  download_speed,
            "--before", "default_reset",
            "--after", "hard_reset",
            "read_flash",
            hex(fs_info.start),
            hex(fs_info.length),
            fs_file
    ]
    esptoolpy_cmd = [env["PYTHONEXE"], esptoolpy] + esptoolpy_flags
    print("Download filesystem image")
    print(esptoolpy_cmd)
    try:
        returncode = subprocess.call(esptoolpy_cmd, shell=False)
        return (True, fs_file)
    except subprocess.CalledProcessError as exc:
        print("Downloading failed with " + str(exc))
        return (False, "")

def unpack_fs(fs_info: FSInfo, downloaded_file: str):

    unpack_dir = env.GetProjectOption("custom_unpack_dir", "unpacked_fs")
    cmd = fs_info.get_extract_cmd(downloaded_file, unpack_dir)
    print("Unpack files from filesystem image")
    try:
        returncode = subprocess.call(cmd, shell=True)
        return (True, unpack_dir)
    except subprocess.CalledProcessError as exc:
        print("Unpacking filesystem failed with " + str(exc))
        return (False, "")


def command_download_fs(*args, **kwargs):
    info = get_fs_type_start_and_length()
    _,downloaded_file = download_fs(info)
    unpack_fs(info, downloaded_file)


env.AddCustomTarget(
    name="downloadfs",
    dependencies=None,
    actions=[
        command_download_fs
    ],
    title="Download Filesystem",
    description="Downloads and displays files stored in the target ESP32/ESP8266"
)
