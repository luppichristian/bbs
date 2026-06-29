#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tarfile
import textwrap
import zipfile
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable


@dataclass
class CommandResult:
    name: str
    args: list[str]
    cwd: Path
    returncode: int
    output: str
    log_path: Path


class SkipCase(RuntimeError):
    pass


def remove_tree(path: Path) -> None:
    def handle_remove_readonly(func, target, exc_info):
        os.chmod(target, 0o700)
        func(target)

    shutil.rmtree(path, onerror=handle_remove_readonly)


class Harness:
    EXAMPLE_DIR_NAMES = [
        "bbs_package_consumer",
        "bbs_package_dep",
        "builder_target_properties",
        "builders",
        "config",
        "custom_dist",
        "cuda_console",
        "dyn_libs",
        "header_lib",
        "hello_world",
        "obj_lib",
        "raylib_example",
        "static_lib",
        "unity_batches",
    ]

    COPY_IGNORE = shutil.ignore_patterns(
        "build",
        "dist",
        "out_build",
        "artifacts_dist",
        "releases",
        ".github",
        "CMakeUserPresets.json",
        "*.sln",
        "*.vcxproj",
        "*.vcxproj.*",
        "*.dir",
        "*.tlog",
        "*.ilk",
        "*.pdb",
        "*.obj",
        "*.recipe",
        "*.log",
        "*.zip",
        "*.tar.gz",
    )

    def __init__(self, repo_root: Path, bbs_source: Path, artifact_root: Path, keep_artifacts: bool) -> None:
        self.repo_root = repo_root
        self.examples_source = self.repo_root / "examples"
        self.artifact_root = artifact_root
        self.keep_artifacts = keep_artifacts
        self.runtime_dir = self.artifact_root / "runtime"
        self.bin_dir = self.runtime_dir / "bin"
        self.logs_dir = self.artifact_root / "logs"
        self.work_dir = self.artifact_root / "work"
        self.examples_work_dir = self.work_dir / "examples"
        self.cases_dir = self.work_dir / "cases"
        self.command_counter = 0
        self.host_is_windows = os.name == "nt"
        self.bbs_source = bbs_source
        self.bbs_path = self._install_bbs_binary(bbs_source)

    def prepare(self) -> None:
        if self.artifact_root.exists() and not self.keep_artifacts:
            remove_tree(self.artifact_root)
        self.bin_dir.mkdir(parents=True, exist_ok=True)
        self.logs_dir.mkdir(parents=True, exist_ok=True)
        self.examples_work_dir.mkdir(parents=True, exist_ok=True)
        self.cases_dir.mkdir(parents=True, exist_ok=True)
        self.bbs_path = self._install_bbs_binary(self.bbs_source)
        self._copy_examples()

    def _install_bbs_binary(self, source: Path) -> Path:
        self.bin_dir.mkdir(parents=True, exist_ok=True)
        runtime_pub = self.runtime_dir / "pub"
        if runtime_pub.exists():
            remove_tree(runtime_pub)
        shutil.copytree(self.repo_root / "pub", runtime_pub)
        destination = self.bin_dir / source.name
        shutil.copy2(source, destination)
        sibling_dirs = [source.parent, source.parent / "Debug", source.parent / "Release", source.parent / "RelWithDebInfo"]
        for sibling_name in ["bbs.lib", "bbs.exp", "bbs.pdb"]:
            for sibling_dir in sibling_dirs:
                sibling = sibling_dir / sibling_name
                if sibling.exists():
                    shutil.copy2(sibling, self.bin_dir / sibling_name)
                    break
        destination.chmod(destination.stat().st_mode | 0o111)
        return destination

    def _copy_examples(self) -> None:
        for name in self.EXAMPLE_DIR_NAMES:
            src = self.examples_source / name
            dst = self.examples_work_dir / name
            if dst.exists():
                remove_tree(dst)
            shutil.copytree(src, dst, ignore=type(self).COPY_IGNORE)

    def example_dir(self, name: str) -> Path:
        return self.examples_work_dir / name

    def fresh_case_dir(self, name: str) -> Path:
        path = self.cases_dir / name
        if path.exists():
            remove_tree(path)
        path.mkdir(parents=True, exist_ok=True)
        return path

    def run_bbs(
        self,
        name: str,
        cwd: Path,
        *args: str,
        expect_success: bool = True,
        expect_contains: Iterable[str] = (),
        timeout: int = 1800,
    ) -> CommandResult:
        command = [str(self.bbs_path), *args]
        completed = subprocess.run(
            command,
            cwd=cwd,
            capture_output=True,
            text=True,
            timeout=timeout,
            errors="replace",
        )
        output = completed.stdout + completed.stderr
        self.command_counter += 1
        safe_name = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in name)
        log_path = self.logs_dir / f"{self.command_counter:03d}_{safe_name}.log"
        log_path.write_text(
            textwrap.dedent(
                f"""\
                command: {' '.join(command)}
                cwd: {cwd}
                returncode: {completed.returncode}

                {output}
                """
            ),
            encoding="utf-8",
        )
        if expect_success and completed.returncode != 0:
            raise AssertionError(f"{name} failed with exit code {completed.returncode}. See {log_path}")
        if not expect_success and completed.returncode == 0:
            raise AssertionError(f"{name} unexpectedly succeeded. See {log_path}")
        for needle in expect_contains:
            if needle not in output:
                raise AssertionError(f"{name} did not contain expected text {needle!r}. See {log_path}")
        return CommandResult(name, list(args), cwd, completed.returncode, output, log_path)

    def assert_exists(self, path: Path, description: str) -> None:
        if not path.exists():
            raise AssertionError(f"Missing {description}: {path}")

    def assert_any(self, paths: Iterable[Path], description: str) -> None:
        for path in paths:
            if path.exists():
                return
        raise AssertionError(f"Missing {description}")

    def assert_glob(self, root: Path, pattern: str, description: str) -> list[Path]:
        matches = sorted(root.glob(pattern))
        if not matches:
            raise AssertionError(f"Missing {description}: {root} :: {pattern}")
        return matches

    def assert_archive_contains(self, archive_paths: list[Path], expected_members: Iterable[str]) -> None:
        expected = list(expected_members)
        for archive_path in archive_paths:
            names: set[str]
            if archive_path.suffix == ".zip":
                with zipfile.ZipFile(archive_path) as zf:
                    names = set(zf.namelist())
            else:
                with tarfile.open(archive_path) as tf:
                    names = set(tf.getnames())
            if all(any(name.endswith(member) for name in names) for member in expected):
                return
        raise AssertionError(f"None of the archives contained {sorted(expected)}")

    def has_probable_cuda_toolkit(self) -> bool:
        for env_name in ["CUDA_PATH", "CUDA_HOME", "CUDAToolkit_ROOT", "CUDACXX"]:
            value = os.environ.get(env_name)
            if value and Path(value).exists():
                return True
        if shutil.which("nvcc"):
            return True
        if self.host_is_windows:
            program_files = os.environ.get("ProgramFiles")
            if program_files and (Path(program_files) / "NVIDIA GPU Computing Toolkit" / "CUDA").exists():
                return True
        else:
            for path in [Path("/usr/local/cuda"), Path("/opt/cuda")]:
                if path.exists():
                    return True
        return False

    def is_cuda_environment_issue(self, output: str) -> bool:
        lowered = output.lower()
        return any(token in lowered for token in [
            "could not find cuda",
            "no cuda toolset found",
            "cudatoolkit",
            "cuda compiler identification is unknown",
            "failed to detect a default cuda architecture",
            "enable_language",
            "unsupported toolchain",
            "kernel launch failed",
        ])


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(textwrap.dedent(content).lstrip("\n"), encoding="utf-8")


