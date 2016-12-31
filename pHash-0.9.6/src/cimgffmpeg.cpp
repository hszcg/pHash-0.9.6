/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    D Grant Starkweather - dstarkweather@phash.org

*/

/* 
	Modified by Ivan Pizhenko <ivanp2015@users.noreply.github.com>
	https://github.com/ivanp2015/pHash-0.9.6
*/

#include "debug.h"
#include "cimgffmpeg.h"

void vfinfo_close(VFInfo  *vfinfo){
    if (vfinfo->pFormatCtx != NULL){
	avcodec_close(vfinfo->pCodecCtx);
	vfinfo->pCodecCtx = NULL;
	avformat_close_input(&vfinfo->pFormatCtx);
	vfinfo->pFormatCtx = NULL;
	vfinfo->width = -1;
	vfinfo->height = -1;
    }
}

int ReadFrames2(VFInfo *st_info, OnFrameCallabck callback, void* callback_data, unsigned int low_index, unsigned int hi_index)
{
        //target pixel format
	AVPixelFormat ffmpeg_pixfmt;
	if (st_info->pixelformat == 0)
	    ffmpeg_pixfmt = AV_PIX_FMT_GRAY8;
	else 
	    ffmpeg_pixfmt = AV_PIX_FMT_RGB24;

	st_info->next_index = low_index;

	if (st_info->pFormatCtx == NULL){
	    st_info->current_index= 0;

        av_log_set_level(AV_LOG_QUIET);
	av_register_all();

	debug_printf(("Opening file %s\n", st_info->filename));
	
	    // Open video file
	    if(avformat_open_input(&st_info->pFormatCtx, st_info->filename, NULL, NULL)!=0)
		return -1 ; // Couldn't open file
		
	debug_printf(("File %s opened.\n", st_info->filename));
	 
	    // Retrieve stream information
	    if(avformat_find_stream_info(st_info->pFormatCtx,NULL)<0) { 
		    avformat_close_input(&st_info->pFormatCtx);
		return -1; // Couldn't find stream information
	    }
	
	    //dump_format(pFormatCtx,0,NULL,0);//debugging function to print infomation about format
	debug_printf(("Stream info found.\n"));
	
	    unsigned int i;
	    // Find the video stream
	    for(i=0; i<st_info->pFormatCtx->nb_streams; i++)
	    {
		if(st_info->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
	        {
		    st_info->videoStream=i;
		    break;
		}
	    }
	    if(st_info->videoStream==-1) {
		avformat_close_input(&st_info->pFormatCtx);
		return -1; //no video stream
	    }

	    debug_printf(("Video Stream index %d\n", st_info->videoStream));
	
	
	    // Get a pointer to the codec context for the video stream
	    st_info->pCodecCtx = st_info->pFormatCtx->streams[st_info->videoStream]->codec;
	    if (st_info->pCodecCtx == NULL){
		debug_printf(("There is no codec data\n"));
		avformat_close_input(&st_info->pFormatCtx);
		return -1;
	    }

	    // Find the decoder
	    st_info->pCodec = avcodec_find_decoder(st_info->pCodecCtx->codec_id);
	    if(st_info->pCodec==NULL) 
	    {
		debug_printf(("Decoder not found\n"));
		avformat_close_input(&st_info->pFormatCtx);
	  	return -1 ; // Codec not found
	    }
	    // Open codec
	    if(avcodec_open2(st_info->pCodecCtx, st_info->pCodec,NULL)<0) {
		debug_printf(("Failed to open codec\n"));
		avformat_close_input(&st_info->pFormatCtx);
		return -1; // Could not open codec
	    }
	    
	    st_info->height = (st_info->height<=0) ? st_info->pCodecCtx->height : st_info->height;
	    st_info->width  = (st_info->width<= 0) ? st_info->pCodecCtx->width : st_info->width;
	}

        AVFrame *pFrame;

	// Allocate video frame
	//pFrame=avcodec_alloc_frame();
	pFrame = av_frame_alloc();
	if (pFrame==NULL)
	    return -1;

	// Allocate an AVFrame structure
	//AVFrame *pConvertedFrame = avcodec_alloc_frame();
	AVFrame *pConvertedFrame = av_frame_alloc();
	if(pConvertedFrame==NULL) {
		  vfinfo_close(st_info);
		return -1;
	}
	uint8_t *buffer;
	int numBytes;
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(ffmpeg_pixfmt, st_info->width,st_info->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	if (buffer == NULL) {
	    vfinfo_close(st_info);
	    return -1;
	}

	avpicture_fill((AVPicture *)pConvertedFrame,buffer,ffmpeg_pixfmt,st_info->width,st_info->height);
		
	int frameFinished;
	int size = 0;
	

        int channels = ffmpeg_pixfmt == AV_PIX_FMT_GRAY8 ? 1 : 3;

	AVPacket packet;
	int result = 1;
	CImg<uint8_t> next_image;
	SwsContext *c = sws_getContext(st_info->pCodecCtx->width, st_info->pCodecCtx->height, st_info->pCodecCtx->pix_fmt, st_info->width, st_info->height, ffmpeg_pixfmt , SWS_BICUBIC, NULL, NULL, NULL);
	while ((result>=0)&&(size<st_info->nb_retrieval)&&(st_info->current_index<=hi_index)){  
	  result =  av_read_frame(st_info->pFormatCtx, &packet);
          if (result < 0)
	      break;
    	  if(packet.stream_index==st_info->videoStream) {

		AVPacket avpkt; 
		av_init_packet(&avpkt); 
		avpkt.data = packet.data; 
		avpkt.size = packet.size; 
		// 
		// HACK for CorePNG to decode as normal PNG by default 
		// same method used by ffmpeg 
		avpkt.flags = AV_PKT_FLAG_KEY; 
	      
	 	avcodec_decode_video2(st_info->pCodecCtx, pFrame, &frameFinished,&avpkt);

	      // avcodec_decode_video(st_info->pCodecCtx, pFrame, &frameFinished,packet.data, packet.size);

	      if(frameFinished) {
		  if (st_info->current_index == st_info->next_index){
		      st_info->next_index += st_info->step;
		      sws_scale(c, pFrame->data, pFrame->linesize, 0, st_info->pCodecCtx->height, pConvertedFrame->data, pConvertedFrame->linesize);
			
		  next_image.assign(*pConvertedFrame->data, channels,st_info->width,st_info->height,1,true);
		  next_image.permute_axes("yzcx");
		  callback(callback_data, next_image);
		  size++;
		  }    
		  st_info->current_index++;
	      }
	      av_free_packet(&packet);
	  }
	}


	if (result < 0){
	    avcodec_close(st_info->pCodecCtx);
	    avformat_close_input(&st_info->pFormatCtx);
	    st_info->pFormatCtx = NULL;
	    st_info->pCodecCtx = NULL;
	    st_info->width = -1;
	    st_info->height = -1;
	}

	av_free(buffer);
	buffer = NULL;
	av_free(pConvertedFrame);
	pConvertedFrame = NULL;
	av_free(pFrame);
	pFrame = NULL;
	sws_freeContext(c);
	c = NULL;

	return size; 
}


int NextFrames2(VFInfo *st_info, OnFrameCallabck callback, void* callback_data)
{
        AVPixelFormat ffmpeg_pixfmt;
	if (st_info->pixelformat == 0)
	    ffmpeg_pixfmt = AV_PIX_FMT_GRAY8;
        else 
	    ffmpeg_pixfmt = AV_PIX_FMT_RGB24;

	if (st_info->pFormatCtx == NULL)
	{
	        st_info->current_index = 0;
		st_info->next_index = 0;
		av_register_all();
		st_info->videoStream = -1;
		//st_info->pFormatCtx = (AVFormatContext*)malloc(sizeof(AVFormatContext));
		//st_info->pCodecCtx = (AVCodecContext*)malloc(sizeof(AVCodecContext));
		//st_info->pCodec = (AVCodec*)malloc(sizeof(AVCodec));

#ifdef DEBUG
		av_log_set_level(AV_LOG_DEBUG);
#else
		av_log_set_level(AV_LOG_QUIET);
#endif

		debug_printf(("Opening file %s\n", st_info->filename));
		
		// Open video file
 		if(avformat_open_input(&st_info->pFormatCtx,st_info->filename,NULL,NULL)!=0){
			return -1 ; // Couldn't open file
		}

		debug_printf(("Opened file %s\n", st_info->filename));

		// Retrieve stream information
		if(avformat_find_stream_info(st_info->pFormatCtx,NULL)<0){
			debug_printf(("Stream info not found"));
			avformat_close_input(&st_info->pFormatCtx);
			return -1; // Couldn't find stream information
		}

		unsigned int i;

		// Find the video stream
		for(i=0; i< st_info->pFormatCtx->nb_streams; i++)
		{
			if(st_info->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
			{
				st_info->videoStream=i;
				break;
			}
		}

		debug_printf(("Video Stream index %d\n", st_info->videoStream));

		if(st_info->videoStream==-1){
			debug_printf(("Video stream not found"));
			avformat_close_input(&st_info->pFormatCtx);
		    return -1; //no video stream
		}
	
		// Get a pointer to the codec context for the video stream
		st_info->pCodecCtx = st_info->pFormatCtx->streams[st_info->videoStream]->codec;
		if(st_info->pCodecCtx == NULL) {
			debug_printf(("There is no codec info\n"));
			avformat_close_input(&st_info->pFormatCtx);
			return -1 ; // Codec not found
			
		}
		
		union {
			unsigned u;
			char s[5];
		} z;
		z.u = st_info->pCodecCtx->codec_tag;
		z.s[4] = 0;
		debug_printf(("Codec tag: %x %s\n", z.u, z.s));

		// Find the decoder
		st_info->pCodec = avcodec_find_decoder(st_info->pCodecCtx->codec_id);
		if(st_info->pCodec==NULL) 
		{
			debug_printf(("Decoder not found\n"));
			avformat_close_input(&st_info->pFormatCtx);
			return -1 ; // Codec not found
		}
		// Open codec
		if(avcodec_open2(st_info->pCodecCtx, st_info->pCodec,NULL)<0){
			debug_printf(("Can't open codec\n"));
			avformat_close_input(&st_info->pFormatCtx);
		    return -1; // Could not open codec
		}
		
		st_info->width = (st_info->width<=0) ? st_info->pCodecCtx->width : st_info->width;
		st_info->height = (st_info->height<=0) ? st_info->pCodecCtx->height : st_info->height;
		
	} else {
		debug_printf(("File already opened %s\n", st_info->filename));
	}
	
	AVFrame *pFrame;

	// Allocate video frame
	//pFrame=avcodec_alloc_frame();
	pFrame=av_frame_alloc();
		
	// Allocate an AVFrame structure
//	AVFrame *pConvertedFrame = avcodec_alloc_frame();
	AVFrame *pConvertedFrame = av_frame_alloc();
	if(pConvertedFrame==NULL){
		debug_printf(("Can't allocate frame\n"));
		vfinfo_close(st_info);
	  return -1;
	}
		
	uint8_t *buffer;
	int numBytes;
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(ffmpeg_pixfmt, st_info->width,st_info->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	if (buffer == NULL){
		debug_printf(("Can't allocate picture buffer\n"));
		av_free(pConvertedFrame);
		vfinfo_close(st_info);
	    return -1;
	}

	avpicture_fill((AVPicture *)pConvertedFrame,buffer,ffmpeg_pixfmt,st_info->width,st_info->height);
		
	int frameFinished;
	int size = 0;
	AVPacket packet;
	int result = 1;
	CImg<uint8_t> next_image;
	SwsContext *c = sws_getContext(st_info->pCodecCtx->width, st_info->pCodecCtx->height, st_info->pCodecCtx->pix_fmt, st_info->width, st_info->height, ffmpeg_pixfmt , SWS_BICUBIC, NULL, NULL, NULL);
	while ((result >= 0) && (size < st_info->nb_retrieval))
	{
		result = av_read_frame(st_info->pFormatCtx, &packet); 
		if (result < 0) {
			debug_printf(("Can't read frame\n"));
			break;
		}
		if(packet.stream_index == st_info->videoStream) {
			
		int channels = ffmpeg_pixfmt == AV_PIX_FMT_GRAY8 ? 1 : 3;
 		AVPacket avpkt;
                av_init_packet(&avpkt);
                avpkt.data = packet.data;
                avpkt.size = packet.size;
                //
                // HACK for CorePNG to decode as normal PNG by default
                // same method used by ffmpeg
                avpkt.flags = AV_PKT_FLAG_KEY;

                avcodec_decode_video2(st_info->pCodecCtx, pFrame, &frameFinished,&avpkt);

		   // avcodec_decode_video(st_info->pCodecCtx, pFrame, &frameFinished,
		   //                      packet.data,packet.size);
 
		    if(frameFinished) {
		    	if (st_info->current_index == st_info->next_index)
		    	{
			    st_info->next_index += st_info->step;
			   
			    sws_scale(c, pFrame->data, pFrame->linesize, 0, st_info->pCodecCtx->height, pConvertedFrame->data, pConvertedFrame->linesize);
				   	
				next_image.assign(*pConvertedFrame->data, channels, st_info->width,st_info->height,1,true);
				next_image.permute_axes("yzcx");
				callback(callback_data, next_image);
				size++;
				   	 
		    	}    
				st_info->current_index++;
		    }
    	  }
    	        av_free_packet(&packet);
	}
	
	av_free(buffer);
	buffer = NULL;
	av_free(pConvertedFrame);
	pConvertedFrame = NULL;
	av_free(pFrame);
	pFrame = NULL;
	sws_freeContext(c);
	c = NULL;
	if (result < 0)
	{
		avcodec_close(st_info->pCodecCtx);
		avformat_close_input(&st_info->pFormatCtx);
		st_info->pCodecCtx = NULL;
		st_info->pFormatCtx = NULL;
		st_info->pCodec = NULL;
		st_info->width = -1;
		st_info->height = -1;
	}
	return size; 
}

int GetNumberStreams(const char *file)
{
	 AVFormatContext *pFormatCtx = NULL;
	 av_log_set_level(AV_LOG_QUIET);
	 av_register_all();
	// Open video file
	if (avformat_open_input(&pFormatCtx, file, NULL, NULL))
	  return -1 ; // Couldn't open file
		 
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	  return -1; // Couldn't find stream information
	int result = pFormatCtx->nb_streams;
	avformat_close_input(&pFormatCtx);
	return result;
}

long GetNumberVideoFrames(const char *file)
{
    long nb_frames = 0L;
	AVFormatContext *pFormatCtx = NULL;
    av_log_set_level(AV_LOG_QUIET);
	av_register_all();
	// Open video file
	debug_printf(("Opening file %s\n", file));
	if (avformat_open_input(&pFormatCtx, file, NULL, NULL))
	  return -1 ; // Couldn't open file
	debug_printf(("Opened file %s\n", file));
			 
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
	  return -1; // Couldn't find stream information
	debug_printf(("Found stream info\n"));
		
	// Find the first video stream
	int videoStream=-1;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++)
	{
	     if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
	     {
		    videoStream=i;
		    break;
             }
	}
	if(videoStream==-1)
	{
	   debug_printf(("Video stream not found\n"));
	   avformat_close_input(&pFormatCtx); 
	   return -1; // Didn't find a video stream
	}
	debug_printf(("Video stream index %d\n", videoStream));
	AVStream *str = pFormatCtx->streams[videoStream];
		
        nb_frames = str->nb_frames;
	if (nb_frames > 0)
	{   //the easy way if value is already contained in struct
		avformat_close_input(&pFormatCtx);
		debug_printf(("Easy nb_frames available: %ld\n", nb_frames));
		return nb_frames;
	}
	else { // frames must be counted
		//AVPacket packet;
		nb_frames = (long)av_index_search_timestamp(str,str->duration, AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD);
		debug_printf(("timestamp by index: %ld\n", nb_frames));
		if (nb_frames <= 0) {
			debug_printf(("str->duration: %ld\n", str->duration));
			if (str->duration > 0) {
				int timebase = str->time_base.den / str->time_base.num;
				nb_frames = str->duration/timebase;
			} else {
				debug_printf(("pFormatCtx->duration: %ld\n", pFormatCtx->duration));
				debug_printf(("str->avg_frame_rate: %d/%d\n", 
					str->avg_frame_rate.num, str->avg_frame_rate.den));
				if(pFormatCtx->duration > 0) {
					nb_frames = (((long double)pFormatCtx->duration/AV_TIME_BASE) 
						* str->avg_frame_rate.num) / str->avg_frame_rate.den;
				}
			}
		}
		// Close the video file
		avformat_close_input(&pFormatCtx); 
		debug_printf(("Counted nb_frames: %ld\n", nb_frames));
		return nb_frames;
	}
}

float fps(const char *filename)
{
        float result = 0;
	AVFormatContext *pFormatCtx = NULL;
	
	debug_printf(("Opening file %s\n", filename));
	// Open video file
	if (avformat_open_input(&pFormatCtx, filename, NULL, NULL))
	  return -1 ; // Couldn't open file
	debug_printf(("Opened file %s\n", filename));
				 
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx,NULL)<0) {
	  avformat_close_input(&pFormatCtx);
	  return -1; // Couldn't find stream information
	}
	debug_printf(("Found stream info\n"));
			
	// Find the first video stream
	int videoStream=-1;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++)
	{
		     if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) 
		     {
			    videoStream=i;
			    break;
		     }
	}
        if(videoStream==-1) {
	   debug_printf(("Video stream not found\n"));
	   avformat_close_input(&pFormatCtx); 
	    return -1; // Didn't find a video stream
	}

	debug_printf(("Video stream index %d\n", videoStream));
	AVStream *str = pFormatCtx->streams[videoStream];
	
#if 0
	int num = str->r_frame_rate.num;
	int den = str->r_frame_rate.den;
	debug_printf(("str->r_frame_rate: num=%d, den=%d\n", num, den));
#else
	int num = 0;
	int den = 0;
#endif
	result = den == 0 ? 0.0f : (float)num/den;
	debug_printf(("Current FPS=%f\n", result));
	if (result == 0.0f) {
		num = str->avg_frame_rate.num;
		den = str->avg_frame_rate.den;		
		debug_printf(("str->avg_frame_rate: num=%d, den=%d\n", num, den));
		result = den == 0 ? 0.0f : (float)num/den;
		debug_printf(("Current FPS=%f\n", result));
	}
	if (result == 0.0f) {
		num = str->codec->time_base.num;
		den = str->codec->time_base.den;		
		debug_printf(("str->codec->time_base: num=%d, den=%d\n", num, den));
		// fps = 1.0 / timebase
		result = num == 0 ? 0.0f : (float)den/num;
		debug_printf(("Current FPS=%f\n", result));
	}
	if (result == 0.0f) {
		result = -1;
	}

	avformat_close_input(&pFormatCtx);

	debug_printf(("Final FPS=%f\n", result));
	return result;

}
