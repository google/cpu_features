// usage:
// 1. copy paste the whole code to https://run.dlang.io/ then click `run`
// 2. `docker run --rm -it -v $(pwd):/src dlanguage/dmd dmd -run generate_badges.d`
import std.algorithm : each, map, cartesianProduct, filter, joiner, sort, uniq;
import std.array;
import std.conv : to;
import std.format;
import std.range : chain, only;
import std.stdio;
import std.traits : EnumMembers;
import std.string : representation;
import std.base64 : Base64;

immutable string bazel_svg = `<svg role="img" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M6 .16l5.786 5.786L6 11.732.214 5.946 6 .161zM0 6.214V12l5.786 5.786V12L0 6.214zM18 .16l5.786 5.786L18 11.732l-5.786-5.786L18 .161zM24 6.214V12l-5.786 5.786V12L24 6.214zM12 6.16l5.786 5.786L12 17.732l-5.786-5.786L12 6.161zM11.84 18.054v5.785l-5.786-5.785v-5.786l5.785 5.786zM12.16 18.054l5.786-5.786v5.786l-5.785 5.785v-5.785z" stroke="transparent" fill="white"/></svg>`;
const string bazel_svg_base64 = Base64.encode(representation(bazel_svg));

enum BuildSystem
{
    CMake,
    Bazel
}

enum Cpu
{
    amd64,
    AArch64,
    ARM,
    MIPS,
    POWER,
    RISCV,
    LOONGARCH,
    s390x,
}

enum Os
{
    Linux,
    FreeBSD,
    MacOS,
    Windows,
}

struct Badge
{
const:

    Cpu cpu;
    Os os;
    BuildSystem build_system;

    string id()
    {
        return format("%d%c%d", cast(uint)(os) + 1, cast(char)('a' + cpu),
            cast(uint)(build_system));
    }

    string disabled_image_ref()
    {
        return format("[d%d]", cast(uint)(build_system));
    }

    string link_ref()
    {
        return format("[l%s]", id());
    }

    string image_ref()
    {
        return format("[i%s]", id());
    }

    bool enabled()
    {
        if (cpu == Cpu.LOONGARCH)
            return false;
        final switch (build_system)
        {
        case BuildSystem.CMake:
            return os == Os.Linux || cpu == Cpu.amd64;
        case BuildSystem.Bazel:
            return os == Os.Linux && (cpu == Cpu.amd64 || cpu == Cpu.AArch64);
        }
    }

    string text()
    {
        if (enabled())
            return format("[![]%s]%s", image_ref, link_ref);
        return format("![]%s", disabled_image_ref);
    }

    string append_logo(string url)
    {
        final switch (build_system)
        {
        case BuildSystem.CMake:
            return url ~ "&logo=cmake";
        case BuildSystem.Bazel:
            return url ~ "&logo=data:image/svg%2bxml;base64," ~ bazel_svg_base64;
        }
    }

    string disabled_image_link()
    {
        return append_logo(format("%s: https://img.shields.io/badge/n%%2Fa-lightgrey?",
                disabled_image_ref));
    }

    string filename()
    {
        import std.uni : toLower;

        return toLower(format("%s_%s_%s.yml", cpu, os, build_system));
    }

    string link_decl()
    {
        return format("%s: https://github.com/google/cpu_features/actions/workflows/%s",
            link_ref, filename());
    }

    string image_decl()
    {
        return append_logo(format("%s: https://img.shields.io/github/actions/workflow/status/google/cpu_features/%s?branch=main&event=push&label=",
                image_ref, filename()));
    }

}

auto tableHeader(in Cpu[] cpus)
{
    return chain(only("Os"), cpus.map!(to!string)).array;
}

auto tableAlignment(in Cpu[] cpus)
{
    return chain(only(":--"), cpus.map!(v => "--:")).array;
}

auto tableCell(Range)(in Os os, in Cpu cpu, Range badges)
{
    return badges.filter!(b => b.cpu == cpu && b.os == os)
        .map!(b => b.text())
        .joiner("<br/>").to!string;
}

auto tableRow(Range)(in Os os, in Cpu[] cpus, Range badges)
{
    return chain(only(os.to!string), cpus.map!(cpu => tableCell(os, cpu, badges))).array;
}

auto tableRows(Range)(in Os[] oses, in Cpu[] cpus, Range badges)
{
    return oses.map!(os => tableRow(os, cpus, badges)).array;
}

auto table(Range)(in Os[] oses, in Cpu[] cpus, Range badges)
{
    return chain(only(tableHeader(cpus)), only(tableAlignment(cpus)),
        tableRows(oses, cpus, badges));
}

void main()
{
    immutable allCpus = [EnumMembers!Cpu];
    immutable allOses = [EnumMembers!Os];
    immutable allBuildSystems = [EnumMembers!BuildSystem];
    auto badges = cartesianProduct(allCpus, allOses, allBuildSystems).map!(
        t => Badge(t[0], t[1], t[2]));
    writefln("%(|%-( %s |%) |\n%) |", table(allOses, allCpus, badges));
    writeln();
    badges.filter!(b => !b.enabled)
        .map!(b => b.disabled_image_link())
        .array
        .sort
        .uniq
        .each!writeln;
    badges.filter!(b => b.enabled)
        .map!(b => [b.link_decl(), b.image_decl()])
        .joiner().array.sort.uniq.each!writeln;
}