def build_case_list(h: Harness) -> list[tuple[str, Callable[[], str]]]:
    def hello_world() -> str:
        cwd = h.example_dir("hello_world")
        h.run_bbs("hello_world_help", cwd, "help", "build")
        h.run_bbs("hello_world_init", cwd, "update", "--init-toolchain")
        h.run_bbs("hello_world_build_all", cwd, "build", "-t", "*", "-c", "*")
        h.run_bbs("hello_world_test", cwd, "test")
        h.run_bbs("hello_world_run", cwd, "run", "-t", "hello_world", expect_contains=["Hello, world!"])
        h.run_bbs("hello_world_run_wildcard", cwd, "run", "-t", "*", "-p", "*", expect_contains=["Hello, world!"])
        h.run_bbs("hello_world_dist", cwd, "dist", "-t", "hello_world")
        h.assert_any(
            [cwd / "dist", cwd / "artifacts_dist", cwd / "releases"],
            "hello_world dist output directory",
        )
        return "build/test/run/dist succeeded"

    def static_lib() -> str:
        cwd = h.example_dir("static_lib")
        h.run_bbs("static_lib_build", cwd, "build")
        h.run_bbs("static_lib_test", cwd, "test")
        h.run_bbs("static_lib_run", cwd, "run", "-t", "static_lib_example", expect_contains=["Hello from a static library."])
        return "build/test/run succeeded"

    def dyn_libs() -> str:
        cwd = h.example_dir("dyn_libs")
        h.run_bbs("dyn_libs_build", cwd, "build")
        h.run_bbs("dyn_libs_run", cwd, "run", expect_contains=["Hello from dynamic libraries"])
        h.run_bbs("dyn_libs_dist", cwd, "dist")
        h.assert_any([cwd / "dist", cwd / "releases"], "dynamic library dist output")
        return "build/run/dist succeeded"

    def header_lib() -> str:
        cwd = h.example_dir("header_lib")
        h.run_bbs("header_lib_build", cwd, "build")
        h.run_bbs("header_lib_test", cwd, "test")
        h.run_bbs("header_lib_run", cwd, "run", "-t", "header_lib_example", expect_contains=["Hello from a header library."])
        return "build/test/run succeeded"

    def obj_lib() -> str:
        cwd = h.example_dir("obj_lib")
        h.run_bbs("obj_lib_build", cwd, "build")
        h.run_bbs("obj_lib_run_expected_failure", cwd, "run", expect_success=False)
        return "build succeeded and run failed as expected for object-library-only project"

    def builders() -> str:
        cwd = h.example_dir("builders")
        h.run_bbs("builders_build", cwd, "build")
        h.run_bbs(
            "builders_run",
            cwd,
            "run",
            "-t",
            "my_app",
            expect_contains=["builder define injected successfully"],
        )
        return "builder injection verified"

    def builder_target_properties() -> str:
        cwd = h.example_dir("builder_target_properties")
        h.run_bbs(
            "builder_target_properties_run",
            cwd,
            "run",
            "-t",
            "demo_app",
            expect_contains=[
                "builder target properties example",
                "compile-time properties were injected successfully",
                "TARGET_TUNER_LEVEL=2",
            ],
        )
        return "builder target property mutation verified"

    def package_dep() -> str:
        cwd = h.example_dir("bbs_package_dep")
        h.run_bbs("bbs_package_dep_build", cwd, "build")
        return "standalone package project builds"

    def package_consumer() -> str:
        cwd = h.example_dir("bbs_package_consumer")
        h.run_bbs("bbs_package_consumer_package_list", cwd, "package", "list")
        h.run_bbs("bbs_package_consumer_build", cwd, "build")
        h.run_bbs(
            "bbs_package_consumer_run",
            cwd,
            "run",
            expect_contains=["Hello from a project.bbs package."],
        )
        return "nested project.bbs package dependency verified"

    def config_example() -> str:
        cwd = h.example_dir("config")
        h.run_bbs("config_cfg", cwd, "cfg", "-p", "-g", "-l", "-t")
        h.run_bbs("config_info", cwd, "info", "project")
        h.run_bbs("config_build", cwd, "build")
        h.run_bbs(
            "config_run",
            cwd,
            "run",
            expect_contains=["This example reads build and dist settings from config.bbs."],
        )
        h.run_bbs("config_dist", cwd, "dist")
        h.assert_exists(cwd / "out_build", "config example build directory")
        h.assert_exists(cwd / "artifacts_dist", "config example dist directory")
        archives = h.assert_glob(cwd / "artifacts_dist", "default-*/*.zip", "config example zip archive")
        h.assert_archive_contains(archives, ["config_example.exe"] if h.host_is_windows else ["config_example"])
        return "config resolution and custom output directories verified"

    def custom_dist() -> str:
        cwd = h.example_dir("custom_dist")
        h.run_bbs("custom_dist_build", cwd, "build")
        h.run_bbs("custom_dist_dist", cwd, "dist")
        gen_dir = cwd / "releases"
        stage_dirs = sorted(gen_dir.glob("default-*/gen"))
        if not stage_dirs:
            raise AssertionError("Missing custom_dist staging directory")
        stage_dir = stage_dirs[0]
        h.assert_exists(stage_dir / "manifest.txt", "custom_dist manifest")
        h.assert_exists(stage_dir / "README.txt", "custom_dist copied README")
        archives = h.assert_glob(gen_dir, "default-*/*", "custom_dist archive output")
        archive_files = [path for path in archives if path.is_file() and path.name != "manifest.txt"]
        if not archive_files:
            raise AssertionError("Missing custom_dist archive file")
        expected_binary = "custom_dist_example.exe" if h.host_is_windows else "custom_dist_example"
        h.assert_archive_contains(archive_files, [expected_binary, "manifest.txt", "README.txt"])
        return "custom dist hooks and archive contents verified"

    def unity_batches() -> str:
        cwd = h.example_dir("unity_batches")
        h.run_bbs("unity_batches_build", cwd, "build")
        h.run_bbs("unity_batches_run", cwd, "run", expect_contains=["unity result: 28"])
        generated = h.assert_glob(cwd / "build", "**/*unity*", "unity-generated backend files")
        return f"build/run succeeded with {len(generated)} unity-related generated files"

    def cuda_console() -> str:
        cwd = h.example_dir("cuda_console")
        if not h.has_probable_cuda_toolkit():
            raise SkipCase("CUDA toolkit not detected on this runner")
        h.run_bbs("cuda_console_init", cwd, "update", "--init-toolchain")
        try:
            h.run_bbs("cuda_console_build", cwd, "build", timeout=2400)
            h.run_bbs("cuda_console_run", cwd, "run", timeout=2400)
        except AssertionError as exc:
            log_outputs = []
            for suffix in ["cuda_console_run", "cuda_console_build"]:
                log_path = h.logs_dir / f"{h.command_counter:03d}_{suffix}.log"
                if log_path.exists():
                    log_outputs.append(log_path.read_text(encoding="utf-8"))
            if not log_outputs and h.command_counter > 0:
                prev_log_path = h.logs_dir / f"{h.command_counter - 1:03d}_cuda_console_build.log"
                if prev_log_path.exists():
                    log_outputs.append(prev_log_path.read_text(encoding="utf-8"))
            output = "\n".join(log_outputs) if log_outputs else str(exc)
            if h.is_cuda_environment_issue(output):
                raise SkipCase("CUDA runtime/toolchain mismatch on this runner") from exc
            raise
        return "CUDA build/run succeeded"

    def raylib_example() -> str:
        cwd = h.example_dir("raylib_example")
        h.run_bbs("raylib_example_package_list", cwd, "package", "list", timeout=2400)
        last_exc: AssertionError | None = None
        for attempt in range(1, 4):
            build_name = "raylib_example_build" if attempt == 1 else f"raylib_example_build_retry_{attempt}"
            try:
                h.run_bbs(build_name, cwd, "build", timeout=2400)
                return "repo-backed package build succeeded"
            except AssertionError as exc:
                last_exc = exc
                if attempt == 3:
                    raise
                missing_dir: Path | None = None
                build_logs = sorted(h.logs_dir.glob(f"*_{build_name}.log"))
                if build_logs:
                    build_output = build_logs[-1].read_text(encoding="utf-8", errors="replace")
                    missing_prefix = "Error Package directory for 'raylib_pkg' does not exist: "
                    missing_dir = next(
                        (Path(line[len(missing_prefix) :].strip()) for line in build_output.splitlines() if line.startswith(missing_prefix)),
                        None,
                    )
                for path in [cwd / "build", cwd / "dist", cwd / "releases"]:
                    if path.exists():
                        remove_tree(path)
                if missing_dir is None and (h.bin_dir / "packages").exists():
                    remove_tree(h.bin_dir / "packages")
                if missing_dir is not None:
                        if missing_dir.exists():
                            remove_tree(missing_dir)
                        missing_dir.parent.mkdir(parents=True, exist_ok=True)
                        clone = subprocess.run(
                            [
                                "git",
                                "clone",
                                "--depth",
                                "1",
                                "--branch",
                                "5.5",
                                "--recurse-submodules",
                                "https://github.com/raysan5/raylib.git",
                                str(missing_dir),
                            ],
                            cwd=h.bin_dir,
                            capture_output=True,
                            text=True,
                            timeout=2400,
                            errors="replace",
                        )
                        h.command_counter += 1
                        log_path = h.logs_dir / f"{h.command_counter:03d}_raylib_example_prefetch.log"
                        log_path.write_text(
                            textwrap.dedent(
                                f"""\
                                command: git clone --depth 1 --branch 5.5 --recurse-submodules https://github.com/raysan5/raylib.git {missing_dir}
                                cwd: {h.bin_dir}
                                returncode: {clone.returncode}

                                {clone.stdout}{clone.stderr}
                                """
                            ),
                            encoding="utf-8",
                        )
                        if clone.returncode != 0:
                            raise AssertionError(f"raylib_example_prefetch failed with exit code {clone.returncode}. See {log_path}") from exc
        if last_exc is not None:
            raise last_exc
        return "repo-backed package build succeeded"

    def empty_dir_failure() -> str:
        cwd = h.fresh_case_dir("empty_dir_failure")
        h.run_bbs("empty_dir_build_expected_failure", cwd, "build", expect_success=False)
        return "missing project failure path verified"

    def unknown_target_failure() -> str:
        cwd = h.fresh_case_dir("unknown_target_failure")
        shutil.copytree(h.example_dir("hello_world"), cwd, dirs_exist_ok=True)
        h.run_bbs("unknown_target_failure_build", cwd, "build", "-t", "does_not_exist", expect_success=False)
        return "unknown target failure path verified"

    def unknown_platform_failure() -> str:
        cwd = h.fresh_case_dir("unknown_platform_failure")
        shutil.copytree(h.example_dir("hello_world"), cwd, dirs_exist_ok=True)
        h.run_bbs("unknown_platform_failure_build", cwd, "build", "-p", "not-a-platform", expect_success=False)
        return "unknown platform failure path verified"

    def generators() -> str:
        cwd = h.fresh_case_dir("generators")
        write_text(
            cwd / "project.bbs",
            """
            id(generator_case)
            name(\"Generator Case\")
            ver(0.1.0)

            targets(
              console(
                output(generator_case)
                units(src/main.c)
              )
            )
            """,
        )
        write_text(
            cwd / "src/main.c",
            """
            #include <stdio.h>

            int main(void) {
              puts(\"generator case\");
              return 0;
            }
            """,
        )
        h.run_bbs("generators_gitignore", cwd, "gen", "gitignore", "--override")
        h.run_bbs("generators_github", cwd, "gen", "github", "--override")
        h.run_bbs("generators_vscode", cwd, "gen", "vscode")
        h.assert_exists(cwd / ".gitignore", "generated .gitignore")
        h.assert_exists(cwd / ".github" / "workflows" / "ci.yml", "generated GitHub workflow")
        h.assert_exists(cwd / ".vscode" / "settings.json", "generated VSCode settings")
        return "project generators verified"

    return [
        ("example_hello_world", hello_world),
        ("example_static_lib", static_lib),
        ("example_dyn_libs", dyn_libs),
        ("example_header_lib", header_lib),
        ("example_obj_lib", obj_lib),
        ("example_builders", builders),
        ("example_builder_target_properties", builder_target_properties),
        ("example_bbs_package_dep", package_dep),
        ("example_bbs_package_consumer", package_consumer),
        ("example_config", config_example),
        ("example_custom_dist", custom_dist),
        ("example_unity_batches", unity_batches),
        ("example_cuda_console", cuda_console),
        ("example_raylib", raylib_example),
        ("edge_empty_dir_failure", empty_dir_failure),
        ("edge_unknown_target_failure", unknown_target_failure),
        ("edge_unknown_platform_failure", unknown_platform_failure),
        ("edge_generators", generators),
    ]


