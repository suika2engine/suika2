import cv2
import numpy as np
import sys
import os
import glob
import subprocess

ANSWER_DIR = sys.argv[1]
RESULT_DIR = sys.argv[2]

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
        print(basename + " differs " + str(delta))
        error_count = error_count + 1

        # Write a difference image.
        if not os.path.isdir("diff"):
            os.mkdir("diff")
        subprocess.run(["compare",
                        "-metric",
                        "AE",
                        ANSWER_DIR + "/" + basename,
                        RESULT_DIR + "/" + basename,
                        "diff/" + basename])

# Fail if there were errors.
if error_count > 0:
    sys.exit(1)

# Success