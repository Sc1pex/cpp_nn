const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

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

    nn_lib.installHeadersDirectory(b.path("nn_lib/include"), "nn_lib", .{
        .include_extensions = &.{"h"},
    });
    const flags = CXX_FLAGS ++ .{"-Inn_lib/include/"};
    nn_lib.addCSourceFiles(.{
        .root = b.path("nn_lib/src"),
        .files = &.{},
        .flags = &flags,
    });

    const eigen_dep = b.dependency("eigen", .{});
    const eigen_lib = eigen_dep.artifact("eigen");
    nn_lib.linkLibrary(eigen_lib);

    b.installArtifact(nn_lib);

    const cdb_step = b.step("cdb", "Compile CDB fragments into compile_commands.json");
    cdb_step.makeFn = collect_cdb_fragments;
    cdb_step.dependOn(&nn_lib.step);
    b.getInstallStep().dependOn(cdb_step);
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

const CXX_FLAGS = .{
    "-std=c++23",
    "-Wall",
    "-Wextra",

    // https://github.com/ziglang/zig/issues/25455
    "-U_LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS",

    // Used to generate compile_commands.json fragments
    "-gen-cdb-fragment-path",
    "cdb-frags",
};