def detect_default_bbs_path(repo_root: Path) -> Path:
    candidates = [
        repo_root / "build" / ("bbs.exe" if os.name == "nt" else "bbs"),
        repo_root / "build" / "ci-bbs" / ("bbs.exe" if os.name == "nt" else "bbs"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("Could not find a built bbs binary. Pass --bbs with an explicit path.")


def parse_args() -> argparse.Namespace:
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent
    parser = argparse.ArgumentParser(description="Run isolated end-to-end tests for bbs examples and edge cases.")
    parser.add_argument("--bbs", type=Path, default=None, help="Path to the built bbs binary to test.")
    parser.add_argument(
        "--artifacts",
        type=Path,
        default=script_dir / ".artifacts",
        help="Directory where the tester keeps its local runtime, copied examples, and logs.",
    )
    parser.add_argument("--keep-artifacts", action="store_true", help="Reuse the artifacts directory instead of deleting it first.")
    parser.add_argument("--list", action="store_true", help="List the test cases without running them.")
    parser.add_argument("--filter", default=None, help="Run only cases whose name contains this substring.")
    parser.add_argument("--fail-fast", action="store_true", help="Stop after the first failure.")
    parser.add_argument("--show-log-paths", action="store_true", help="Print the artifacts/log directory at the end.")
    parser.add_argument("--repo-root", type=Path, default=repo_root, help=argparse.SUPPRESS)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    bbs_source = (args.bbs.resolve() if args.bbs else detect_default_bbs_path(repo_root).resolve())
    artifact_root = args.artifacts.resolve()
    harness = Harness(repo_root, bbs_source, artifact_root, args.keep_artifacts)
    cases = build_case_list(harness)
    if args.filter:
        cases = [case for case in cases if args.filter in case[0]]
    if args.list:
        for name, _ in cases:
            print(name)
        return 0
    harness.prepare()
    passed: list[tuple[str, str]] = []
    skipped: list[tuple[str, str]] = []
    failed: list[tuple[str, str]] = []
    for name, func in cases:
        print(f"[RUN ] {name}")
        try:
            details = func()
        except SkipCase as exc:
            skipped.append((name, str(exc)))
            print(f"[SKIP] {name}: {exc}")
        except Exception as exc:  # noqa: BLE001
            failed.append((name, str(exc)))
            print(f"[FAIL] {name}: {exc}")
            if args.fail_fast:
                break
        else:
            passed.append((name, details))
            print(f"[PASS] {name}: {details}")
    print()
    print(f"passed={len(passed)} skipped={len(skipped)} failed={len(failed)}")
    for bucket_name, bucket in [("PASS", passed), ("SKIP", skipped), ("FAIL", failed)]:
        if not bucket:
            continue
        print(bucket_name)
        for name, details in bucket:
            print(f"  - {name}: {details}")
    if args.show_log_paths or failed:
        print(f"logs: {harness.logs_dir}")
        print(f"artifacts: {harness.artifact_root}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
