#include <stdio.h>

#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>

const char *src_filename=NULL;
const char *video_dst_filename=NULL;
const char *audio_dst_filename=NULL;


FILE *pOutputAudio = NULL;
FILE *pOutputVideo = NULL;

AVFormatContext *fmt_ctx=NULL;

AVStream *st = NULL;
AVCodec *dec = NULL;
AVCodecContext *dec_ctx = NULL;

//分别定义音视频流文件
AVStream *videoStream = NULL, *audioStream = NULL;
AVCodecContext *videoDecCtx = NULL, *audioDecCtx = NULL;

int frame_width = 0,frame_height = 0;
enum AVPixelFormat pix_fmt;

unsigned char *video_dst_data[4];
int video_dst_linesize[4];
int video_dst_bufsize;

AVFrame *frame=NULL;
AVPacket packet;

static int open_codec_context(enum AVMediaType type);
int decode_packet(int *got_frame);

int main(int argc, char **argv)
{
	int ret, got_frame=0;

	if ( argc == 4 )
	{
	 	src_filename = argv[1];
		video_dst_filename = argv[2];
		audio_dst_filename = argv[3];

		printf("Demuxing %s to %s and %s.\n",src_filename,
			video_dst_filename,audio_dst_filename);
	}
	else
	{
		printf("Error:command line format.\n");
		return -1;
	}

	//=====FFmpeg相关初始化=====
	av_register_all();

	/*
	打开待处理的音视频文件，探测读取文件格式信息保存到ftm_ctx中，
	同时读取多媒体文件信息，根据视音频流创建AVstream	
	*/
	if ( avformat_open_input(&fmt_ctx,src_filename, NULL,NULL) < 0)
	{
	 	printf("Error:open input file failed\n");
		return -1;	
	}

	/*
	读取一部分视音频数据并且获得一些相关的信息,主要给每个媒体流的AVStream结构体赋值	
	*/
	if ( avformat_find_stream_info(fmt_ctx,NULL) < 0)
	{
		printf("Error:find stream info failed\n");
		return -1;
	}
	
	//=====打开音频和视频流=====
	if ( open_codec_context(AVMEDIA_TYPE_VIDEO) >=0 )
	{
	 	videoStream = st;
		videoDecCtx = dec_ctx;

		frame_width = videoDecCtx->width;
		frame_height = videoDecCtx->height;
		pix_fmt = videoDecCtx->pix_fmt;

		/*
		分配像素的存储空间
		该函数的四个参数分别表示AVFrame结构中的缓存指针、各个颜色分量的宽度、图像分辨率（宽、高）、像素格式和内存对其的大小。
		该函数会返回分配的内存的大小。其实就是对video_dst_data,video_dst_linesize内存进行分配
		*/
		ret = av_image_alloc(video_dst_data,video_dst_linesize,frame_width,frame_height,pix_fmt,1);
		if ( ret < 0 )
		{
		 	printf("Error: Raw video buffer allocation failed.\n");
			return -1;
		}
		else
		{
			video_dst_bufsize = ret;
			printf("av image alloc size:%d\n",video_dst_bufsize);
		}

		if ( !(pOutputVideo = fopen(video_dst_filename, "wb")) )
		{
		 	printf("Error: opening output yuv file failed.\n");
			return -1;
		}
			
	}

	/*
	查找对应type流，并初始化设置解码器上下文
	*/
	if ( open_codec_context(AVMEDIA_TYPE_AUDIO) >=0 )
	{
		audioStream = st;
		audioDecCtx = dec_ctx;

		if ( !(pOutputAudio = fopen(audio_dst_filename, "wb")) )
		{
		 	printf("Error: Opening output aac file failed.\n");
			return -1;
		}
	}
	
	//=====打印输入输出格式信息====
	av_dump_format(fmt_ctx, 0, src_filename, 0);

	//=====读取和处理音视频文件=====
	frame = av_frame_alloc();
	if (!frame)
	{
		printf("alloc frame fail!\nn");
		return -1;
	}
	
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;

	/*
	读取压缩包	
	*/
	while (av_read_frame(fmt_ctx, &packet) >= 0)
	{
	    do
	    {
	    	/*
	    	解码视音频流
		*/
	     	ret = decode_packet(&got_frame);
		printf("decode packet size:%d\n",ret);	
		packet.data += ret;
		packet.size -= ret;
	    } while (packet.size > 0);
	}

	/*
	将编码器缓存中的数据解码完
	*/
	packet.data = NULL;
	packet.size = 0;
	do
	 {
	     	ret = decode_packet(&got_frame);
		packet.data += ret;
		packet.size -= ret;
	    } while (got_frame);

	
	//=====释放资源=====
	avcodec_close(videoDecCtx);
	avcodec_close(audioDecCtx);
	avformat_close_input(&fmt_ctx);
	av_frame_free(&frame);
	av_free(video_dst_data[0]);

	fclose(pOutputAudio);	
	fclose(pOutputVideo);	

	return 0;
}


int decode_packet(int *got_frame)
{
	int ret = 0;

	*got_frame = 0;

	if ( packet.stream_index == videoStream->index )
	{
		//视频流
		ret = avcodec_decode_video2(videoDecCtx,frame,got_frame, &packet);
		if ( ret < 0 )
		{
			printf("Error: decodec video frame failed\n");
			return -1;
		}

		if ( *got_frame )
		{
			printf("Decode 1 frame.\n");
			/* copy decoded frame to destination buffer:
			* this is required since rawvideo expects non aligned data */
			av_image_copy(video_dst_data,video_dst_linesize, (const unsigned char **)frame->data, frame->linesize,pix_fmt, frame_width, frame_height);
			
			fwrite(video_dst_data[0], 1, video_dst_bufsize, pOutputVideo);
		}
	}
	else if(packet.stream_index == audioStream->index)
	{
		//音频流		
		ret = avcodec_decode_audio4(audioDecCtx, frame, got_frame, &packet);
		if ( ret < 0 )
		{
			printf("Error: decodec audio frame failed\n");
			return -1;		
		}

		if ( *got_frame )
		{
			/*
			av_get_bytes_per_sample((enum AVSampleFormat)frame->format)返回的是采样位数16bits or 8 bits，此函数进行了右移
			操作，即除以8来获得字节数
			nb_samples指每个声道每一帧所包含的采样个数
			*/
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((enum AVSampleFormat)frame->format);

			/* Write the raw audio data samples of the first plane. This works
			* fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			* most audio decoders output planar audio, which uses a separate
			* plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			* In other words, this code will write only the first audio channel
			* in these cases.
			* You should use libswresample or libavfilter to convert the frame
			* to packed data. */
			fwrite(frame->extended_data[0], 1, unpadded_linesize, pOutputAudio);
		}
	}

	return FFMIN(ret,packet.size);
}


static int open_codec_context(enum AVMediaType type)
{
	int ret = 0;

	/*
	根据type找到对应的音视频流或者字母流，返回的是对应文件流的index
	*/
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL,0);
	if ( ret < 0 )
	{
		printf("Error: find stream failed.\n");
		return -1;
	}
	else
	{
		st = fmt_ctx->streams[ret];
		dec_ctx = st->codec;
		//查找解码器相关
		dec = avcodec_find_decoder(dec_ctx->codec_id);
		if ( !dec )
		{
			printf("Error: cannot find decoder.\n");
			return -1;
		}

		/*
		用于初始化一个视音频编解码器的AVCodecContext
		*/
		if ( ret = avcodec_open2(dec_ctx, dec, NULL) < 0 )
		{
			printf("Error: cannot open decoder.\n");
			return -1;
		}
		
	}
	
	return 0;
}