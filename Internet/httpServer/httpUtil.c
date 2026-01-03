/**
    @file	httpUtil.c
    @brief	HTTP Server Utilities
    @version 1.0
    @date	2014/07/15
    @par Revision
 			2014/07/15 - 1.0 Release
    @author
    \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpUtil.h"
#include "main.h"
#include "wizchip_conf.h"

uint8_t http_get_cgi_handler(uint8_t * uri_name, uint8_t * buf, uint32_t * file_len) {
    uint8_t ret = HTTP_OK;
    uint16_t len = 0;

    if (predefined_get_cgi_processor(uri_name, buf, &len)) {
        ;
    } else if (strcmp((const char *)uri_name, "example.cgi") == 0) {
        // To do

        ;
    } else {
        // CGI file not found
        ret = HTTP_FAILED;
    }

    if (ret)	{
        *file_len = len;
    }
    return ret;
}



uint8_t http_post_cgi_handler(uint8_t * uri_name, st_http_request * p_http_request, uint8_t * buf, uint32_t * file_len) {
    uint8_t ret = HTTP_OK;
    uint16_t len = 0;
    uint8_t val = 0;

    if (predefined_set_cgi_processor(uri_name, p_http_request->URI, buf, &len)) {
        ;
    } else if (strcmp((const char *)uri_name, "example.cgi") == 0) {
        // To do
        val = 1;
        len = sprintf((char *)buf, "%d", val);
    } else {
        // CGI file not found
        ret = HTTP_FAILED;
    }

    if (ret)	{
        *file_len = len;
    }
    return ret;
}


/* uint8_t http_post_cgi_handler(uint8_t * uri_name, 
                              st_http_request * p_http_request, 
                              uint8_t * buf, 
                              uint32_t * file_len)
{
    uint8_t ret = HTTP_OK;
    uint16_t len = 0;
    uint8_t val = 0;
    // 匹配你的路径，例如 /api 或 /post
    if (strcmp((char*)uri_name, "api") == 0 || strcmp((char*)uri_name, "post") == 0)
    {
        printf("Received POST to /%s\r\n", uri_name);

        len = sprintf((char*)buf,
            "{"
            "\"code\":0,"
            "\"msg\":\"success\","
            "\"data\":{"
            "\"received\":\"%s\","
            "\"title\":\"foo\","
            "\"body\":\"bar\","
            "\"userId\":1"
            "}"
            "}",
            uri_name
        );
    } else if (strcmp((const char *)uri_name, "example.cgi") == 0) {
        // To do
        val = 1;
        len = sprintf((char *)buf, "%d", val);
    }
    else
    {
        // 未匹配路径，返回 404 内容
        len = sprintf((char*)buf, "POST path not found");
        ret = HTTP_FAILED;  // 如果想返回 404，可以设为 FAILED
    }

    if (ret)	{
        *file_len = len;
    }
    return ret;

}
*/

extern wiz_NetInfo gWIZNETINFO;

uint8_t predefined_get_cgi_processor(uint8_t * uri_name, uint8_t * buf, uint16_t * len) {
	uint8_t ret = HTTP_OK;

    // only accept status.cgi
    if ( (strcmp((char*)uri_name, "status/") == 0) || (strcmp((char*)uri_name, "status.cgi") == 0) )
    {
        *len = sprintf((char*)buf,
			"{"
            "\"device\":\"APM32+W5500\","
            "\"uptime_ms\":%u,"
            "\"free_heap\":%u,"   // 如果有 heap 信息
            "\"ip\":\"%d.%d.%d.%d\""
            "}",
			HAL_GetTick(),
            0,  // 占位，可替换为实际值
            gWIZNETINFO.ip[0], gWIZNETINFO.ip[1],
            gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]
        );
        return ret;
    }

    // ??2:?? /config.cgi,??????
    if (strcmp((char*)uri_name, "config.cgi") == 0)
    {
        *len = sprintf((char*)buf,
            "{"
            "\"ip\": \"%d.%d.%d.%d\","
            "\"device_id\": \"DM1000-001\""
            "}",
            gWIZNETINFO.ip[0], gWIZNETINFO.ip[1],
            gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]
        );
        return ret;
    }

    // ??????,?? 0 ?????(??????????)
    return 0;
    ;
}

