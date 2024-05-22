namespace ClickJack.Extensions;

public static class Cli
{
    public static string GetValue(this string[] args, string substring)
    {
        if (args.Contains(substring))
            return args[args.GetIndex(substring) + 1];

        return "";

    }
    
    public static int GetIndex(this string[] args, string substring)
    {

        return args.ToList().FindIndex(a => a == substring);

    }

    public static bool Contains(this string[] args, string substring)
    {

        if (args.ToList().FindIndex(a => a == substring) != -1)
            return true;

        return false;
    }

}