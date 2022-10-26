//////////////////////////////////////////////////////////////////////////
// IImageWrapper.h : Interface of ImageWrapper 
//////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_IIMAGE_WRAPPER
#define _INCLUDE_IIMAGE_WRAPPER

#include <Unknwn.h>

//////////////////////////////////////////////////////////////////////////
//	Define Guid
//////////////////////////////////////////////////////////////////////////
// {f660fbc0-5936-11dd-ae16-0800200c9a66}
DEFINE_GUID( CLSID_IMAGE_WRAPPER, 
			0xf660fbc0, 0x5936, 0x11dd, 0xae, 0x16, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66);

// {ff7c86c0-5936-11dd-ae16-0800200c9a66}
DEFINE_GUID( IID_IMAGE_WRAPER, 
			0xff7c86c0, 0x5936, 0x11dd, 0xae, 0x16, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66);

// {6d8aeac6-aca4-4379-9e03-d45cad91a779}
DEFINE_GUID( IID_IMAGE_WRAPERCONFIG, 
			0x6d8aeac6, 0xaca4, 0x4379, 0x9e, 0x03, 0xd4, 0x5c, 0xad, 0x91, 0xa7, 0x79);

//////////////////////////////////////////////////////////////////////////
//	Define image type 
//////////////////////////////////////////////////////////////////////////
enum IMAGE_TYPE {
	IMAGE_TYPE_UNKNOWN = 0,	// All other types than the following supported
	//	types
	IMAGE_TYPE_GIF,
	IMAGE_TYPE_JPG,
	IMAGE_TYPE_BMP,
	IMAGE_TYPE_TIF,
	IMAGE_TYPE_PCX,
	IMAGE_TYPE_TGA,
	IMAGE_TYPE_PNG,
	IMAGE_TYPE_PCT,
	IMAGE_TYPE_EPS,
	IMAGE_TYPE_RAS,
	IMAGE_TYPE_PCD,
	IMAGE_TYPE_PSD,
	IMAGE_TYPE_MPO,
	IMAGE_TYPE_JPS
};

enum IMAGE_ORIENTATION {

	IMAGE_ORIENTATION_0_DEGREE =0,
	IMAGE_ORIENTATION_90_DEGREE,
	IMAGE_ORIENTATION_180_DEGREE,
	IMAGE_ORIENTATION_270_DEGREE
};
 
enum IMAGEWRAPPER_IMAGELAYOUTMODE

{
	LAYOUT_INTERLEAVED_LEFT_FIRST = 0,
    LAYOUT_INTERLEAVED_RIGHT_FIRST,
    LAYOUT_SIDEBYSIDE_LEFT_FIRST,
    LAYOUT_SIDEBYSIDE_RIGHT_FIRST,
    LAYOUT_OVERUNDER_LEFT_FIRST,
	LAYOUT_OVERUNDER_RIGHT_FIRST,
    LAYOUT_ANAGLYPH_LEFT_FIRST,
    LAYOUT_ANAGLYPH_RIGHT_FIRST,
	LAYOUT_UNKNOWN                // It means we would view it as 2D image 
};

// Show Left, Right, Side-by-side
enum IMAGEWRAPPER_DISPLAYMODE
{
	DM_LEFTONLY = 0,
	DM_RIGHTONLY,
	DM_SIDEBYSIDE
};

// MPO file type
enum IMAGEWRAPPER_MPOTYPE
{
	MPO_STEREO = 0,  // normal stereo image 
	MPO_PANORAMA     // panorama 
};

// Non-stereo image buffer mode 
enum IMAGEWRAPPER_FAKED3DBUFFERMODE
{
	FAKED3D_SIDEBYSIDE_LEFTFIRST = 0,  //  | L | R |
	FAKED3D_SIDEBYSIDE_RIGHTFIRST,     //  | R | L | 
    FAKED3D_TOPDOWN_LEFTFIRST,     
	                               // | L |
	                               // | R | 

	FAKED3D_TOPDOWN_RIGHTFIRST     // | R |
								   // | L | 
};

//////////////////////////////////////////////////////////////////////////
//	define image format for save file 
//////////////////////////////////////////////////////////////////////////
#define FILE_GIF                          2    // CompuServe GIF
#define FILE_BMP                          6    // Windows BMP
#define FILE_JFIF                         FILE_JPEG
#define FILE_JPEG                         10   // Jpeg File Interchange Format
#define FILE_PNG                          75

