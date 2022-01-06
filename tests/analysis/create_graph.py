import matplotlib.pyplot as plt
import json

results_file = "out/results.json"
with open(results_file, "r") as f:
    results = json.load(f)

results_folder = "out"
# build_types = ["Release", "Debug", "notype", "RelWithDebInfo", "MinSizeRel"]
build_types = ["Release"]
bar_width = 0.8
for build_type in build_types:
    dataset = []
    for result in results:
        if result["build_type"] == build_type:
            dataset.append(result)
    name_list = []
    run_time_list = []
    results_ticks = range(len(dataset))
    for machine in dataset:
        name_list.append(machine["machine"] + " " + machine["branch"])
        run_time_list.append(machine["run_time"])
    fig, ax = plt.subplots(6, 5, figsize=(66, 55))
    plots = []
    for row in ax:
        for col in row:
            plots.append(col)
    test_list = []
    for i in range(28):
        plot = plots[i]
        test_name = dataset[0]["tests"][i]["name"]
        results = []
        for machine in dataset:
            results.append(machine["tests"][i])
        test_list.append(results)

        warm_up_list = []
        total_time_list = []
        for individual_result in results:
            warm_up_list.append(individual_result["warm_up"])
            total_time_list.append(individual_result["total_time"])

        plot.barh(results_ticks, warm_up_list, height=bar_width, color="#ec407a")
        plot.barh(results_ticks, total_time_list, left=warm_up_list, height=bar_width, color="#ffa726")
        plot.set_yticks(results_ticks)
        plot.set_yticklabels(name_list, fontsize=12)
        plot.set_title(test_name + "\n Lower is better")
        plot.legend(["warm up time (ms)", "run time (ms)"])
        for j, v in enumerate(warm_up_list):
            run_time = total_time_list[j]
            plot.text(v / 2, j, str(round(v, 2)), va="center")
            plot.text(v + (run_time / 2), j, str(round(run_time, 2)), va="center")

    run_time_plot = plots[29]
    run_time_plot.barh(results_ticks, run_time_list, height=bar_width, color="#9ccc65")
    run_time_plot.set_yticks(results_ticks)
    run_time_plot.set_yticklabels(name_list, fontsize=12)
    run_time_plot.set_title("run time" + "\n Lower is better")
    run_time_plot.legend(["run time (s)"])
    for i, v in enumerate(run_time_list):
        run_time_plot.text(v / 2, i, str(round(v, 2)), va="center")
    plt.savefig(f"{results_folder}/{build_type}.png")
    plt.close()
