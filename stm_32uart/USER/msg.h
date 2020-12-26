#ifndef __MSG_H
#define __MSG_H

    /**********************????,0x10??*************************/
    #define CMD_NO_AUTH                            0x111
    #define CMD_PING                               0x1001
    #define CMD_PONG                               0x1000
    #define CMD_DATA_UPLOAD_REQ                    0x1010
    #define CMD_DATA_UPLOAD_RSP                    0x1011
    #define CMD_TIME_OUT                           0x2000
    /**********************Admin??,0x11??*************************/
    #define CMD_ADMIN_LOGIN_REQ                    0x1100
    #define CMD_ADMIN_LOGIN_RSP                    0x1101
    /**********************Client??,0x12??*************************/
    #define CMD_CLIENT_LOGIN_REQ                   0x1200
    #define CMD_CLIENT_LOGIN_RSP                   0x1201
    #define CMD_CLIENT_VOICE_REQ                   0x1202
    #define CMD_CLIENT_VOICE_RSP                   0x1203
    #define CMD_CLIENT_SESSION_OUTTIME_PUSH        0x1291
    /**********************worker??,0x13??*************************/
    #define CMD_WORKER_LOGIN_REQ                   0x1300
    #define CMD_WORKER_LOGIN_RSP                   0x1301
    #define CMD_WORKER_GET_HOTELS_REQ              0x1302
    #define CMD_WORKER_GET_HOTELS_RSP              0x1303
    #define CMD_WORKER_GET_ROOMS_REQ               0x1304
    #define CMD_WORKER_GET_ROOMS_RSP               0x1305
    #define CMD_WORKER_ROOM_ACTION_REQ             0x1306
    #define CMD_WORKER_ROOM_ACTION_RSP             0x1307
    #define CMD_WORKER_SYNC_DEVICE_2_NETBRIDGE     0x1310  //?????????,???????????????,?????????
    #define CMD_WORKER_SYNC_DEVICE_2_NETBRIDGE_RSP 0x1311
    /**********************Room??,0x15??*************************/
    #define CMD_ROOM_LOGIN_REQ                     0x1502
    #define CMD_ROOM_LOGIN_RSP                     0x1501
    #define CMD_ROOM_SYNC_DEVICE_2_NETBRIDGE       0x1503
    #define CMD_ROOM_SYNC_DEVICE_2_NETBRIDGE_RSP   0x1504
    #define CMD_ROOM_DEVICE_SYNC_PING              0x1505
    #define CMD_ROOM_DEVICE_SYNC_PONG              0x1506
    #define CMD_DEVICE_PUSH                        0x1599

#endif


