import json
import subprocess

test_count = 100
out_folder = "analysis/test/"
test_name = "test-multisig_double_spend_alt_true-2"

success = []
fail = []
segmentation_fault = []

subprocess.run(f"mkdir -p {out_folder}/{test_name}", shell=True)

for test in range(test_count):
    process = subprocess.run(f"./core_tests --generate_and_play_test_data > {out_folder}/{test_name}/{test}.log",
                             shell=True)
    if process.returncode == 0:
        success.append(test)
    elif process.returncode == 139:
        segmentation_fault.append(test)
    else:
        fail.append(test)

results = {
    "stats": {
        "success": len(success),
        "fail": len(fail),
        "segmentation_fault": len(segmentation_fault),
        "total": len(success) + len(fail) + len(segmentation_fault)
    },
    "test_count": test_count,
    "success": success,
    "fail": fail,
    "segmentation_fault": segmentation_fault
}

with open(f"{out_folder}/{test_name}/results.json", "w") as json_file:
    json.dump(results, json_file)
