import std.algorithm : each, map, cartesianProduct, filter;
import std.array : array;
import std.conv : to;
import std.format;
import std.range : chain, only;
import std.stdio;
import std.traits : EnumMembers;

enum Cpu
{
    amd64,
    AArch64,
    ARM,
    MIPS
}

enum Os
{
    FreeBSD,
    Linux,
    MacOS,
    Windows
}

string getWorkflowName(Cpu cpu)
{
    final switch (cpu)
    {
    case Cpu.amd64:
        return "amd64";
    case Cpu.AArch64:
        return "aarch64";
    case Cpu.ARM:
        return "arm";
    case Cpu.MIPS:
        return "mips";
    }
}

string getWorkflowName(Os os)
{
    final switch (os)
    {
    case Os.FreeBSD:
        return "FreeBSD";
    case Os.Linux:
        return "Linux";
    case Os.MacOS:
        return "macOS";
    case Os.Windows:
        return "Windows";
    }
}

struct Configuration
{
const:
    Cpu cpu;
    Os os;

    bool disabled()
    {
        import std.algorithm.comparison : among;

        return os.among(Os.FreeBSD, Os.Windows, Os.MacOS) && cpu.among(Cpu.AArch64, Cpu.ARM, Cpu
                .MIPS);
    }

    string cell()
    {
        return format("%d%c", cast(uint)(os) + 1, cast(char)('a' + cpu));
    }

    string link_ref()
    {
        return format("[lnk_%s]", cell());
    }

    string image_ref()
    {
        return format("[img_%s]", cell());
    }

    string cell_text()
    {
        if (disabled())
            return "![][img_na]";
        return format("[![]%s]%s", image_ref, link_ref);
    }

}

immutable allCpus = [EnumMembers!Cpu];
immutable allOses = [EnumMembers!Os];

auto tableHeader(in Cpu[] cpus)
{
    return chain(only("Os"), cpus.map!(to!string)).array;
}

auto tableAlignment(in Cpu[] cpus)
{
    return chain(only(":--"), cpus.map!(v => "--:")).array;
}

auto tableRow(in Os os, in Cpu[] cpus)
{
    return chain(only(os.to!string), cpus.map!(cpu => Configuration(cpu, os).cell_text())).array;
}

auto tableRows(in Os[] oses, in Cpu[] cpus)
{
    return oses.map!(os => tableRow(os, cpus)).array;
}

auto table(in Os[] oses, in Cpu[] cpus)
{
    return chain(only(tableHeader(cpus)), only(tableAlignment(cpus)), tableRows(oses, cpus));
}

void main()
{
    immutable allCpus = [EnumMembers!Cpu];
    immutable allOses = [EnumMembers!Os];

    writefln("%(|%-( %-21s |%) |\n%) |", table(allOses, allCpus));
    writeln();
    writeln("[img_na]: https://img.shields.io/badge/build-N%2FA-lightgrey");
    cartesianProduct(allCpus, allOses)
        .map!(t => Configuration(t[0], t[1]))
        .filter!(conf => !conf.disabled)
        .each!((conf) {
            const cpu_name = getWorkflowName(conf.cpu);
            const os_name = getWorkflowName(conf.os);
            writefln("%s: https://github.com/google/cpu_features/actions/workflows/%s_%s.yml", conf.link_ref, cpu_name, os_name);
            writefln("%s: https://img.shields.io/github/workflow/status/google/cpu_features/%s%%20%s/main", conf
                .image_ref, cpu_name, os_name);
        });
}
