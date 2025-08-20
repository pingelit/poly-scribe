import argparse
import datetime
import os
import re
from pathlib import Path

now = datetime.datetime.now(tz=datetime.UTC)


def advance_changelog(file: Path, version: str) -> bool:
    with open(file, errors="surrogateescape") as f:
        file_content = f.read()

        file_start = """# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog]"""

        # check if the file starts with the expected header
        if not file_content.startswith(file_start):
            print(f"File {file} does not start with the expected header")
            return False

        match = re.search(r"## \[Unreleased\]", file_content)

        if not match:
            return False

        if re.search(rf"## \[{version}\]", file_content):
            print(f"Version {version} already exists in {file}")
            return False

        current_date = now.strftime("%Y-%m-%d")
        file_content = re.sub(
            r"## \[Unreleased\]",
            f"## [Unreleased]\n\n## [{version}] - {current_date}",
            file_content,
        )

    with open(file, "w", errors="surrogateescape") as f:
        f.write(file_content)
    return True


def main():
    parser = argparse.ArgumentParser(description="Advance the changelog file based on 'Keep a Changelog'")
    parser.add_argument("--version", help="The version to advance the changelog to", default=None)
    parser.add_argument("--file", help="The path to the changelog file", default="CHANGELOG.md")

    args = parser.parse_args()

    version = args.version
    if not version:
        version = os.environ["BVHOOK_NEW_VERSION"]

    changelog_file = Path.cwd() / args.file

    print(f"Advancing changelog in {changelog_file} to version {version}")

    if not changelog_file.exists():
        msg = "Changelog file not found in the current directory"
        raise FileNotFoundError(msg)

    if not advance_changelog(changelog_file, version):
        msg = "Failed to advance the changelog"
        raise ValueError(msg)


if __name__ == "__main__":
    main()
