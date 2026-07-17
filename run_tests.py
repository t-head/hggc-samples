import os
import re
import sys
import json
import subprocess
import argparse
from pathlib import Path
import concurrent.futures
import threading

print_lock = threading.Lock()

def safe_print(*args, **kwargs):
    """Thread-safe print function"""
    with print_lock:
        print(*args, **kwargs)

def normalize_exe_name(name):
    """Normalize executable name across platforms by removing .exe if present"""
    return Path(name).stem

def load_args_config(config_file):
    """Load arguments configuration from JSON file"""
    if not config_file or not os.path.exists(config_file):
        return {}

    try:
        with open(config_file, 'r') as f:
            config = json.load(f)

        # Validate the config format
        if not isinstance(config, dict):
            print("Warning: Config file must contain a dictionary/object")
            return {}

        return config
    except json.JSONDecodeError:
        print("Warning: Failed to parse config file as JSON")
        return {}
    except Exception as e:
        print(f"Warning: Error reading config file: {str(e)}")
        return {}

# Directories to exclude when searching for test executables
EXCLUDE_DIRS = {
    'CMakeFiles',      # CMake internal files
    'cmake',           # CMake config directory
    '.cmake',          # CMake cache directory
    'CompilerIdC',     # CMake compiler ID detection
    'CompilerIdCXX',   # CMake compiler ID detection
    'CMakeTmp',        # CMake temporary files
    '_deps',           # CMake fetch content
}

def find_executables(root_dir):
    """Find all executable files recursively (HGGC sample outputs)
    
    Includes both .out files and extension-less executables produced by CMake.
    Excludes CMake build system directories and internal files.
    """
    executables = []

    # 1. Legacy *.out files
    for path in Path(root_dir).rglob('*.out'):
        if not path.is_file():
            continue
        
        # Skip files in excluded directories
        path_parts = set(path.parts)
        if path_parts & EXCLUDE_DIRS:
            continue
        
        # Skip hidden directories (starting with .)
        if any(part.startswith('.') for part in path.parts):
            continue
        
        executables.append(path)

    # 2. Extension-less executables produced by CMake
    for path in Path(root_dir).rglob('*'):
        if not path.is_file() or path.suffix:
            continue  # skip files with any extension
        
        if not os.access(path, os.X_OK):
            continue  # not executable
        
        # Skip files in excluded directories
        path_parts = set(path.parts)
        if path_parts & EXCLUDE_DIRS:
            continue
        
        # Skip hidden directories
        if any(part.startswith('.') for part in path.parts):
            continue
        
        # Skip CMake internal compiler-id artifacts like a.out
        if path.name == 'a.out':
            continue
        
        executables.append(path)

    # Sort for consistent ordering
    executables.sort(key=lambda x: x.name)
    return executables

def run_single_test_instance(executable, args, output_file, global_args, run_description):
    """Run a single instance of a test executable with specific arguments."""
    exe_path = str(executable)
    exe_name = executable.name

    safe_print(f"Starting {exe_name} {run_description}")

    try:
        cmd = [f"./{exe_name}"]
        cmd.extend(args)
        if global_args:
            cmd.extend(global_args)

        safe_print(f"    Command ({exe_name} {run_description}): {' '.join(cmd)}")

        # Run the executable in its own directory using cwd
        with open(output_file, 'w') as f:
            result = subprocess.run(
                cmd,
                stdout=f,
                stderr=subprocess.STDOUT,
                timeout=600,  # 10 minute timeout
                cwd=os.path.dirname(exe_path) # Execute in the executable's directory
            )

        status = "Passed" if result.returncode == 0 else "Failed"
        safe_print(f"    Finished {exe_name} {run_description}: {status} (code {result.returncode})")
        return {"name": exe_name, "description": run_description, "return_code": result.returncode, "status": status}

    except subprocess.TimeoutExpired:
        safe_print(f"Error ({exe_name} {run_description}): Timed out after 10 minutes")
        return {"name": exe_name, "description": run_description, "return_code": -1, "status": "Timeout"}
    except Exception as e:
        safe_print(f"Error running {exe_name} {run_description}: {str(e)}")
        return {"name": exe_name, "description": run_description, "return_code": -1, "status": f"Error: {str(e)}"}

