#include "D3DUtil.h"
#include <tchar.h>
#include <stdio.h>

BOOL CreateDIBSection32(
	CONST int	nWidth,
	CONST int	nHeight,	//	might be negative
	HBITMAP*	phBmp,
	void**		ppRGBABuf)
{
	if(nWidth==0 || nHeight==0)
		return NULL;

	BITMAPINFO	bmi;

	HDC hdcScreen = CreateDC(_T("DISPLAY"), (LPCTSTR)NULL, (LPCTSTR)NULL, (DEVMODE*)NULL);

	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			= nWidth;
	bmi.bmiHeader.biHeight			= nHeight;
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biBitCount		= 32;
	bmi.bmiHeader.biCompression		= BI_RGB;
	bmi.bmiHeader.biSizeImage		=	bmi.bmiHeader.biWidth *
										-nHeight *
										bmi.bmiHeader.biBitCount / 8;
	bmi.bmiHeader.biXPelsPerMeter	= 100;
	bmi.bmiHeader.biYPelsPerMeter	= 100;
	bmi.bmiHeader.biClrUsed			= 0;
	bmi.bmiHeader.biClrImportant	= 0;

	*phBmp =
		CreateDIBSection(hdcScreen,
						 &bmi,
						 DIB_RGB_COLORS,
						 (VOID**)ppRGBABuf,
						 NULL,
						 0);
	if (*phBmp == NULL) {
		return FALSE;
	}

	if (hdcScreen != NULL) ::DeleteDC(hdcScreen);

	return TRUE;
}

BYTE* DumpTexture2DData(ID3D11Device* pD3dDevice,
	  				   ID3D11DeviceContext *p_D3dDeviceContext,
					   ID3D11Texture2D *p_SourceTexture)
{
	HRESULT hr = S_OK;
	BOOL bCreateNewTexture = FALSE;
	ID3D11Texture2D* pSysTexture = NULL;

	D3D11_TEXTURE2D_DESC desc;
	p_SourceTexture->GetDesc(&desc);

	if((desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) == 0)	{
		D3D11_TEXTURE2D_DESC sysdesc;
		memcpy(&sysdesc, &desc, sizeof(D3D11_TEXTURE2D_DESC));

		sysdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		sysdesc.Usage = D3D11_USAGE_STAGING;
		sysdesc.BindFlags = 0;
		sysdesc.ArraySize = 1;

		hr = pD3dDevice->CreateTexture2D(&sysdesc, nullptr, &pSysTexture);
		if(FAILED(hr))
			return NULL;

		p_D3dDeviceContext->CopyResource(pSysTexture, p_SourceTexture);
		bCreateNewTexture = TRUE;
	} else {
		pSysTexture = p_SourceTexture;
	}

	BYTE* pDestBuffer = new BYTE[desc.Width * desc.Height * 4];
	UINT unStride = desc.Width * 4;

	// Map texture to D3D subresource
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	hr = p_D3dDeviceContext->Map(pSysTexture,
								 0,
								 D3D11_MAP_READ,
								 0, 
								 &mappedSubresource );

	if(SUCCEEDED(hr))
	{
		// Copy the mapped data to output buffer
		if(mappedSubresource.pData != nullptr)
		{
			// The sub resource data's row length must be power of 2.
			// We should only copy valid data from sub resource's buffer 
			// instead of copying whole buffer
			unsigned char *pDst = (unsigned char*)pDestBuffer;
			unsigned char *pSrc = (unsigned char*)mappedSubresource.pData;
			if (unStride == mappedSubresource.RowPitch)
			{
				memcpy(pDst, pSrc, unStride * desc.Height);
			}
			else
			{
				for (UINT h = 0 ; h < desc.Height; h++)
				{
					memcpy(pDst, pSrc, mappedSubresource.RowPitch);
					pDst += unStride;
					pSrc += mappedSubresource.RowPitch;
				}
			}
		}

		// unmap resource
		p_D3dDeviceContext->Unmap(pSysTexture, 0);
	}

	if(bCreateNewTexture)
		pSysTexture->Release();

	return pDestBuffer;
}

