#ifndef _D3DUTIL_H_
#define _D3DUTIL_H_
#include <Windows.h>
#include <d3d11.h>

const float ANGLETORADIAN = 0.01745329278f;

#define SAFE_RELEASE(x)				{if(x) (x)->Release(); (x) = NULL;}


extern BOOL CreateDIBSection32(CONST int	nWidth, CONST int	nHeight,	//	might be negative
								HBITMAP*	phBmp, void**		ppRGBABuf);

extern BYTE* DumpTexture2DData(ID3D11Device* pD3dDevice,
			  				   ID3D11DeviceContext *p_D3dDeviceContext,
							   ID3D11Texture2D *p_SourceTexture);

extern HRESULT DumpTexture2DDatatoScreen(ID3D11Device* pD3dDevice,
										 ID3D11DeviceContext *p_D3dDeviceContext,
										 ID3D11Texture2D *p_SourceTexture);

extern HRESULT CopyBufferToTexture(ID3D11DeviceContext *p_d3dDeviceContext,
								   unsigned char *p_buffer,
								   UINT bytes_per_row,
								   UINT nums_of_rows,
								   ID3D11Texture2D *p_targetTexture);

extern HRESULT CreateRGBATexture2D(ID3D11Device* pD3dDevice, UINT unTextureWidth, UINT unTextureHeight, UINT unBindFlag, UINT unCPUAccessFlag,
								   DXGI_FORMAT fmt, D3D11_USAGE unUsage, ID3D11Texture2D** ppNewTexture, ID3D11ShaderResourceView** ppTextureShaderView);
#endif