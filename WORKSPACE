workspace(name = "github_com_mason-bially_for-davis")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")


########################
# CLI11:
new_git_repository(
    name = "cli11",
    remote = "https://github.com/CLIUtils/CLI11.git",
    commit = "5cb3efabce007c3a0230e4cc2e27da491c646b6c",
    shallow_since = "1592663367 -0400",
    build_file_content = """
cc_library(
    name = "cli11",
    visibility = ["//visibility:public"],
    hdrs = glob([
        "include/**/*.h*",
    ]),
    includes = ["include"],
)
    """,
)

########################
# spdlog:
git_repository(
    name = "spdlog",
    remote = "https://github.com/cgrinker/spdlog",
    commit = "d853f44b345d752538c5e3cd8c5f68e9f45e6806",
    shallow_since = "1611099882 -0800",
)
