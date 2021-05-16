
cc_binary(
    name = "A056744",
    srcs = glob(["**/*.c*", "**/*.h*"]),
    deps = [
        "@cli11//:cli11",
        "@spdlog//:headers",
    ],

    copts = select({
        "@bazel_tools//src/conditions:windows": ["/std:c++latest"],
        "//conditions:default": ["-std=c++20"],
    }),
)
