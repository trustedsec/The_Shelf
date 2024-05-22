using System;
using System.IO;
using System.Net.Http;
using System.Threading.Tasks;
using ClickJack.Extensions;
using Newtonsoft.Json.Linq;

namespace ClickJack.Modules;

class GrayhatApi
{
    static readonly HttpClient client = new HttpClient();
    
    public static async Task DownloadGrayhatLinks(string[] args)
    {
        int start = 0;
        int limit = 1000;
        string authToken = args.GetValue("--apitoken");
        string outfile = args.GetValue("--outfile");
        if (authToken == "" || outfile == "")
        {
            Console.WriteLine("--apitoken is required and --outfile is required");
            Environment.Exit(1);
        }
        string query = ".application";
        
        client.DefaultRequestHeaders.Add("Authorization", $"Bearer {authToken}");

        while (true)
        {
            string response = null;

            for (int attempt = 0; attempt < 10; attempt++)
            {
                try
                {
                    response = await GetResponseAsync($"https://buckets.grayhatwarfare.com/api/v2/files?keywords={query}&limit={limit}&start={start}");
                    break;  // Break the loop if the request is successful
                }
                catch (Exception ex) when (ex is HttpRequestException || ex is TaskCanceledException)
                {
                    Console.WriteLine($"Attempt {attempt + 1} failed. Retrying in 15 seconds...");
                    await Task.Delay(TimeSpan.FromSeconds(15));  // Wait for 15 seconds before retrying
                }
            }

            if (response == null)
            {
                Console.WriteLine("Failed to get a response after 10 attempts. Exiting...");
                return;
            }

            JObject jsonResponse = JObject.Parse(response);
            
            if (jsonResponse["files"].HasValues)
            {
                using (StreamWriter file = File.AppendText("urls.txt"))
                {
                    foreach (var item in jsonResponse["files"])
                    {
                        string url = item["url"].ToString();
                        if (url.EndsWith(".application"))
                        {
                            await file.WriteLineAsync(url);
                        }
                    }
                }
                start += limit;
                Console.Write(".");
            }
            else
            {
                break;
            }
        }
    }

    static async Task<string> GetResponseAsync(string url)
    {
        HttpResponseMessage response = await client.GetAsync(url);

        response.EnsureSuccessStatusCode();

        return await response.Content.ReadAsStringAsync();
    }
}