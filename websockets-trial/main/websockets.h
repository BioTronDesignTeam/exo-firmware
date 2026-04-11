#pragma once

void websocket_init(const char* uri);
void websocket_send(const char* message);
void websocket_stop();