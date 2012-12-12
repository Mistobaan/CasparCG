/* Copyright (c) 2012 Robert Nagy and Peter Karlsson
*
* This file is part of CasparCG CircomDemo.
*
* CircomDemo is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CircomDemo is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CircomDemo. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy and Peter Karlsson
*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Text.RegularExpressions;
using System.Net.Mail;
using System.Net.Mime;
using System.Net;
using CasparRx;
using System.Reactive.Linq;
using System.Reactive.Threading;
using System.Reactive.Disposables;
using System.Reactive.Concurrency;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Reactive.Subjects;
using Settings = CircomDemo.Properties.Settings;

namespace CircomDemo
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        enum State
        {
            Idle,
            Recording,
            Playing
        };

        Connection              caspar          = new Connection(Settings.Default.CasparHost);
        string                  currentFilename = null;
        string                  lastFilename    = null;
        CancellationTokenSource cmdToken        = null;
        Task                    cmdTask         = null;
        BehaviorSubject<State>  stateSubject    = new BehaviorSubject<State>(State.Idle);
        
        public MainWindow()
        {
            InitializeComponent();

            this.stateSubject
                .ObserveOnDispatcher()
                .Subscribe(x =>
                {
                    if (!this.caspar.OnConnected.First())
                        return;

                    this.btnPlay.IsEnabled = x == State.Idle && !String.IsNullOrEmpty(this.lastFilename);
                    this.btnRecord.IsEnabled = x != State.Recording;
                    this.btnStop.IsEnabled = x != State.Idle;
                    this.btnEmail.IsEnabled = x != State.Recording && !String.IsNullOrEmpty(this.lastFilename);

                    if (x == State.Idle)
                    {
                        this.MediaEL.Stop();
                        if (!String.IsNullOrWhiteSpace(this.lastFilename))
                            this.MediaEL.Source = new Uri(Directory.GetCurrentDirectory() + "\\email.mov");
                        else
                            ;// this.MediaEL.Source = new Uri(Directory.GetCurrentDirectory() + "\\spacebar.mov");
                        this.MediaEL.Play();
                    }
                    else if (x == State.Recording)
                    {
                        this.MediaEL.Stop();
                        this.MediaEL.Source = new Uri(Directory.GetCurrentDirectory() + "\\onair.mov");
                        this.MediaEL.Play();
                    }
                });

            this.caspar.OnConnected
                .ObserveOnDispatcher()
                .Subscribe(x =>
                {
                    if (x)
                        Stop();
                    else
                    {
                        //this.caspar.Send("PLAY 1-1 DECKLINK DEVICE 2 FORMAT 1080i5000 BUFFER 1");
                        this.btnStop.IsEnabled   = false;
                        this.btnRecord.IsEnabled = false;
                        this.btnPlay.IsEnabled   = false;
                    }
                });

            this.MediaEL.MediaEnded += (object sender, RoutedEventArgs e) =>
                {
                    if(this.stateSubject.First() == State.Playing)
                        this.stateSubject.OnNext(State.Idle);
                };

            this.KeyDown += (object sender, KeyEventArgs e) =>
            {
                try
                {
                    if (e.Key == Key.Space)
                    {
                        if (this.stateSubject.First() == State.Idle)
                            this.Record(true);
                        else if (this.stateSubject.First() == State.Recording)
                            this.Stop();
                    }
                }
                catch
                {
                    MessageBox.Show("Oops, something went wrong.");
                }
            };

            //this.caspar.Send("PLAY 1-1 DECKLINK DEVICE 2 FORMAT 1080i5000 BUFFER 1");
            this.RunCommands("offair.txt");
        }
        
        private void btnRecord_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.Record(true);
            }
            catch
            {
                MessageBox.Show("Oops, something went wrong.");
            }
        }

        private void btnPlay_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.Play();
            }
            catch
            {
                MessageBox.Show("Oops, something went wrong.");
            }
        }

        private void btnStop_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.Stop();
            }
            catch
            {
                MessageBox.Show("Oops, something went wrong.");
            }
        }

        private void btnSendToMail_Click(object sender, RoutedEventArgs e)
        {
            var toAddress   = this.txtEmail.Text;
            var fileName    = this.lastFilename;

            this.lastFilename = String.Empty;
            this.stateSubject.OnNext(State.Idle);

            if (this.stateSubject.First() == State.Idle)
            {
                this.MediaEL.Stop();
                //this.MediaEL.Source = new Uri(Directory.GetCurrentDirectory() + "\\spacebar.mov");
                this.MediaEL.Play();
            }

            Task.Factory.StartNew(() =>
            {
                SendToMail(toAddress, fileName);
            }, TaskCreationOptions.LongRunning);
        }

        private void Play()
        {
            this.Record(false);

            if (String.IsNullOrEmpty(this.lastFilename))
                return;

            this.MediaEL.Source = new Uri(String.Format("{0}{1}", Properties.Settings.Default.MediaPath, this.lastFilename));
            this.MediaEL.Play();

            this.stateSubject.OnNext(State.Playing);
        }

        private void Stop()
        {
            if (this.MediaEL.Source != null)
            {
                this.MediaEL.Stop();
                this.MediaEL.Source = null;
            }
            
            this.Record(false);
            this.stateSubject.OnNext(State.Idle);
        }

        private void Record(bool value)
        {
            if (!value)
            {
                if (this.currentFilename != null)
                {
                    this.caspar.Send(String.Format("REMOVE 1 FILE {0}", this.currentFilename));
                    this.lastFilename = this.currentFilename;
                    this.currentFilename = null;
                    this.RunCommands("offair.txt");
                }
            }
            else
            {
                this.Record(false);

                if (this.MediaEL.Source != null)
                {
                    this.MediaEL.Stop();
                    this.MediaEL.Source = null;
                }
                
                for (int index = 0;
                     String.IsNullOrEmpty(this.currentFilename) ||
                        new FileInfo(String.Format("{0}{1}", Properties.Settings.Default.MediaPath, this.currentFilename)).Exists;
                     ++index)
                {
                    this.currentFilename = "circom" + index.ToString() + ".mp4";
                }

                this.RunCommands("onair.txt");
                
                this.stateSubject.OnNext(State.Recording);
            }
        }
        
        private void RunCommands(string path)
        {
            if (this.cmdToken != null)
            {
                this.cmdToken.Cancel();
                this.cmdTask.Wait();
                this.cmdToken = null;
                this.cmdTask = null;
            }

            var dispatcher = this.Dispatcher;
            var name = String.IsNullOrWhiteSpace(this.txtName.Text) ? "John Doe" : this.txtName.Text;

            this.cmdToken = new CancellationTokenSource();
            this.cmdTask = Task.Factory.StartNew(() =>
            {
                try
                {
                    bool loop = true;

                    while (!this.cmdToken.IsCancellationRequested && loop)
                    {
                        loop = false;

                        using (var stream = new StreamReader(path, Encoding.UTF8))
                        {
                            while (!stream.EndOfStream && !this.cmdToken.IsCancellationRequested)
                            {
                                var line = stream.ReadLine().Trim();
                                
                                Match match = Regex.Match(line, "#WAIT (\\d+)");
                                if (match.Success)
                                    this.Sleep(int.Parse(match.Groups[1].Value), this.cmdToken.Token);
                                else if (Regex.IsMatch(line, "^#LOOP"))
                                {
                                    loop = true;
                                    break;
                                }
                                else if (Regex.IsMatch(line, "^#STOP"))
                                {
                                    dispatcher.BeginInvoke(new Action(() => this.Stop()));
                                    break;
                                }
                                else if (Regex.IsMatch(line, "^#ADD"))
                                    this.caspar.Send(String.Format("ADD 1 FILE {0} -vcodec libx264 -preset ultrafast -crf 20", this.currentFilename));
                                else
                                {
                                    this.caspar.Send(Regex.Replace(line, "#NAME#", name));
                                }
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(String.Format("Oops, failed to execute schedule, {0}", ex.Message));
                }
            }, this.cmdToken.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
        }

        private void Sleep(int time, CancellationToken token)
        {
            for (int curTime = 0; curTime < time && !token.IsCancellationRequested; curTime += 500)
                Thread.Sleep(500);
        }

        private static void SendToMail(string toAddress, string fileName)
        {
            try
            {
                var filePath = String.Format("{0}{1}", Properties.Settings.Default.MediaPath, fileName);

                if (!new FileInfo(filePath).Exists)
                    throw new Exception("No recording found.");

                using (Process process = new Process())
                {
                    process.StartInfo.FileName = @"C:\\Program Files\\ffmpeg\\ffmpeg.exe";
                    process.StartInfo.Arguments             =
                        String.Format("-i {0} -vcodec libx264 -preset slow -crf 25 -r 30 -s 1024x576 -y -threads 0 {1}", 
                                      filePath, fileName);
                    process.StartInfo.UseShellExecute       = false;
                    process.StartInfo.RedirectStandardError = true;
                    process.StartInfo.CreateNoWindow        = true;
                    process.Start();

                    using (StreamReader reader = process.StandardError)
                    {
                        Debug.Write(reader.ReadToEnd());
                    }

                    process.WaitForExit();
                }

                var from = new MailAddress("demo@casparcg.com", "CasparCG");
                var to = new MailAddress(toAddress);

                var pass = String.Empty;
                using (var stream = new StreamReader("password.txt"))
                    pass = stream.ReadLine();

                var smtp = new SmtpClient
                {
                    
                    Host                    = "send.one.com",
                    Port                    = 2525,
                    EnableSsl               = true,
                    DeliveryMethod          = SmtpDeliveryMethod.Network,
                    UseDefaultCredentials   = false,
                    Credentials             = new NetworkCredential(from.Address, pass)
                };
                using (var message = new MailMessage(from, to)
                {
                    Subject = "CasparCG CIRCOM 2012 Recording",
                    Body = "Here is your recording from CIRCOM 2012."
                })
                {
                    Attachment data = new Attachment(fileName, MediaTypeNames.Application.Octet);
                    ContentDisposition disposition  = data.ContentDisposition;
                    disposition.CreationDate = System.IO.File.GetCreationTime(fileName);
                    disposition.ModificationDate = System.IO.File.GetLastWriteTime(fileName);
                    disposition.ReadDate = System.IO.File.GetLastAccessTime(fileName);
                    message.Attachments.Add(data);

                    smtp.Send(message);
                }

                Debug.WriteLine(String.Format("Sent recording {0} to {1}!", fileName, to.Address));
                //MessageBox.Show(String.Format("Sent recording {0} to {1}!", fileName, to.Address));
            }
            catch (Exception ex)
            {
                MessageBox.Show(String.Format("Oops, failed to send mail to {0}, {1}", toAddress, ex.Message));
            }
        }
    }
}
