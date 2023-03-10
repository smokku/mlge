const std = @import("std");
const raylib_build = @import("ext/raylib/src/build.zig");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // ---- vendor ----

    const raylib = raylib_build.addRaylib(b, target, optimize);
    raylib.install();

    // ---- shared game logic ----

    const shared = b.addStaticLibrary(.{
        .name = "shared",
        .root_source_file = .{ .path = "shared/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    shared.install();

    const shared_tests = b.addTest(.{
        .root_source_file = .{ .path = "shared/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // ---- game graphical client ----

    const client = b.addExecutable(.{
        .name = "client",
        .root_source_file = .{ .path = "client/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    client.linkLibrary(shared);

    client.addIncludePath("ext/raylib/src");
    client.linkLibrary(raylib);

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

    // Creates a step for unit testing.
    const client_tests = b.addTest(.{
        .root_source_file = .{ .path = "client/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    // ---- headless game server ----

    const server = b.addExecutable(.{
        .name = "server",
        .root_source_file = .{ .path = "server/main.zig" },
        .target = target,
        .optimize = optimize,
    });

    server.linkLibrary(shared);

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

    // ---- unit testing ----

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&shared_tests.step);
    test_step.dependOn(&client_tests.step);
    test_step.dependOn(&server_tests.step);
}
