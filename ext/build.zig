const std = @import("std");

pub fn addYojimbo(b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode) *std.Build.CompileStep {
    const yojimbo = b.addStaticLibrary(.{
        .name = "yojimbo",
        .target = target,
        .optimize = optimize,
    });

    yojimbo.linkLibC();
    yojimbo.linkSystemLibrary("sodium");
    yojimbo.linkSystemLibrary("mbedtls");
    yojimbo.linkSystemLibrary("mbedx509");
    yojimbo.linkSystemLibrary("mbedcrypto");

    yojimbo.addIncludePath(srcdir ++ "/yojimbo/netcode.io");
    yojimbo.addIncludePath(srcdir ++ "/yojimbo/reliable.io");

    yojimbo.addCSourceFiles(&.{
        srcdir ++ "/yojimbo/certs.c",
        srcdir ++ "/yojimbo/certs.h",
        srcdir ++ "/yojimbo/netcode.io/netcode.c",
        srcdir ++ "/yojimbo/reliable.io/reliable.c",
        srcdir ++ "/yojimbo/tlsf/tlsf.c",
        srcdir ++ "/yojimbo/tlsf/tlsf.h",
    }, &.{});

    yojimbo.addCSourceFiles(&.{
        srcdir ++ "/yojimbo/yojimbo.cpp",
    }, &.{
        "-fno-rtti",
    });

    switch (optimize) {
        .Debug => {
            yojimbo.defineCMacro("YOJIMBO_DEBUG", null);

            yojimbo.linkLibCpp(); // YOJIMBO_DEBUG_MESSAGE_LEAKS needs <map>
        },
        else => {
            yojimbo.defineCMacro("YOJIMBO_RELEASE", null);
        },
    }

    return yojimbo;
}

const srcdir = struct {
    fn getSrcDir() []const u8 {
        return std.fs.path.dirname(@src().file).?;
    }
}.getSrcDir();
