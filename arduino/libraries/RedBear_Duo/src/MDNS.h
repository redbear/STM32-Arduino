#include "Arduino.h"

#ifndef _INCL_MDNS
#define _INCL_MDNS

#include "Buffer.h"
#include "Label.h"
#include "QuerySet.h"
#include "TxtData.h"

#define MDNS_PORT 5353

#define BUFFER_SIZE 512
#define IP_SIZE 4

#define HOST_NAME 0
#define SERVICE_NAME 1
#define INSTANCE_NAME 2
#define NAME_COUNT 3

#define A_TYPE 0x01
#define PTR_TYPE 0x0c
#define TXT_TYPE 0x10
#define AAAA_TYPE 0x1c
#define SRV_TYPE 0x21
#define NSEC_TYPE 0x2f
#define ANY_TYPE 0xFF

#define IN_CLASS 1

#define TTL_2MIN 120
#define TTL_75MIN 4500

#define A_FLAG 0x01
#define PTR_FLAG 0x02
#define SRV_FLAG 0x04
#define TXT_FLAG 0x08
#define NSEC_HOST_FLAG 0x10
#define NSEC_INSTANCE_FLAG 0x20

#define FLAG_COUNT 6

#define ADDITIONAL(a) a << 6

class MDNS {
    public:

        bool setHostname(String hostname);

        bool setService(String protocol, String service, uint16_t port, String instance);

        bool addTXTEntry(String key, String value);

        bool begin();

        bool processQueries();

        QuerySet * getQuerySet();

    private:

        UDP * udp = new UDP();
        Buffer * buffer = new Buffer(BUFFER_SIZE);

        QuerySet * querySet;

        Label * ROOT = new Label("");
        Label * LOCAL = new Label("local", ROOT);
        Label * labels[NAME_COUNT];
        Label::Matcher * matcher = new Label::Matcher(labels, NAME_COUNT);

        uint16_t port;
        TxtData * txtData = new TxtData();

        uint16_t getResponses();
        void writeResponses(uint16_t responses);
        void writeARecord();
        void writePTRRecord();
        void writeSRVRecord();
        void writeTXTRecord();
        void writeNSECHostRecord();
        void writeNSECInstanceRecord();
        void writeRecord(uint8_t nameIndex, uint16_t type, uint32_t ttl);
        bool isAlphaDigitHyphen(String string);
        bool isNetUnicode(String string);
        uint8_t count(uint16_t bits);
};

#endif
