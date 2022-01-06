# performance tests analysis

## parse_performance_tests

Reads all performance_tests results: `./performance_tests > results.txt` and save the results to a json file.

Results must be named following this rule: `results/<machine>/performance_tests_<git-branch>_<build-type>.txt`

## create_graph

Reads json created by `parse_performance_tests` and saves a picture with a graph.
