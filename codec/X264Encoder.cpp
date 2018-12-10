#include "X264Encoder.h"
#include "General.h"

#pragma comment(lib,"libx264d")

x264Encoder::x264Encoder()
{

}

x264Encoder::~x264Encoder()
{

}

void x264Encoder::_SetFastParam(x264_param_t *param) 
{
//    x264_param_apply_profile(&param, x264_profile_names[0]);

	x264_param_parse(param, "subme", "1");
	x264_param_parse(param, "me", "dia");
	x264_param_parse(param, "merange", "16");
	x264_param_parse(param, "analyse", "none");
	x264_param_parse(param, "direct", "spatial");
	x264_param_parse(param, "pbratio", "1.5");
//	x264_param_parse(param, "bframes", "1");
}

bool x264Encoder::InitX264Encoder(unsigned short usWidth,unsigned short usHeight,int nFrameRate,int nBitRate ) 
{ 
	x264_param_t param; 
	x264_param_default(&param); 
	int ret;
	ret = x264_param_default_preset(&param, "ultrafast", NULL);

	param.i_width = usWidth; 
	param.i_height = usHeight; 
	param.i_csp = X264_CSP_I420;

//	param.i_log_level = X264_LOG_DEBUG; 

	//set bitrate
	param.rc.i_rc_method = X264_RC_ABR;//参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率); 
	param.rc.i_bitrate= nBitRate; 

	//set fps
	param.i_fps_den = 1;
	param.i_fps_num = nFrameRate;
	
	h = x264_encoder_open(&param); 
	if(h == NULL) 
	{ 
		return false; 
	} 

	ret = x264_picture_alloc(&m_pic_in, param.i_csp, usWidth, usHeight);
    x264_picture_init(&m_pic_out);

	m_usWidth=usWidth; 
	m_usHeight=usHeight; 
	return true; 
} 

int x264Encoder::encode_nals(unsigned char *buf, x264_nal_t *nals, int nnal) 
{ 
	unsigned char *p = buf; 

	x264_nal_encode(h, p, nals); 

	return nals->i_payload;//p - buf; 
} 

bool x264Encoder::GetHeaderFrame(unsigned char* pOutFrame,int& nOutLen, int frameType) 
{
	x264_nal_t *nals = NULL; 
	int nnal = 0;
	if( x264_encoder_headers( h, &nals, &nnal ) < 0 )
		return false; 

	int i = 0;
	for( ;i<nnal;i++ )
		if( nals[i].i_type == frameType )
		{
			nOutLen = nals[i].i_payload;
			memcpy(pOutFrame, nals[i].p_payload, nals[i].i_payload );

			//nOutLen = encode_nals(pOutFrame, &nals[i], 1); 

			if(nOutLen <= 0) 
				return false; 

			return true;
		}
		
	return false; 
}

bool x264Encoder::X264Encode(unsigned char* pInFrame, x264_nal_t **nals, int& nnal)
{
	if(pInFrame) 
	{ 
		m_pic_in.img.plane[0] = pInFrame; 
		m_pic_in.img.plane[1] = pInFrame + m_usWidth * m_usHeight; 
		m_pic_in.img.plane[2] = m_pic_in.img.plane[1] + (m_usWidth * m_usHeight / 4); 
		m_pic_in.i_pts = m_pts++;

		if(x264_encoder_encode(h, nals, &nnal, &m_pic_in, &m_pic_out) < 0) 
		{ 
			return false; 
		} 
	} 
	else 
	{ 
		if(x264_encoder_encode(h, nals, &nnal, NULL, &m_pic_out) < 0) 
		{ 
			return false; 
		} 
	} 

	if(nnal <= 0) 
		return false; 

	return true; 
}

void x264Encoder::ReleaseConnection() 
{ 
    x264_picture_clean(&m_pic_in);

	if(h) 
	{ 
		x264_encoder_close(h); 
		h = NULL; 
	} 
	delete this; 
} 

/* int testencoder(int argc, char* argv[]) 
{ 
	if (argc != 5) 
	{ 
		printf("please input: Enc_Demo.exe filename1[input] Width Height filename2[output]\n"); 
	} 


	//params set 
	unsigned short usWidth = atoi(argv[2]); 
	unsigned short usHeight = atoi(argv[3]); 

	//create X264 instance 
	x264Encoder* pX264enc = new x264Encoder(); 

	if(!pX264enc || !pX264enc->InitX264Encoder(usWidth, usHeight, 15, 1000) ) 
	{ 
		pX264enc->ReleaseConnection(); 
		delete pX264enc; 
		return -1; 
	} 

	unsigned char *p_In_Frame = new unsigned char[usWidth * usHeight * 3/2]; 
	unsigned char *p_Out_Frame = new unsigned char[usWidth * usHeight * 3/2]; 
	FILE* ifp = fopen(argv[1],"rb"); 
	FILE* ofp = fopen(argv[4],"wb"); 

	bool b_continue = true; 
	int nReadUnit = usWidth * usHeight * 3/2; 
	while (b_continue || !feof(ifp)) 
	{ 
		int n_OutFrame_Size  = 0; 
		bool bKeyFrame = false; 
		int nCount = fread(p_In_Frame, 1, nReadUnit, ifp); 
		if(nCount != nReadUnit) 
		{ 
			b_continue = false; 
			break; 
		} 

		unsigned char *pSrc = p_In_Frame; 
		if(pX264enc->X264Encode(pSrc, nCount, p_Out_Frame, n_OutFrame_Size,bKeyFrame)) 
		{ 
			fwrite(p_Out_Frame, n_OutFrame_Size, 1, ofp); 
		} 
	} 

	do 
	{ 
		int n_OutFrame_Size = 0; 
		bool b_KeyFrame = false; 
		if(pX264enc->X264Encode(NULL, 0, p_Out_Frame, n_OutFrame_Size, b_KeyFrame)) 
		{ 
			fwrite(p_Out_Frame, n_OutFrame_Size, 1, ofp); 
		} 
		else 
		{ 
			break; 
		} 
	}while(1); 

	//realse 
	delete []p_In_Frame; 
	delete []p_Out_Frame; 
	pX264enc->ReleaseConnection(); 
	fclose(ifp); 
	fclose(ofp); 

	return 0; 
} */