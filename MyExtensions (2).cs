using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExtensionMethods
{
	public static class MyExtensions
	{
		public static string InsertSplitter( this string strSrc, int num, string strSplit, bool bReverse = false )
		{
			if ( strSrc == null || strSplit == null || num < 0 )
				return strSrc;

			StringBuilder sb = new StringBuilder();
			string ss = strSrc;
			int idx = 1;
			int length = ss.Length;

			if ( bReverse )
				ss = String.Join( "", ss.Reverse() );

			foreach ( char c in ss )
			{
				sb.Append( c );
				if ( idx % num == 0 && idx < length )
					sb.Append( strSplit );
				idx++;
			}

			ss = sb.ToString();
			if ( bReverse )
				ss = String.Join( "", ss.Reverse() );

			return ss;
		}
		public static string InsertSplitter( this string strSrc, int num, char chSplit, bool bReverse = false )
		{
			return InsertSplitter( strSrc, num, chSplit.ToString(), bReverse );
		}
	}
}
