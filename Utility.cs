using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace UtilityFunctions
{
	public static class Utility
	{
		// ソース：
		// 【.NET】テキストファイルの末尾からn行を読み込む
		// https://qiita.com/yaju/items/cdc261a7e228a914d754

		public static string Tail( string path, int lines = 1, string encoding = "UTF-8" )
		{
			int BUFFER_SIZE = 32;       // バッファーサイズ(あえて小さく設定)
			int offset = 0;
			int loc = 0;
			int foundCount = 0;
			var buffer = new byte[BUFFER_SIZE];
			bool isFirst = true;
			bool isFound = false;

			// ファイル共有モードで開く
			using ( var fs = new FileStream( path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite ) )
			{
				// 検索ブロック位置の繰り返し
				for ( int i = 0; ; i++ )
				{
					// ブロック開始位置に移動
					offset = Math.Min( (int)fs.Length, ( i + 1 ) * BUFFER_SIZE );
					loc = 0;
					if ( fs.Length <= i * BUFFER_SIZE )
					{
						// ファイルの先頭まで達した場合
						if ( foundCount > 0 || fs.Length > 0 ) break;

						// 行が未存在
						throw new ArgumentOutOfRangeException( "NOT FOUND DATA" );
					}

					fs.Seek( -offset, SeekOrigin.End );

					// ブロックの読み込み
					int readLength = offset - BUFFER_SIZE * i;
					for ( int j = 0; j < readLength; j += fs.Read( buffer, j, readLength - j ) ) ;

					// ブロック内の改行コードの検索
					for ( int k = readLength - 1; k >= 0; k-- )
					{
						if ( buffer[k] == 0x0A )
						{
							if ( isFirst && k == readLength - 1 ) continue;
							if ( ++foundCount == lines )
							{
								// 所定の行数が見つかった場合
								loc = k + 1;
								isFound = true;
								break;
							}
						}
					}
					isFirst = false;
					if ( isFound ) break;
				}

				// 見つかった場合
				fs.Seek( -offset + loc, SeekOrigin.End );

				using ( var sr = new StreamReader( fs, Encoding.GetEncoding( encoding ) ) )
				{
					return sr.ReadToEnd();
				}
			}
		}
	}
}