// extern uint8_t http_rx_buf[HTTP_RX_BUF_SIZE];
uint8_t predefined_set_cgi_processor(uint8_t * uri_name, uint8_t * uri, uint8_t * buf, uint16_t * len) {
	
	// uint8_t *body_ptr = get_http_rx_buffer() + 100;
	// uint16_t body_len = 600;           // 实际长度需解析
// 匹配你要处理的 POST 路径
    if (strcmp((char*)uri_name, "api.cgi") == 0 ||
        strcmp((char*)uri_name, "post.cgi") == 0 ||
        strcmp((char*)uri_name, "config.cgi") == 0)
    {
		
		
		
        // Print debug info
		LOG_DEBUG("Http Post request debug:\r\n");
        printf("POST CGI Handler triggered for: %s\r\n", uri_name);
        printf("Full URI: %s\r\n", uri);
		printf("end\r\n");
        // 注意：POST Body 数据通常在库的全局 rx_buf 中
        // 这里我们假设 Body 内容已经被库放入 buf 开头（常见情况）
        // 实际长度需要从其他方式获取，或通过调试观察

        // 示例：简单回显收到的数据（用于测试）
        // 假设 Body 长度已知或通过 Content-Length 头解析（库未提供时可估算）
        *len = sprintf((char*)buf,
            "{"
            "\"status\":\"success\","
            "\"message\":\"POST request processed\","
            "\"received_uri\":\"%s\","
            "\"method\":\"POST\""
            "}",
            uri_name
        );

        // TODO: 在这里添加真实业务逻辑
        // 例如：
        // - 解析 JSON 配置并保存到 Flash
        // - 控制继电器开/关
        // - 修改设备参数
        // - 返回实时电表数据

        // 示例：模拟处理成功
//        printf("Simulating POST data processing...\r\n");
//		printf("Received POST Body (%d bytes): %.*s\r\n", body_len, body_len, body_ptr);
        return HTTP_OK;  // 必须返回 HTTP_OK 表示已处理
    }

    // 未匹配任何路径，返回 0（库会返回 404）
    return 0;
    ;
}

// user define func 26/1/2
/* POST 请求 CGI 处理器（关键！处理你工具发送的 JSON） */
//uint8_t predefined_post_cgi_processor(uint8_t * uri_name,
//                                      st_http_request * p_http_request,
//                                      uint8_t * buf,
//                                      uint32_t * file_len)
//{
//    // 匹配你的 POST 路径（建议用 /api 或 /post.cgi）
//    if (strcmp((char*)uri_name, "api") == 0 || strcmp((char*)uri_name, "post.cgi") == 0)
//    {
//        uint32_t content_len = p_http_request->content_len;

//        // 打印接收到的数据（调试用）
//        printf("POST received on %s, length: %lu\r\n", uri_name, content_len);

//        if (content_len > 0 && content_len < 1024)  // 防止溢出
//        {
//            // 数据在 p_http_request->URI 之后，或库的 rx_buf 中
//            // 简单起见，直接用 sprintf 打印前 200 字节
//            printf("Body: %.*s\r\n", (int)content_len, (char*)buf);
//        }

//        // 生成成功响应 JSON
//        *file_len = sprintf((char*)buf,
//            "{"
//            "\"status\":\"success\","
//            "\"message\":\"POST data received\","
//            "\"received_length\":%lu,"
//            "\"echo_title\":\"foo\","
//            "\"echo_body\":\"bar\","
//            "\"echo_userId\":1"
//            "}",
//            content_len
//        );

//        return HTTP_OK;  // 必须返回 HTTP_OK 表示已处理
//    }

//    // 未匹配路径，返回 0 让库处理其他逻辑
//    return 0;
//}

