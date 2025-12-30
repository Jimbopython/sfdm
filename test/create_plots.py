import argparse
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MultipleLocator


def do_plot_work(entries, lib, time_label=None):
    labels = []
    successes = []
    failures = []
    for name, (succ, fail) in entries.items():
        labels.append(f"{name}")
        successes.append(succ)
        failures.append(fail)

    x = np.arange(len(labels))

    plt.figure(figsize=(8, 5))
    plt.bar(x, successes, label="Successes", color="green")
    plt.bar(x, failures, bottom=successes, label="Failures", color="red")

    plt.ylabel("Count")
    plt.title(f"Detection of Datamatrix Codes with {lib} (timeout: {time_label})")
    plt.xticks(x, labels, rotation=90)
    plt.gca().yaxis.set_major_locator(MultipleLocator(1))
    plt.legend()
    plt.tight_layout()

    plt.show()


def plot_success_rates(data):
    for lib, entries in data.items():
        if lib == "LibDMTX":
            for time_label, a in entries.items():
                do_plot_work(a, lib, time_label)
        else:
            do_plot_work(entries, lib)


def main():
    parser = argparse.ArgumentParser(description="Process Catch2 xml test results")
    parser.add_argument(
        "xml_file",
        type=str,
        help="Path to the XML file"
    )
    args = parser.parse_args()

    tree = ET.parse(args.xml_file)
    root = tree.getroot()

    data = {
        "LibDMTX":
            {
                "0ms": {},
                "100ms": {},
                "200ms": {},
            },
        "ZXing": {}
    }

    # Iterate over all TestCase elements
    for testcase in root.findall("TestCase"):
        for section in testcase.findall("Section"):
            timeout_name = section.get("name")  # e.g., "100ms timeout"
            if "timeout" in timeout_name:
                if "100ms" in timeout_name:
                    timeout = "100ms"
                elif "200ms" in timeout_name:
                    timeout = "200ms"
                elif "0ms" in timeout_name:
                    timeout = "0ms"
                for sub_section in section.findall(".//Section"):
                    name = sub_section.get("name")
                    if name and name.startswith("DMX"):
                        overall = sub_section.find("OverallResults")
                        if overall is not None:
                            successes = int(overall.get("successes", 0))
                            failures = int(overall.get("failures", 0))

                            data["LibDMTX"][timeout][name] = (successes, failures)
            elif "ZXing" in testcase.get("name"):
                for sub_section in section.findall(".//Section"):
                    name = sub_section.get("name")
                    if name and name.startswith("DMX"):
                        overall = sub_section.find("OverallResults")
                        if overall is not None:
                            successes = int(overall.get("successes", 0))
                            failures = int(overall.get("failures", 0))

                            data["ZXing"][name] = (successes, failures)
            else:
                continue

    plot_success_rates(data)


if __name__ == '__main__':
    main()
