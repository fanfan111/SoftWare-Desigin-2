#pragma once

// ��ȡͼƬ(���ļ���)
BOOL LoadBitmapFromFile(const CString strPathFile, CBitmap &bitmap, CSize &size);
// ��ȡͼƬ(����Դ��)
BOOL LoadBitmapFromIDResource(UINT nID, CBitmap &bitmap, CSize &size, CString strType);
// ��ȡͼƬ(���ڴ��м���)
BOOL LoadBitmapFromMem(BYTE* pByte, DWORD dwSize, CBitmap &bitmap, CSize &size);
// ��ȡͼ��(���ļ���)
BOOL LoadIconFromFile(CString strPathFile, HICON &hIcon);
// ��ȡͼ��(����Դ��)
BOOL LoadIconFromIDResource(UINT nID, HICON &hIcon);
// ��ȡͼ��(���ڴ��м���)
BOOL LoadIconFromMem(BYTE* pByte, DWORD dwSize, HICON &hIcon);
// ����ͼƬ�ļ����ڴ���
BOOL LoadImageFromFile(CString strFile, BOOL useEmbeddedColorManagement, Image*& pImg);
// ����Դ�м���ͼƬ
BOOL LoadImageFromIDResource(UINT nID, CString strType, BOOL useEmbeddedColorManagement, Image*& pImg);
// ���ڴ��м���ͼƬ�ļ�
BOOL LoadImageFromMem(BYTE* pByte, DWORD dwSize, BOOL useEmbeddedColorManagement, Image*& pImg);

// ȡ��ͼƬƽ����ɫ
BOOL GetAverageColor(CDC *pDC, CBitmap &bitmap, const CSize &sizeImage, COLORREF &clrImage);
// ȡ��ͼƬ��С
BOOL GetSize(CBitmap &bitmap, CSize &size);
// ȡ�������С
Size GetTextBounds(const Gdiplus::Font& font,const StringFormat& strFormat,const CString& strText);
// ȡ�������С
Size GetTextBounds(const Gdiplus::Font& font,const StringFormat& strFormat, int nWidth, const CString& strText);
// ȡ�������С
Size GetTextBounds(const Gdiplus::Font& font,const CString& strText);
// ȡ��λ��
CPoint GetOriginPoint(int nWidth, int nHeight, int nChildWidth, int nChildHeight, UINT uAlignment = DT_CENTER, UINT uVAlignment = DT_VCENTER);
// ȡ��λ��
CPoint GetOriginPoint(CRect rc, int nChildWidth, int nChildHeight, UINT uAlignment = DT_CENTER, UINT uVAlignment = DT_VCENTER);
// ת������
CString DecimalFormat(int nNumber);


// �滭����

// ����ֱ����
int DrawVerticalTransition(CDC &dcDes, CDC &dcSrc, const CRect &rcDes, const CRect &rcSrc, int nBeginTransparent = 0, int nEndTransparent = 100);
// ��ˮƽ����
int DrawHorizontalTransition(CDC &dcDes, CDC &dcSrc, const CRect &rcDes, const CRect &rcSrc, int nBeginTransparent = 0, int nEndTransparent = 100);
// �����½ǹ���
void DrawRightBottomTransition(CDC &dc, CDC &dcTemp, CRect rc, const int nOverRegio, const COLORREF clrBackground);
// ��ͼƬ�߿�
void DrawImageFrame(Graphics &graphics, Image *pImage, const CRect &rcControl, int nX, int nY, int nW, int nH, int nFrameSide = 4);
// ��ͼƬ�߿�(��ָ���Ź����λ��,nX/nY��ԭͼ�����Ͻ�,nWH/nHL�����Ͻǵ�����,nWR/nHR�����½ǵ�����)
void DrawImageFrameMID(Graphics &graphics, Image *pImage, const CRect &rcControl, int nX, int nY, int nW, int nH, int nWL, int nHL, int nWR, int nHR);
// ������Բ�Ǿ���
void DrawRectangle(CDC &dcDes, const CRect &rcDes, BOOL bUp = TRUE, int nBeginTransparent = 60, int nEndTransparent = 90);

// ����λͼ
HBITMAP	DuiCreateCompatibleBitmap(CRect& rcClient);