HRESULT DumpTexture2DDatatoScreen(ID3D11Device* pD3dDevice,
								  ID3D11DeviceContext *p_D3dDeviceContext,
								  ID3D11Texture2D *p_SourceTexture)
{
	D3D11_TEXTURE2D_DESC desc;
	p_SourceTexture->GetDesc(&desc);
	UINT unStride = desc.Width * 4;

	BYTE* pDestBuffer = DumpTexture2DData(pD3dDevice, p_D3dDeviceContext, p_SourceTexture);

	// blit it to screen
	HBITMAP	hbmpDst;
	BYTE* pDestBuf = NULL;
	int nHeight = desc.Height;
	CreateDIBSection32(desc.Width, -nHeight, &hbmpDst, (void**)&pDestBuf);
	memcpy(pDestBuf, pDestBuffer, desc.Height * unStride);

	HDC hDC = ::GetDC(NULL);
	HDC hTempDC = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hTempDC, hbmpDst);
	::BitBlt(hDC, 0, 0, desc.Width, desc.Height, hTempDC, 0, 0, SRCCOPY);
	::SelectObject(hTempDC, hOldBmp);
	::DeleteObject(hbmpDst);
	::DeleteDC(hTempDC);
	::ReleaseDC(NULL, hDC);

	if(pDestBuffer)
		delete [] pDestBuffer;

	return S_OK;
}

HRESULT CopyBufferToTexture(ID3D11DeviceContext *p_d3dDeviceContext,
							unsigned char *p_buffer,
							UINT bytes_per_row,
							UINT nums_of_rows,
							ID3D11Texture2D *p_targetTexture)
{
	// Map texture data
	D3D11_MAPPED_SUBRESOURCE mapResource;
	ZeroMemory(&mapResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	HRESULT hr = p_d3dDeviceContext->Map(
			p_targetTexture,
			0, 
			D3D11_MAP_WRITE_DISCARD, 
			0, 
			&mapResource
			);
	if(FAILED(hr))
		return hr;

	// Map color data to texture buffer
	unsigned char *p_texture_data = (unsigned char*)mapResource.pData;
	unsigned char *p_source_data = p_buffer;
	if (bytes_per_row == mapResource.RowPitch)
	{
		memcpy(p_texture_data, p_source_data, nums_of_rows * bytes_per_row);
	}
	else
	{
		// bug fixed : http://ecl/Ebug/EbugHandle/HandleMainEbug.asp?BugCode=ACD151218-0018
		// Intel VGA + Win10 it shows uncorrect YUY2 result while upload texture data.
		// Since its RowPitch is larger than original size.
		D3D11_TEXTURE2D_DESC tex2DDesc;
		p_targetTexture->GetDesc(&tex2DDesc);
		

		if(tex2DDesc.Format == DXGI_FORMAT_YUY2 && tex2DDesc.Width * 4 == mapResource.RowPitch)
		{
			for (UINT h = 0; h < nums_of_rows; h ++)
			{
				for(int t = 0; t < bytes_per_row / 2; t++)
				{
					p_texture_data[4 * t] = p_source_data[2 * t];
					p_texture_data[4 * t + 1] = p_source_data[2 * t + 1];
				}
				
				p_texture_data += mapResource.RowPitch;
				p_source_data += bytes_per_row;
			}
		}
		else
		{
			for (UINT h = 0; h < nums_of_rows; h ++)
			{
				memcpy(p_texture_data, p_source_data, bytes_per_row);
				p_texture_data += mapResource.RowPitch;
				p_source_data += bytes_per_row;
			}
		}
	}

	p_d3dDeviceContext->Unmap(p_targetTexture, 0);
	return S_OK;
}

HRESULT CreateRGBATexture2D(ID3D11Device* pD3dDevice, UINT unTextureWidth, UINT unTextureHeight, UINT unBindFlag, UINT unCPUAccessFlag,
							DXGI_FORMAT fmt, D3D11_USAGE unUsage, ID3D11Texture2D** ppNewTexture, ID3D11ShaderResourceView** ppTextureShaderView)
{
	if(pD3dDevice == NULL)
		return E_INVALIDARG;

	if(ppNewTexture == NULL)
		return E_INVALIDARG;

	D3D11_TEXTURE2D_DESC tex2DDesc;
	ZeroMemory( &tex2DDesc, sizeof( tex2DDesc ) );
	tex2DDesc.ArraySize = 1;
	tex2DDesc.BindFlags = unBindFlag;
	tex2DDesc.Usage = unUsage;
	tex2DDesc.CPUAccessFlags = unCPUAccessFlag;
	tex2DDesc.Format = fmt;
	tex2DDesc.Width = unTextureWidth;
	tex2DDesc.Height = unTextureHeight;
	tex2DDesc.MipLevels = 1;
	tex2DDesc.SampleDesc.Count = 1;
				
	HRESULT hr = pD3dDevice->CreateTexture2D( &tex2DDesc, NULL, &(*ppNewTexture)) ;
	if(FAILED(hr)){
		return hr;
	}

	if(ppTextureShaderView != NULL){
		hr = pD3dDevice->CreateShaderResourceView(*ppNewTexture, NULL, &(*ppTextureShaderView));
		if(FAILED(hr)){
			return hr;
		}
	}

	return S_OK;
}