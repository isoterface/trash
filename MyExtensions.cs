using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExtensionMethods
{
	/// <summary>
	/// 拡張メソッド群
	/// </summary>
	public static class MyExtensions
	{
		/// <summary>
		/// 文字列に指定桁ごとに区切り文字を追加
		/// </summary>
		/// <param name="str">対象文字列(this)</param>
		/// <param name="num">区切る桁数</param>
		/// <param name="splitstr">区切り文字列</param>
		/// <returns>追加後の文字列</returns>
		public static string InsertSplitter(this string str, int num, string splitstr)
		{
			if (str == null || splitstr == null || num < 0)
			{
				return str;
			}

			StringBuilder sb = new StringBuilder();
			int idx = 1;
			int length = str.Length;

			foreach (char c in str)
			{
				sb.Append(c);
				if (idx % num == 0 && idx < length)
				{
					sb.Append(splitstr);
				}
				idx++;
			}
			return sb.ToString();
		}
		/// <summary>
		/// 文字列に指定桁ごとに区切り文字を追加
		/// </summary>
		/// <param name="str">対象文字列</param>
		/// <param name="num">区切る桁数</param>
		/// <param name="splitchar">区切り文字</param>
		/// <returns>追加後の文字列</returns>
		public static string InsertSplitter(this string str, int num, char splitchar)
		{
			if (str == null || num < 0)
			{
				return str;
			}

			StringBuilder sb = new StringBuilder();
			int idx = 1;
			int length = str.Length;

			foreach (char c in str)
			{
				sb.Append(c);
				if (idx % num == 0 && idx < length)
				{
					sb.Append(splitchar);
				}
				idx++;
			}
			return sb.ToString();
		}
	}
}
