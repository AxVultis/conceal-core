import os
import glob
import re
import json

base_folder = "results"
pattern = "performance_tests_*.txt"
out_json = "out/results.json"
all_results = []
for root, dirs, files in os.walk(base_folder, topdown=True):
    for machine in dirs:
        results = sorted(glob.glob(os.path.join(os.path.join(root, machine), pattern)))
        for txt_result in results:
            re_file = re.search(".*_([A-Za-z0-9-]*)_([A-Za-z]*).txt", txt_result)
            if re_file:
                branch = re_file.group(1)
                build_type = re_file.group(2) if re_file.group(2) != "" else "notype"
                with open(txt_result, "r") as perf_result:
                    result = {
                        "machine": machine,
                        "branch": branch,
                        "build_type": build_type,
                        "tests": []
                    }
                    for line in perf_result:
                        re_warm_up = re.search("Warm up: ([0-9]*) ms", line)
                        if re_warm_up:
                            warm_up = re_warm_up.group(1)
                            raw_test_line = next(perf_result)
                            re_test_name = re.search("test_([a-z_]*)<?([0-9]*)?(?:, )?([0-9]*)?>? - ([A-Z]*)",
                                                     raw_test_line)
                            if re_test_name:
                                test_name = re_test_name.group(1)
                                first = re_test_name.group(2)
                                second = re_test_name.group(3)
                                status = re_test_name.group(4)
                                raw_loop_count = next(perf_result)
                                re_loop_count = re.search(" {2}loop count: {4}([0-9]*)", raw_loop_count)
                                if re_loop_count:
                                    loop_count = re_loop_count.group(1)
                                    raw_total_time = next(perf_result)
                                    re_total_time = re.search(" {2}elapsed: {7}([0-9]*)", raw_total_time)
                                    if re_total_time:
                                        total_time = re_total_time.group(1)
                                        raw_time_per_call = next(perf_result)
                                        re_time_per_call = re.search(" {2}time per call: ([0-9]*)", raw_time_per_call)
                                        if re_time_per_call:
                                            time_per_call = re_time_per_call.group(1)
                                            test = {
                                                "name": test_name,
                                                "first": int(first) if first != "" else 0,
                                                "second": int(second) if second != "" else 0,
                                                "status": status,
                                                "loop_count": int(loop_count),
                                                "warm_up": int(warm_up),
                                                "total_time": int(total_time),
                                                "time_per_call": int(time_per_call)
                                            }
                                            result["tests"].append(test)
                        re_total = re.search(".*: ([0-9]*) sec", line)
                        if re_total:
                            total = re_total.group(1)
                            result["run_time"] = int(total)
                    all_results.append(result)

with open(out_json, "w") as json_file:
    json.dump(all_results, json_file)