//////////////////////////////////////////////////////////////////////////
//	define Config Mode 
//////////////////////////////////////////////////////////////////////////
// Not using LeadTool in ImageWrapper 3.0 so this flag is disabled 
#define IW_MODE_ForceUseLeadTool			0x00	// Bool
#define IW_MODE_EnableImageCache		0x01	// Bool
#define IW_MODE_EnableAutoRotateByEXIF			0x02// Bool , Enable Auto rotated image buffer based on EXIF information. /// if enable, the width & height will be the rotated value.
#define IW_MODE_DisplayMode			            0x03// IMAGEWRAPPER_DISPLAYMODE, set the display mode  
#define IW_MODE_ImageLayOut				        0x04 // IMAGEWRAPPER_IMAGELAYOUTMODE  
// If it is unknown, it means we won't view it as 3D image. If it is not unknown, we'll view it as 3D image 
#define IW_MODE_NoneStereoBufferMode            0x05 // IMAGEWRAPPER_NONSTEREOBUFFERMODE, set the nonstereo buffer mode 

//////////////////////////////////////////////////////////////////////////
//	Image Wrapper Interface
//////////////////////////////////////////////////////////////////////////
interface	IImageWrapper: public IUnknown
{
public:
	//	return the file is image or not 
	//	if the file is image clip , will also fill in the image format informat . 
	virtual BOOL	IsImageClip(
		/*  [in] */ LPWSTR pwszFileName, 
		/*  [in] , [out] */ UINT* puImageWidth = NULL, 
		/*  [in] , [out] */ UINT* puImageHeight = NULL, 
		/*  [in] , [out] */ UINT* puImageBitCount = NULL, 
		/*  [in] , [out] */ UINT* puImageType = NULL, 
		/*  [in] , [out] */ SYSTEMTIME* pSystemTime = NULL)PURE;

	virtual HRESULT LoadImage(WCHAR* pwstrFileName)PURE;

	virtual HRESULT GetImageType(UINT *uImageType)PURE;

	virtual HRESULT GetImageBitCount(UINT* BitCount)PURE;

	virtual HRESULT GetImageHeight(UINT* Height)PURE;

	virtual HRESULT GetImageWidth(UINT* Width)PURE;

	virtual HRESULT GetOriginalDateTime(SYSTEMTIME *pSystemTime)PURE;

	// convert Hbitmap to image file 
	virtual HRESULT SaveImageFromHBitmap(
		/*  [in] */ HDC hDC, 
		/*  [in] */ HBITMAP hBitmap, 
		/*  [in] */ LPWSTR wfileName, 
		/*  [in] */ LONG format)PURE;

	// Convert the loaded image to an HBITMAP. 
	// Caller can configure the image size or default use source image size .
	// notes: the Hbitmap was create by ImageWrapper , but caller need to delete it after use it . 
	virtual HBITMAP GetDIBBitmap(
		/*  [in] */ int nWidth = 0 , 
		/*  [in] */ int nHeight = 0)PURE;

	// Convert the loaded image to an RGB32 buffer.
	// Caller should create the buffer.
	virtual HRESULT GetFrame(
		/*  [in] , [out] */ BYTE* pbyBuffer, 
		/*  [in] */ int nWidth , 
		/*  [in] */ int nHeight , 
		/*  [in] */ int nStride)PURE;
	
	// Unload Current Image 
	virtual HRESULT UnloadImage()PURE;

	virtual HRESULT	GetImageOrientation(IMAGE_ORIENTATION *orientation)PURE;

	// Convert the loaded image to an RGB32 buffer.
	// Caller should create the buffer.
	// ImageWrapper will try to get thumbnail from EXIF information as first priority.
	virtual HRESULT GetThumbnailFrame(
		/*  [in] , [out] */ BYTE* pbyBuffer, 
		/*  [in] */ int nWidth , 
		/*  [in] */ int nHeight , 
		/*  [in] */ int nStride)PURE;
 
	virtual HRESULT SaveMPO(WCHAR* pwstrFileName,PBYTE SrcBuffer,INT nBufferW, INT nBufferH, INT nStride)PURE;
    
	// pwstrFileName ¡G MPO abstract filename which contains the folder path 
	// SrcBuffer ¡GSide_by_side buffer 
    // nBufferW¡GSide_by_side buffer width
    // nBufferH¡GSide_by_side buffer height
    // nStride¡GSide_by_side buffer stride

	virtual HRESULT SaveJPS(WCHAR* pwstrFileName, PBYTE SrcBuffer, INT nBufferW, INT nBufferH, INT nStride)PURE;

	virtual HRESULT SetMPORightFirst(BOOL bRightFirst)PURE;
	virtual IMAGEWRAPPER_MPOTYPE GetMPOFileType()PURE; // Get the mpo file type to let MO decide the width and height 
};


interface	IImageWrapperConfig: public IUnknown
{
public:
		virtual	HRESULT	GetConfig(DWORD dwMode, LPVOID pPointer)PURE;
		virtual	HRESULT	SetConfig(DWORD dwMode, LPVOID pPointer)PURE;
};

#endif
