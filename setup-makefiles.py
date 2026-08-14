#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

import os
import sys
import importlib.util

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..', '..', '..'))
EXTRACT_UTILS_DIR = os.path.join(ROOT_DIR, 'tools', 'extract-utils')
EXTRACT_FILES_PATH = os.path.join(SCRIPT_DIR, 'extract-files.py')

COMMON_DEVICES = ['X00TD', 'X01BD']
COMMON_DEVICES_STR = ' '.join(COMMON_DEVICES)

VENDOR_PATH = os.path.join(
    ROOT_DIR,
    'vendor', 'asus', 'sdm660-common'
)

# Чтобы работало и при запуске через python3 setup-makefiles.py
if EXTRACT_UTILS_DIR not in sys.path:
    sys.path.insert(0, EXTRACT_UTILS_DIR)

# Принудительно передаём режим генерации makefiles
sys.argv = [sys.argv[0], '--regenerate_makefiles']


def load_extract_files_module():
    if not os.path.isfile(EXTRACT_FILES_PATH):
        raise FileNotFoundError(f'Not found: {EXTRACT_FILES_PATH}')

    spec = importlib.util.spec_from_file_location(
        'extract_files_module',
        EXTRACT_FILES_PATH
    )
    if spec is None or spec.loader is None:
        raise ImportError(f'Failed to load module from: {EXTRACT_FILES_PATH}')

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


from extract_utils.main import ExtractUtils


def add_device_filter():
    device_filter = f'ifeq ($(TARGET_DEVICE),$(filter $(TARGET_DEVICE),{COMMON_DEVICES_STR}))'

    for mk in ['Android.mk', 'sdm660-common-vendor.mk']:
        mk_path = os.path.join(VENDOR_PATH, mk)
        if not os.path.exists(mk_path):
            print(f'[SKIP] Missing: {mk_path}')
            continue

        with open(mk_path, 'r', encoding='utf-8') as f:
            content = f.read()

        if device_filter in content:
            print(f'[SKIP] Filter already present: {mk}')
            continue

        content = content.rstrip() + '\n'

        with open(mk_path, 'w', encoding='utf-8') as f:
            f.write(f'{device_filter}\n')
            f.write(content)
            f.write('endif\n')

        print(f'[ OK ] Added filter to: {mk}')


if __name__ == '__main__':
    extract_files = load_extract_files_module()
    utils = ExtractUtils.device(extract_files.module)
    utils.run()
    add_device_filter()
    print('Done! Device filter added for: ' + COMMON_DEVICES_STR)
