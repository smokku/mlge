const std = @import("std");
const raylib_build = @import("ext/raylib/src/build.zig");
const ext_build = @import("ext/build.zig");

const cdb_path = "zig-cache/cdb";

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const flags = [_][]const u8{
        "-Wall",
        "-Wextra",
        "-Werror=return-type",
        "-gen-cdb-fragment-path",
        cdb_path,
    };

    const cflags = flags ++ [_][]const u8{
        "-std=c99",
    };
    _ = cflags;
    const cxxflags = flags ++ [_][]const u8{
        "-std=c++17",
        "-fno-rtti",
    };

    // --- vendor ---

    const raylib = raylib_build.addRaylib(b, target, optimize);
    raylib.defineCMacro("SUPPORT_FILEFORMAT_TGA", null);
    raylib.defineCMacro("SUPPORT_FILEFORMAT_JPG", null);
    const yojimbo = ext_build.addYojimbo(b, target, optimize);
    const rmlui = ext_build.addRmlUi(b, target, optimize) catch |err|
        std.debug.panic("RmlUi build failed: {}", .{err});

    // --- shared game logic ---

    const shared = b.addStaticLibrary(.{
        .name = "shared",
        .root_source_file = .{ .path = "shared/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const shared_tests = b.addTest(.{
        .root_source_file = .{ .path = "shared/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // --- game graphical client ---

    const client = b.addExecutable(.{
        .name = "client",
        // .root_source_file = .{ .path = "client/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    client.addCSourceFiles(&.{
        "client/main.cpp",
        "client/rml.cpp",
    }, &cxxflags);

    client.addIncludePath("ext/fmt/include");
    client.addCSourceFiles(&.{"ext/fmt/src/format.cc"}, &cxxflags);

    client.linkLibrary(shared);

    client.defineCMacro("PLATFORM_DESKTOP", null);
    client.addIncludePath("ext/raylib/src");
    client.linkLibrary(raylib);
    client.addIncludePath("ext/raylib-cpp/include");

    ext_build.addRmlUiOpts(client);
    client.linkLibrary(rmlui);

    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    client.install();

    // This *creates* a RunStep in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const client_cmd = client.run();

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    client_cmd.step.dependOn(b.getInstallStep());

    // This allows the user to pass arguments to the application in the build
    // command itself, like this: `zig build run -- arg1 arg2 etc`
    if (b.args) |args| {
        client_cmd.addArgs(args);
    }

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    const client_step = b.step("client", "Run the client");
    client_step.dependOn(&client_cmd.step);

    // --- headless game server ---

    const server = b.addExecutable(.{
        .name = "server",
        // .root_source_file = .{ .path = "server/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    server.addCSourceFiles(&.{
        "server/server.cpp",
    }, &cxxflags);

    server.linkLibCpp();

    server.linkLibrary(shared);

    server.addIncludePath("ext/yojimbo");
    server.linkLibrary(yojimbo);

    server.install();

    const server_cmd = server.run();
    server_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        server_cmd.addArgs(args);
    }

    const server_step = b.step("server", "Run the server");
    server_step.dependOn(&server_cmd.step);

    const server_tests = b.addTest(.{
        .root_source_file = .{ .path = "server/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // --- unit testing ---

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&shared_tests.step);
    test_step.dependOn(&server_tests.step);

    // --- tooling ---

    // const clean_cdb = b.addRemoveDirTree(cdb_path);

    const cdb_step = b.step("cdb", "Create compile_commands.json");
    cdb_step.makeFn = &makeCdb;
    cdb_step.dependOn(&shared.step);
    cdb_step.dependOn(&client.step);
    cdb_step.dependOn(&server.step);
}

fn makeCdb(b: *std.Build.Step) !void {
    var cdb_file = try std.fs.cwd().createFile("compile_commands.json", .{ .truncate = true });
    defer cdb_file.close();

    _ = try cdb_file.write("[\n");

    var dir = std.fs.cwd().openIterableDir(cdb_path, .{ .no_follow = true }) catch |err| {
        std.debug.print("compilation database fragments dir `{s}` misssing\n", .{cdb_path});
        return err;
    };

    var walker = try dir.walk(b.dependencies.allocator);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        const ext = std.fs.path.extension(entry.basename);
        if (std.mem.eql(u8, ext, ".json")) {
            var json_file = try entry.dir.openFile(entry.basename, .{});
            defer json_file.close();

            try cdb_file.writeFileAllUnseekable(json_file, .{});
        }
    }

    _ = try cdb_file.write("]\n");
}
