using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MyUtil
{
	class ConsoleOutNetworkStream
	{
		public static void main()
		{
			System.Net.Sockets.Socket sock = new System.Net.Sockets.Socket(
				System.Net.Sockets.AddressFamily.InterNetwork,
				System.Net.Sockets.SocketType.Stream,
				System.Net.Sockets.ProtocolType.Tcp);
			System.Net.IPEndPoint ipEndPt = new System.Net.IPEndPoint(
				new System.Net.IPAddress(new byte[] { 127, 0, 0, 1 }), 20000);
			sock.Connect(ipEndPt);

			if (!sock.Connected)
				Console.WriteLine("Not Connected");

			System.Net.Sockets.NetworkStream ns = new System.Net.Sockets.NetworkStream(sock);

			System.IO.StreamWriter sw = new System.IO.StreamWriter(ns);
			sw.AutoFlush = true;		// trueにしないと出力が即時反映されない
			System.IO.TextWriter tw = System.IO.TextWriter.Synchronized(sw);
			Console.SetOut(tw);

			Console.WriteLine("Start");
			Console.WriteLine("Test");
			//sw.WriteLine("Test");
			Console.WriteLine("Test2");
			//sw.WriteLine("Test");
			Console.WriteLine("Test3");
			//sw.WriteLine("Test3");

			while (true)
			{
				string dt = DateTime.Now.ToString("yyyyMMdd HH:mm:ss.fff");
				Console.WriteLine(dt);
				//sw.WriteLine(dt);
				//sw.Flush();
				//byte[] buff = System.Text.Encoding.ASCII.GetBytes(dt + Environment.NewLine);
				//ns.Write(buff, 0, buff.Length);
				System.Threading.Thread.Sleep(1000);
			}
		}
	}
}