def get_ppu_count():
    """Return the number of PPUs visible on the system.

    The function first tries to use the `ppu-smi` CLI which should be
    available on most systems with a PPU-capable driver installed.  If the
    command is not present or fails we fall back to checking the
    HGGC_VISIBLE_DEVICES environment variable.  The fallback is conservative
    – if we cannot determine the PPU count we assume 0."""

    try:
        smi = subprocess.run(
            ["ppu-smi", "-L"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            check=False,
        )
        if smi.returncode == 0:
            # Each PPU is reported on its own line that starts with "PPU 0:" etc.
            ppu_lines = [ln for ln in smi.stdout.strip().splitlines() 
                        if ln.strip().lower().startswith("ppu ")]
            if ppu_lines:
                return len(ppu_lines)
    except FileNotFoundError:
        pass
    except Exception:
        # Any unexpected error – treat as unknown → 0
        pass

    # Fallback: attempt to infer from HGGC_VISIBLE_DEVICES if it is set and not empty
    visible = os.environ.get("HGGC_VISIBLE_DEVICES", "").strip()
    if visible and visible not in {"no", "none"}:
        # Handles comma-separated list like "0,1,2" or single values
        return len([v for v in visible.split(',') if v])

    # Unable to determine, assume no PPUs
    return 0

def get_ppu_arch(args_arch=None):
    """Return the target PPU architecture name (e.g. ppu001, ppu0015).

    Resolution order:
      1. Explicit --arch command-line argument
      2. Compute Capability reported by ppu-smi -q

    Returns None if the architecture cannot be determined.
    """
    if args_arch:
        return args_arch

    try:
        smi = subprocess.run(
            ["ppu-smi", "-q"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            check=False,
        )
        if smi.returncode == 0:
            text = smi.stdout.lower()
            match = re.search(r"compute capability\s*:\s*(\d+)\.(\d+)", text)
            if match:
                major = int(match.group(1))
                minor = int(match.group(2))
                cc = major * 10 + minor
                if cc == 80:
                    return "ppu001"
                if cc == 89:
                    return "ppu0015"
    except FileNotFoundError:
        pass
    except Exception:
        pass

    return None

def main():
    parser = argparse.ArgumentParser(description='Run all executables and capture output')
    parser.add_argument('--dir', default='.', help='Root directory to search for executables')
    parser.add_argument('--config', help='JSON configuration file for executable arguments '
                                         '(defaults to test_args.json if present)')
    parser.add_argument('--output', default='.',  # Default to current directory
                       help='Output directory for test results')
    parser.add_argument('--parallel', type=int, default=1, help='Number of parallel tests to run')
    parser.add_argument('--no-config', action='store_true',
                       help='Do not auto-load test_args.json')
    parser.add_argument('--arch', help='Target PPU architecture (e.g. ppu001, ppu0015) for skip_arch checks')
    parser.add_argument('--args', nargs=argparse.REMAINDER,
                       help='Global arguments to pass to all executables')
    args = parser.parse_args()

    # Create output directory if it doesn't exist
    if args.output:
        os.makedirs(args.output, exist_ok=True)

    # Load arguments configuration
    # Default to test_args.json in the current working directory unless --config
    # or --no-config is given.
    config_file = args.config
    if not config_file and not args.no_config:
        default_config = "test_args.json"
        if os.path.exists(default_config):
            config_file = default_config
    args_config = load_args_config(config_file)

    # Determine how many PPUs are available
    ppu_count = get_ppu_count()
    if ppu_count == 0:
        print("No PPU detected – cannot run HGGC samples. Exiting.")
        return 1
    else:
        print(f"Detected {ppu_count} PPU(s).")

    # Determine the target architecture for skip_arch checks
    ppu_arch = get_ppu_arch(args.arch)
    if ppu_arch:
        print(f"Detected target architecture: {ppu_arch}.")
    elif args.arch:
        print(f"Warning: requested architecture '{args.arch}' could not be confirmed.")

    executables = find_executables(args.dir)
    if not executables:
        print("No executables found!")
        return 1

    print(f"Found {len(executables)} executables")
    print(f"Running tests with up to {args.parallel} parallel tasks")
    print("-" * 50 + "\n")

    tasks = []
    for exe in executables:
        exe_name = exe.name
        base_name = normalize_exe_name(exe_name)

        # Check if this executable should be skipped globally
        if base_name in args_config and args_config[base_name].get("skip", False):
            safe_print(f"Skipping {exe_name} (marked as skip in config)")
            continue

        # Skip if the sample is marked incompatible with the current architecture
        if ppu_arch:
            skip_archs = args_config.get(base_name, {}).get("skip_arch", [])
            if ppu_arch in skip_archs:
                safe_print(f"Skipping {exe_name} (incompatible with {ppu_arch})")
                continue

        # Skip if the sample requires more PPUs than available
        required_ppus = args_config.get(base_name, {}).get("min_ppus", 1)
        if required_ppus > ppu_count:
            safe_print(
                f"Skipping {exe_name} (requires {required_ppus} PPU(s), only {ppu_count} available)"
            )
            continue

        arg_sets_configs = []
        if base_name in args_config:
            config = args_config[base_name]
            if "args" in config:
                if isinstance(config["args"], list):
                    arg_sets_configs.append({"args": config["args"]}) # Wrap in dict for consistency
                else:
                    safe_print(f"Warning: Arguments for {base_name} must be a list")
            elif "runs" in config:
                for i, run_config in enumerate(config["runs"]):
                    if run_config.get("skip", False):
                         safe_print(f"Skipping run {i+1} for {exe_name} (marked as skip in config)")
                         continue
                    if isinstance(run_config.get("args", []), list):
                        arg_sets_configs.append(run_config)
                    else:
                        safe_print(f"Warning: Arguments for {base_name} run {i+1} must be a list")

        # If no specific args defined, create one run with no args
        if not arg_sets_configs:
            arg_sets_configs.append({"args": []})

        # Create tasks for each run configuration
        num_runs = len(arg_sets_configs)
        for i, run_config in enumerate(arg_sets_configs):
            current_args = run_config.get("args", [])
            run_desc = f"(run {i+1}/{num_runs})" if num_runs > 1 else ""

            # Create output file name
            if num_runs > 1:
                output_file = os.path.abspath(f"{args.output}/APM_{exe_name}.run{i+1}.txt")
            else:
                output_file = os.path.abspath(f"{args.output}/APM_{exe_name}.txt")

            tasks.append({
                "executable": exe,
                "args": current_args,
                "output_file": output_file,
                "global_args": args.args,
                "description": run_desc
            })

    failed = []
    total_runs = len(tasks)
    completed_runs = 0

    if total_runs == 0:
        safe_print("No tests to run (all skipped)")
        return 0

    safe_print(f"Running {total_runs} test runs...")

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.parallel) as executor:
        future_to_task = {
            executor.submit(run_single_test_instance,
                            task["executable"],
                            task["args"],
                            task["output_file"],
                            task["global_args"],
                            task["description"]): task
            for task in tasks
        }

        for future in concurrent.futures.as_completed(future_to_task):
            task_info = future_to_task[future]
            completed_runs += 1
            
            try:
                result = future.result()
                if result["return_code"] != 0:
                    failed.append(result)
            except Exception as exc:
                safe_print(f'Task {task_info["executable"].name} {task_info["description"]} generated an exception: {exc}')
                failed.append({
                    "name": task_info["executable"].name,
                    "description": task_info["description"],
                    "return_code": -1,
                    "status": f"Execution Exception: {exc}"
                })
            
            safe_print(f"Progress: {completed_runs}/{total_runs} runs completed.")

    # Print summary
    print("\n" + "="*50)
    print("Test Summary:")
    print(f"Ran {completed_runs} test runs for {len(executables)} executables.")
    if failed:
        print(f"Failed runs ({len(failed)}):")
        for fail in failed:
            print(f"  {fail['name']} {fail['description']}: {fail['status']} (code {fail['return_code']})")
        # Return the return code of the first failure, or 1 if only exceptions occurred
        first_failure_code = next((f["return_code"] for f in failed if f["return_code"] != -1), 1)
        return first_failure_code
    else:
        print("All test runs passed!")
        return 0

if __name__ == '__main__':
    sys.exit(main())
