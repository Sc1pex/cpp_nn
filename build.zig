const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const nn_lib = build_nn_lib(b, target, optimize);
    const server = build_server(b, target, optimize, nn_lib);

    const run_server_step = b.step("run-server", "Run the server");
    const run_server = b.addRunArtifact(server);
    run_server_step.dependOn(&run_server.step);

    const cdb_step = b.step("cdb", "Compile CDB fragments into compile_commands.json");
    cdb_step.makeFn = collect_cdb_fragments;
    cdb_step.dependOn(&nn_lib.step);
    cdb_step.dependOn(&server.step);
    b.getInstallStep().dependOn(cdb_step);
}

fn build_nn_lib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const nn_lib = b.addLibrary(.{
        .name = "nn_lib",
        .root_module = b.createModule(
            .{
                .target = target,
                .optimize = optimize,
                .link_libcpp = true,
            },
        ),
    });

    nn_lib.addIncludePath(b.path("nn_lib/include"));
    nn_lib.installHeadersDirectory(b.path("nn_lib/include"), "nn_lib", .{
        .include_extensions = &.{"h"},
    });
    nn_lib.addCSourceFiles(.{
        .root = b.path("nn_lib/src"),
        .files = &.{
            "nn.cpp",
        },
        .flags = CXX_FLAGS,
    });

    const eigen_dep = b.dependency("eigen", .{});
    const eigen_lib = eigen_dep.artifact("eigen");
    nn_lib.linkLibrary(eigen_lib);

    // Make eigen headers available to consumers of nn_lib
    nn_lib.installed_headers.appendSlice(eigen_lib.installed_headers.items) catch unreachable;

    b.installArtifact(nn_lib);

    return nn_lib;
}

fn build_server(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, nn_lib: *std.Build.Step.Compile) *std.Build.Step.Compile {
    const httc_dep = b.dependency("httc", .{});
    const httc_lib = httc_dep.artifact("httc");

    const spdlog_dep = b.dependency("spdlog", .{});
    const spdlog_lib = spdlog_dep.artifact("spdlog");

    const server = b.addExecutable(.{
        .name = "nn_server",
        .root_module = b.createModule(
            .{
                .target = target,
                .optimize = optimize,
                .link_libcpp = true,
            },
        ),
    });
    server.addIncludePath(b.path("nn_web/server/include"));
    server.addCSourceFiles(.{
        .root = b.path("nn_web/server/src"),
        .files = &.{
            "main.cpp",
        },
        .flags = CXX_FLAGS,
    });
    server.linkLibrary(nn_lib);
    server.linkLibrary(httc_lib);
    server.linkLibrary(spdlog_lib);

    b.installArtifact(server);

    return server;
}

// Taken from https://zacoons.com/blog/2025-02-16-how-to-get-clang-lsp-working-with-zig/
fn collect_cdb_fragments(s: *std.Build.Step, _: std.Build.Step.MakeOptions) !void {
    // Open the compile_command.json file
    const outf = try std.fs.cwd().createFile("compile_commands.json", .{});
    defer outf.close();

    // Open the cdb-frags dir
    var dir = std.fs.cwd().openDir("cdb-frags", .{ .iterate = true }) catch {
        std.debug.print("Failed to open ./cdb-frags/", .{});
        return;
    };
    defer dir.close();

    // Iterate over the CDB fragments and add them to compile_commands.json
    try outf.writeAll("[");
    var iter = dir.iterate();

    const alloc = s.owner.allocator;

    while (try iter.next()) |entry| {
        const fpath = try std.fmt.allocPrint(std.heap.page_allocator, "cdb-frags/{s}", .{entry.name});
        const f = try std.fs.cwd().openFile(fpath, .{});

        const contents = try f.readToEndAlloc(alloc, 1024 * 1024);
        try outf.seekFromEnd(0);
        try outf.writeAll(contents);
    }
    try outf.writeAll("]");
}

const CXX_FLAGS = &.{
    "-std=c++23",
    "-Wall",
    "-Wextra",

    // https://github.com/ziglang/zig/issues/25455
    "-U_LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS",

    // Used to generate compile_commands.json fragments
    "-gen-cdb-fragment-path",
    "cdb-frags",
};
