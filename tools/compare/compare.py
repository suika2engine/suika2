import cv2
import numpy as np
import sys
import os
import glob
import subprocess
import tkinter as tk
import tkinter.messagebox as messagebox

if len(sys.argv) < 3:
    ANSWER_DIR = 'record'
    RESULT_DIR = 'replay'
else:
    ANSWER_DIR = sys.argv[1]
    RESULT_DIR = sys.argv[2]

if not os.path.isdir(ANSWER_DIR):
    messagebox.showinfo('suika-compare', 'Cannot open ' + ANSWER_DIR)
if not os.path.isdir(RESULT_DIR):
    messagebox.showinfo('suika-compare', 'Cannot open ' + RESULT_DIR)

os.remove('diff.txt')

error_count = 0
files = glob.glob(ANSWER_DIR + "/*.png")
for file in files:
    basename = os.path.basename(file)

    # Read images.
    answer_img = cv2.imread(ANSWER_DIR + "/" + basename)
    result_img = cv2.imread(RESULT_DIR + "/" + basename)

    # Get ndarrays.
    answer_array = np.asarray(answer_img).astype(np.float32)
    result_array = np.asarray(result_img).astype(np.float32)

    # Get delta.
    diff_array = answer_array - result_array
    delta = diff_array.max()

    # If the difference between the channel values is greater than 1,
    # it is considered an error.
    if delta > 1:
        error_count = error_count + 1
        with open('diff.txt', 'a') as f:
            print(basename + "differs.")

# Fail if there were errors.
if error_count > 0:
    messagebox.showinfo('suika-compare', 'BAD: ' + str(error_count) + ' diffs.')
    sys.exit(1)

# Success
messagebox.showinfo('suika-compare', 'OK: No diff.')
