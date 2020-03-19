using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace ImageTransform
{
	public static class ImageTransform
	{
		#region ===<形状変形処理>===

		/// <summary>
		/// 形状変形パラメータ
		/// </summary>
		private struct TfParam
		{
			public double a;
			public double b;
			public double c;
			public double d;
			public double e;
			public double f;
			public double g;
			public double h;
		}


		/// <summary>
		/// BitmapDataのバイト配列コピー
		/// </summary>
		private class BitmapByte
		{
			private Bitmap SrcImg;
			private BitmapData Data;
			public byte[] PixelBytes;
			public int Width;
			public int Height;
			public int Stride;
			public int DepthByte;

			/// <summary>
			/// BitmapDateを使用開始、バイト配列にデータをコピー
			/// </summary>
			/// <param name="bitmap">ビットマップデータ</param>
			/// <param name="lockMode">ReadOnly/WriteOnly/ReadWrite</param>
			/// <param name="pixelFmt">Format32bppPArgb/Format32bppArgb/Format24bppRgb</param>
			public void LockBitmap(Bitmap bitmap, ImageLockMode lockMode, PixelFormat pixelFmt)
			{
				try
				{
					SrcImg = bitmap;
					Width = SrcImg.Width;
					Height = SrcImg.Height;
					Data = SrcImg.LockBits(new Rectangle(0, 0, Width, Height), lockMode, pixelFmt);
					Stride = Data.Stride;
					DepthByte = ((pixelFmt == PixelFormat.Format32bppArgb) || (pixelFmt == PixelFormat.Format32bppPArgb) ? (4) : (3));

					PixelBytes = new byte[Stride * Height];
					Marshal.Copy(Data.Scan0, PixelBytes, 0, PixelBytes.Length);
				}
				catch (Exception ex)
				{
					throw ex;
				}
			}
			public void LockBitmap(Bitmap bitmap, ImageLockMode lockMode)
			{
				LockBitmap(bitmap, lockMode, bitmap.PixelFormat);
			}


			/// <summary>
			/// BitmapDataを使用終了
			/// </summary>
			public void UnlockBitmap()
			{
				try
				{
					SrcImg.UnlockBits(Data);
				}
				catch (Exception ex)
				{
					throw ex;
				}
			}

			/// <summary>
			/// バイト配列よりBitmapデータにコピー
			/// </summary>
			public void MarshalCopy()
			{
				try
				{
					Marshal.Copy(PixelBytes, 0, Data.Scan0, PixelBytes.Length);
				}
				catch (Exception ex)
				{
					throw ex;
				}
			}
		}


		/// <summary>
		/// 変形処理
		/// </summary>
		/// <param name="srcImg">元画像</param>
		/// <param name="destImg">[out] 変形後画像</param>
		/// <param name="pt">変形後画像の頂点情報</param>
		/// <returns>true:成功, false:失敗</returns>
		public static bool Transform(Bitmap srcImg, out Bitmap destImg, List<Point> pt)
		{
			System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
			sw.Reset();
			sw.Start();

			destImg = null;
			BitmapByte srcByte = new BitmapByte();
			BitmapByte destByte = new BitmapByte();

			Color pxCol = Color.FromArgb(0, 0, 0, 0);
			Color bgCol = Color.FromArgb(0, 0, 0, 0);
			TfParam p = new TfParam();
			int calcWidth, calcHeight;
			int destWidth, destHeight;
			double xa, ya, xb, yb, xc, yc, xd, yd;
			double s, t;        // 対象画素の元画像に対する比率 (0 <= s,t <=1)
			double tx, ty;      // 変形後の座標

			try
			{
				destWidth = pt[0].X;
				destHeight = pt[0].Y; ;
				for (int i = 1; i < pt.Count; i++)
				{
					if (destWidth < pt[i].X) destWidth = pt[i].X;
					if (destHeight < pt[i].Y) destHeight = pt[i].Y;
				}
				destWidth++;
				destHeight++;

				destImg = new Bitmap(destWidth, destHeight);
				destByte.LockBitmap(destImg, ImageLockMode.WriteOnly);

				srcByte.LockBitmap(srcImg, ImageLockMode.ReadOnly);
				calcWidth = srcByte.Width - 1;
				calcHeight = srcByte.Height - 1;

				// 頂点データは反時計回りだが、パラメータには時計回りでセットする
				xa = pt[0].X; ya = pt[0].Y;
				xb = pt[3].X; yb = pt[3].Y;
				xc = pt[2].X; yc = pt[2].Y;
				xd = pt[1].X; yd = pt[1].Y;
				p.a = (xc - xd - xb + xa);
				p.b = (xb - xa);
				p.c = (xd - xa);
				p.f = (yc - yd - yb + ya);
				p.g = (yb - ya);
				p.h = (yd - ya);

				for (int y = 0; y < destHeight; y++)
				{
					for (int x = 0; x < destWidth; x++)
					{
						p.d = (x - xa);
						p.e = (y - ya);

						if (ResolveEqu(p, out s, out t) == false)
						{
							SetPixelByte(destByte, x, y, bgCol);
							continue;
						}

						//if ((0 <= s && s <= 1.0) && (0 <= t && t <= 1.0))
						//{
						tx = s * calcWidth;
						ty = t * calcHeight;
						//}

						if (Interpolate(srcByte, tx, ty, ref pxCol))
						{
							SetPixelByte(destByte, x, y, pxCol);
						}
						else
						{
							SetPixelByte(destByte, x, y, bgCol);
						}
					}
				}

				destByte.MarshalCopy();
			}
			catch (Exception ex)
			{
				return false;
			}
			finally
			{
				try
				{
					if (srcByte != null) srcByte.UnlockBitmap();
				}
				finally
				{
					if (destByte != null) destByte.UnlockBitmap();
				}

				sw.Stop();
				Console.WriteLine("Transform time: {0}", sw.ElapsedMilliseconds);
			}

			return true;
		}


		/// <summary>
		/// BitmapDataのバイト配列よりピクセル情報を取得
		/// </summary>
		/// <param name="bmpByte">BitmapDataのバイト配列情報</param>
		/// <param name="x">対象のx座標</param>
		/// <param name="y">対象のy座標</param>
		/// <returns>対象ピクセルの色情報</returns>
		private static Color GetPixelByte(BitmapByte bmpByte, int x, int y)
		{
			int pos = x * bmpByte.DepthByte + bmpByte.Stride * y;
			byte b = bmpByte.PixelBytes[pos + 0];
			byte g = bmpByte.PixelBytes[pos + 1];
			byte r = bmpByte.PixelBytes[pos + 2];
			byte a = 255;
			if (bmpByte.DepthByte == 4)
			{
				a = bmpByte.PixelBytes[pos + 3];
			}
			return Color.FromArgb(a, r, g, b);
		}


		/// <summary>
		/// BitmapDataのバイト配列対象のピクセルに色情報を設定
		/// </summary>
		/// <param name="bmpByte">BitmapDataのバイト配列情報</param>
		/// <param name="x">対象のx座標</param>
		/// <param name="y">対象のy座標</param>
		/// <param name="col">設定する色情報</param>
		private static void SetPixelByte(BitmapByte bmpByte, int x, int y, Color col)
		{
			int pos = x * bmpByte.DepthByte + bmpByte.Stride * y;
			bmpByte.PixelBytes[pos + 0] = col.B;
			bmpByte.PixelBytes[pos + 1] = col.G;
			bmpByte.PixelBytes[pos + 2] = col.R;
			if (bmpByte.DepthByte == 4)
			{
				bmpByte.PixelBytes[pos + 3] = col.A;
			}
		}


		/// <summary>
		///	比率 s,t を求める
		/// </summary>
		/// <param name="p">形状変形パラメータ</param>
		/// <param name="s">[out]比率(横方向)</param>
		/// <param name="t">[out]比率(縦方向)</param>
		/// <returns>true:範囲内, false:範囲外</returns>
		private static bool ResolveEqu(TfParam p, out double s, out double t)
		{
			s = -1.0;
			t = -1.0;

			double sa = -1.0;
			double ta = -1.0;
			double sb = -1.0;
			double tb = -1.0;

			double p1 = (p.a * p.h - p.f * p.c);
			double p2 = (p.d * p.f - p.c * p.g + p.b * p.h - p.a * p.e);
			double p3 = (p.d * p.g - p.b * p.e);
			double tm;

			if (p1 == 0.0)
			{
				s = (p.d / p.b);
				t = (p.b * p.e - p.d * p.g) / (p.b * p.h + p.d * p.f);
				//return true;
			}
			else if (Resolve2Equ(p1, p2, p3, out ta, out tb))
			{
				tm = (p.a * ta + p.b);
				if (tm != 0.0)
				{
					sa = (p.d - p.c * ta) / tm;
				}

				tm = (p.a * tb + p.b);
				if (tm != 0.0)
				{
					sb = (p.d - p.c * tb) / tm;
				}

				if ((0 <= sa && sa <= 1.0) && (0 <= ta && ta <= 1.0))
				{
					s = sa;
					t = ta;
				}
				else if ((0 <= sb && sb <= 1.0) && (0 <= tb && tb <= 1.0))
				{
					s = sb;
					t = tb;
				}
			}

			return (s != -1.0 && t != -1.0);
		}


		/// <summary>
		/// 比率 t を求める
		/// </summary>
		/// <param name="a">変形パラメータ</param>
		/// <param name="b">変形パラメータ</param>
		/// <param name="c">変形パラメータ</param>
		/// <param name="t1">[out]比率t(1)</param>
		/// <param name="t2">[out]比率t(2)</param>
		/// <returns>true:計算成功, false:計算不可</returns>
		private static bool Resolve2Equ(double a, double b, double c, out double t1, out double t2)
		{
			t1 = -1.0;
			t2 = -1.0;
			double d;

			if (a == 0.0)
			{
				if (c == 0.0)
				{
					return false;       // 計算不可
				}
				t1 = b / c;
				t2 = b / c;
				return true;
			}

			d = b * b - 4.0 * a * c;
			if (d < 0.0)
			{
				return false;       // 計算不可
			}

			t1 = (-b + Math.Sqrt(d)) / (2.0 * a);
			t2 = (-b - Math.Sqrt(d)) / (2.0 * a);
			return true;
		}


		/// <summary>
		/// 補完した値を求める
		/// </summary>
		/// <param name="bmpByte">BitmapDataのバイト配列</param>
		/// <param name="x">変形後の位置 x</param>
		/// <param name="y">変形後の位置 y</param>
		/// <param name="pxCol">求めた色情報</param>
		/// <returns>true:補完成功, false:対象座標は形状外部</returns>
		private static bool Interpolate(BitmapByte bmpByte, double x, double y, ref Color pxCol)
		{
			int x0 = (int)(x + 0.5);
			int y0 = (int)(y + 0.5);
			if (x0 < 0 || bmpByte.Width <= x0 || y0 < 0 || bmpByte.Height <= y0)
			{
				return false;       // 対象座標は形状外部
			}
			pxCol = GetPixelByte(bmpByte, x0, y0);
			return true;
		}

		#endregion


		/// <summary>
		/// 指定サイズのグリッドパターン画像を作成
		/// </summary>
		/// <param name="width">横ピクセル数</param>
		/// <param name="height">縦ピクセル数</param>
		/// <param name="splitw">横分割数</param>
		/// <param name="splith">縦分割数</param>
		/// <param name="colFrame">グリッド色</param>
		/// <param name="colBack">背景色</param>
		/// <returns></returns>
		public static Bitmap CreateGridPattern(int width, int height, int splitw, int splith, Color colFrame, Color colBack)
		{
			Bitmap img = new Bitmap(width, height);
			int partw = width / splitw;
			int parth = height / splith;
			using (Graphics g = Graphics.FromImage(img))
			{
				g.Clear(colBack);
				Pen pen = new Pen(colFrame, 2);
				for (int y = 0; y < splith; y++)
				{
					for (int x = 0; x < splitw; x++)
					{
						g.DrawRectangle(pen, new Rectangle(x * partw, y * parth, partw, parth));
					}
				}
				pen.Dispose();
			}
			return img;
		}


		/// <summary>
		/// 矩形領域内の座標を縦横の比率に対して求める
		/// </summary>
		/// <param name="listPt">矩形領域の座標データリスト</param>
		/// <param name="s">領域内の横方向比率(左始点で 0～1.0)</param>
		/// <param name="t">領域内の縦方向比率(上視点で 0～1.0)</param>
		/// <param name="point">求められたピクセル位置情報</param>
		/// <returns>true:成功, false:失敗</returns>
		public static bool GetRectPoint(List<Point> listPt, double s, double t, out Point point, bool isLocal = false)
		{
			point = new Point();

			if (listPt == null || listPt.Count < 4)
			{
				return false;
			}
			if (s < 0 || 1.0 < s || t < 0 || 1.0 < t)
			{
				return false;
			}

			if (isLocal)
			{
				// 外接矩形の左上座標を(0,0)にする
				Point ptTopLeft;
				List<Point> listLocal = PanelShapeData.GetLocalVertex(listPt, out ptTopLeft);
				listPt.Clear();
				foreach (var pt in listLocal)
				{
					listPt.Add(pt);
				}
			}

			double xa, xb, xc, xd, ya, yb, yc, yd;

			xa = listPt[0].X; ya = listPt[0].Y;
			xb = listPt[1].X; yb = listPt[1].Y;
			xc = listPt[2].X; yc = listPt[2].Y;
			xd = listPt[3].X; yd = listPt[3].Y;

			point.X = (int)((xc - xd - xb + xa) * s * t + (xd - xa) * s + (xb - xa) * t + xa);
			point.Y = (int)((yc - yd - yb + ya) * s * t + (yd - ya) * s + (yb - ya) * t + ya);

			return true;
		}
	}
}
