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

pub fn addRmlUi(b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode) !*std.Build.CompileStep {
    const rmlui = b.addStaticLibrary(.{
        .name = "rmlui",
        .target = target,
        .optimize = optimize,
    });

    addRmlUiOpts(rmlui);

    var sources = std.ArrayList([]const u8).init(b.allocator);
    {
        const prefix = srcdir ++ "/rmlui/Source/Core";
        var path: [std.fs.MAX_PATH_BYTES]u8 = .{};
        std.mem.copy(u8, &path, prefix);
        path[prefix.len] = '/';

        var dir = try std.fs.cwd().openIterableDir(prefix, .{});
        var walker = try dir.walk(b.allocator);
        defer walker.deinit();

        const exts = [_][]const u8{ ".c", ".cpp" };
        while (try walker.next()) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            if (for (exts) |e| {
                if (std.mem.eql(u8, ext, e)) break true;
            } else false) {
                std.mem.copy(u8, path[prefix.len + 1 ..], entry.path);
                try sources.append(b.dupe(path[0 .. prefix.len + 1 + entry.path.len]));
            }
        }
    }
    rmlui.addCSourceFiles(sources.items, &.{
        "-std=c++14",
        "-Wall",
        "-pedantic",
        "-Wextra",
        "-Werror",
        "-fno-rtti",
    });

    return rmlui;
}

pub fn addRmlUiOpts(step: *std.Build.CompileStep) void {
    step.linkLibCpp();
    step.linkSystemLibrary("freetype");
    step.addIncludePath("/usr/include/freetype2");

    step.addIncludePath(srcdir ++ "/rmlui/Include");

    step.defineCMacro("RMLUI_STATIC_LIB", null);
    step.defineCMacro("RMLUI_USE_CUSTOM_RTTI", null);
    step.defineCMacro("RMLUI_NO_THIRDPARTY_CONTAINERS", null);
}

const srcdir = struct {
    fn getSrcDir() []const u8 {
        return std.fs.path.dirname(@src().file).?;
    }
}.getSrcDir();
