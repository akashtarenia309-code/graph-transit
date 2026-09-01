import argparse
import statistics
import subprocess
import time
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent


def run_benchmark(executable, iterations):
    times = []

    for iteration in range(iterations):
        start = time.perf_counter()

        result = subprocess.run(
            [str(executable)],
            capture_output=True,
            text=True
        )

        end = time.perf_counter()

        if result.returncode != 0:
            print("Program execution failed.")
            print(result.stderr)
            return None

        elapsed = end - start
        times.append(elapsed)

        print(
            f"Run {iteration + 1}/{iterations}: "
            f"{elapsed * 1000:.3f} ms"
        )

    return times


def print_statistics(times):
    milliseconds = [value * 1000 for value in times]

    print("\n===== BENCHMARK RESULTS =====\n")

    print(f"Runs: {len(milliseconds)}")
    print(f"Minimum: {min(milliseconds):.3f} ms")
    print(f"Maximum: {max(milliseconds):.3f} ms")
    print(f"Average: {statistics.mean(milliseconds):.3f} ms")

    if len(milliseconds) > 1:
        print(
            f"Standard deviation: "
            f"{statistics.stdev(milliseconds):.3f} ms"
        )


def main():
    parser = argparse.ArgumentParser(
        description="Benchmark the GraphTransit executable."
    )

    parser.add_argument(
        "--executable",
        default=None,
        help="Path to GraphTransit executable."
    )

    parser.add_argument(
        "--iterations",
        type=int,
        default=10,
        help="Number of benchmark runs."
    )

    args = parser.parse_args()

    if args.iterations <= 0:
        raise ValueError("Iterations must be positive.")

    if args.executable is None:
        executable = PROJECT_ROOT / "graph_transit.exe"
    else:
        executable = Path(args.executable).resolve()

    if not executable.exists():
        raise FileNotFoundError(
            f"Executable not found: {executable}"
        )

    print("===== GRAPH TRANSIT BENCHMARK =====\n")
    print(f"Executable: {executable}")
    print(f"Iterations: {args.iterations}\n")

    times = run_benchmark(
        executable,
        args.iterations
    )

    if times is not None:
        print_statistics(times)


if __name__ == "__main__":
    main()