import argparse
import os.path
import re
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MultipleLocator

number_pattern = re.compile(r"(\d+)\s*==\s*(\d+)")


def do_plot_work(entries, lib, time_label=None):
    labels = []
    successes = []
    failures = []
    overall = None
    for name, (succ, fail) in entries.items():
        if name == "Overall":
            overall = (succ, fail)
            continue
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

    os.makedirs("plots", exist_ok=True)
    plt.savefig(f"plots/{lib}_{time_label}.png")
    return overall


def plot_overall(data):
    result = {}
    for lib, value in data.items():
        if lib == 'LibDMTX' or lib == 'Combined':
            for timeout, vals in value.items():
                result[f"{lib}_{timeout}"] = vals
        else:
            result[lib] = value

    labels = []
    successes = []
    failures = []
    for name, (succ, fail) in result.items():
        labels.append(f"{name}")
        successes.append(succ)
        failures.append(fail)

    x = np.arange(len(labels))

    plt.figure(figsize=(8, 5))
    plt.bar(x, successes, label="Successes", color="green")
    plt.bar(x, failures, bottom=successes, label="Failures", color="red")

    plt.ylabel("Count")
    plt.title(f"Detection of Datamatrix Codes Overall")
    plt.xticks(x, labels, rotation=90)
    plt.yticks(list(plt.yticks()[0]) + [successes[0] + failures[0]])
    plt.legend()
    plt.tight_layout()

    os.makedirs("plots", exist_ok=True)
    plt.savefig("plots/Overall.png")


def plot_success_rates(data):
    overalls = {
        "LibDMTX": {
            "0ms": {},
            "100ms": {},
            "200ms": {}
        },
        "ZXing": {},
        "Combined": {
            "0ms": {},
            "100ms": {},
            "200ms": {}
        }
    }
    for lib, entries in data.items():
        if lib == "LibDMTX" or lib == "Combined":
            for time_label, a in entries.items():
                overalls[lib][time_label] = do_plot_work(a, lib, time_label)
        else:
            overalls[lib] = do_plot_work(entries, lib)
    plot_overall(overalls)


def get_data_from_xml(section):
    all_sections = section.findall(".//Section")
    for sub_section in all_sections:
        name = sub_section.get("name")
        if name and name.startswith("DMX"):
            overall = sub_section.find("OverallResults")
            if overall is not None:
                successes = int(overall.get("successes", 0))
                failures = int(overall.get("failures", 0))
                return name, successes, failures
        overall_result = check_overall(sub_section)
        if overall_result[0] is not None:
            return overall_result

    return check_overall(section)


def check_overall(section) -> tuple[str, int, int] | tuple[None, None, None]:
    expression = section.find("Expression")
    if expression is not None:
        expanded = expression.find("Expanded")
        match = number_pattern.search(expanded.text.strip())
        if match:
            found, total = int(match.group(1)), int(match.group(2))
            return "Overall", found, total - found

    return None, None, None


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
        "LibDMTX": {
            "0ms": {},
            "100ms": {},
            "200ms": {},
        },
        "ZXing": {},
        "Combined": {
            "0ms": {},
            "100ms": {},
            "200ms": {}
        }
    }

    for testcase in root.findall("TestCase"):
        for section in testcase.findall("Section"):
            if "LibDMTX" in testcase.get("name") or "Combined" in testcase.get("name"):
                timeout_name = section.get("name")
                if "timeout" in timeout_name:
                    if "100ms" in timeout_name:
                        timeout = "100ms"
                    elif "200ms" in timeout_name:
                        timeout = "200ms"
                    elif "0ms" in timeout_name:
                        timeout = "0ms"

                    name, successes, failures = get_data_from_xml(section)

                    if name is not None:
                        if "LibDMTX" in testcase.get("name"):
                            data["LibDMTX"][timeout][name] = (successes, failures)
                        elif "Combined" in testcase.get("name"):
                            data["Combined"][timeout][name] = (successes, failures)
            elif "ZXing" in testcase.get("name"):
                name, successes, failures = get_data_from_xml(section)
                if name is not None:
                    data["ZXing"][name] = (successes, failures)

    plot_success_rates(data)


if __name__ == '__main__':
    main()
