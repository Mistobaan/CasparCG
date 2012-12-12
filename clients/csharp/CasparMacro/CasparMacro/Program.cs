using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;
using CasparRx;

namespace CasparMacro
{
    class Program
    {
        static void Main(string[] args)
        {
            using (Connection caspar = new Connection("localhost", 5250))
            {
                Task task = null;
                CancellationTokenSource token = null;

                for (var line = Console.ReadLine(); line != "q"; line = Console.ReadLine())
                {
                    if (task != null)
                    {
                        token.Cancel();
                        task.Wait();
                        token = null;
                        task = null;
                    }

                    if (line != "stop")
                    {
                        token = new CancellationTokenSource();
                        task = RunCommands(line, caspar, token);
                    }
                }
            }
        }

        static Task RunCommands(string path, Connection caspar, CancellationTokenSource token)
        {
            return Task.Factory.StartNew(() =>
            {
                try
                {
                    bool loop = true;

                    while (!token.IsCancellationRequested && loop)
                    {
                        loop = false;

                        using (var stream = new StreamReader(path, Encoding.UTF8))
                        {
                            while (!stream.EndOfStream && !token.IsCancellationRequested)
                            {
                                var line = stream.ReadLine().Trim();

                                Match match = Regex.Match(line, "#WAIT (\\d+)");
                                if (match.Success)
                                {
                                    var time = int.Parse(match.Groups[1].Value);
                                    for (int curTime = 0; curTime < time && !token.IsCancellationRequested; curTime += 500)
                                        Thread.Sleep(500);
                                }
                                else if (Regex.IsMatch(line, "^#LOOP"))
                                {
                                    loop = true;
                                    break;
                                }
                                else if (!String.IsNullOrWhiteSpace(line))
                                {
                                    caspar.Send(line);
                                }
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(String.Format("Oops, failed to execute schedule, {0}", ex.Message));
                }
            }, token.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
        }
    }
}
