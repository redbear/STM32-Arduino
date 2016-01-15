#include "MDNS.h"

bool MDNS::setHostname(String hostname) {
    bool success = hostname.length() < MAX_LABEL_SIZE && isAlphaDigitHyphen(hostname);

    if (success) {
        labels[HOST_NAME] = new Label(hostname, LOCAL);

        success = labels[HOST_NAME]->getSize() == hostname.length();
    }

    return success;
}

bool MDNS::setService(String protocol, String service, uint16_t port, String instance) {
    bool success = protocol.length() < MAX_LABEL_SIZE - 1 && service.length() < MAX_LABEL_SIZE - 1 &&
        instance.length() < MAX_LABEL_SIZE && isAlphaDigitHyphen(protocol) && isAlphaDigitHyphen(service) && isNetUnicode(instance);

    Label * protoLabel;

    if (success) {
        protoLabel = new Label("_" + protocol, LOCAL);

        success = protoLabel->getSize() == protocol.length() + 1;
    }

    if (success) {
        labels[SERVICE_NAME] = new Label("_" + service, protoLabel);

        success = labels[SERVICE_NAME]->getSize() == service.length() + 1;
    }

    if (success) {
        labels[INSTANCE_NAME] = new Label(instance, labels[SERVICE_NAME], true);

        success = labels[INSTANCE_NAME]->getSize() == instance.length();
    }

    this->port = port;

    return success;
}

bool MDNS::addTXTEntry(String key, String value) {
    return txtData->addEntry(key, value);
}

bool MDNS::begin() {
    // Wait for WiFi to connect
    while (!WiFi.ready()) {
    }

    udp->begin(MDNS_PORT);
    udp->joinMulticast(IPAddress(224, 0, 0, 251));

    // TODO: Probing + announcing

    return true;
}

bool MDNS::processQueries() {
    uint16_t n = udp->parsePacket();

    if (n > 0) {
        buffer->read(udp);

        udp->flush();

        uint16_t responses = getResponses();

        buffer->clear();

        if (responses > 0) {
            writeResponses(responses);
        }

        if (buffer->available() > 0) 
        {
            //Serial.println("buffer->available()");
            udp->beginPacket(IPAddress(224, 0, 0, 251), MDNS_PORT);

            buffer->write(udp);

            udp->endPacket();
        }
    }

    return n > 0;
}

uint16_t MDNS::getResponses() {
    uint16_t responses = 0;

    if (querySet) {
        delete querySet;
    }

    querySet = new QuerySet();

    if (querySet->readHeader(buffer)) {
        //Serial.println("a");
        if (querySet->isQuery()) {
            uint8_t count = 0;
            //Serial.println("b");
            while (count++ < querySet->getQueryCount()) {
                QuerySet::Query query;
                //Serial.println("c");
                query.matchedName = matcher->match(buffer);
                query.name += matcher->getLastName();
                query.type = buffer->readUInt16();
                query.cls = buffer->readUInt16();

                //querySet->addEntry(query);

                if (query.matchedName >= 0) {
                    switch (query.matchedName) {
                        case HOST_NAME:
                            if (query.type == A_TYPE || query.type == ANY_TYPE) {
                                //Serial.println("1");
                                responses |= A_FLAG | ADDITIONAL(NSEC_HOST_FLAG);
                            } else if (query.type == AAAA_TYPE) {
                                responses |= NSEC_HOST_FLAG;
                                //Serial.println("2");
                            }
                            break;

                        case SERVICE_NAME:
                            if (query.type == PTR_TYPE || query.type == ANY_TYPE) {
                                responses |= PTR_FLAG | ADDITIONAL(SRV_FLAG) | ADDITIONAL(TXT_FLAG) | ADDITIONAL(A_FLAG);
                                //Serial.println("3");
                            }
                            break;

                        case INSTANCE_NAME:
                            if (query.type == SRV_TYPE) {
                                responses |= SRV_FLAG | ADDITIONAL(A_FLAG) | ADDITIONAL(NSEC_INSTANCE_FLAG);
                                //Serial.println("4");
                            } else if (query.type == TXT_TYPE) {
                                responses |= TXT_FLAG | ADDITIONAL(NSEC_INSTANCE_FLAG);
                                //Serial.println("5");
                            } else if (query.type == ANY_TYPE) {
                                responses |= SRV_FLAG | TXT_FLAG | ADDITIONAL(A_FLAG) | NSEC_INSTANCE_FLAG;
                                //Serial.println("6");
                            }
                            break;

                        default:
                            break;
                    }
                } else if (query.matchedName == BUFFER_UNDERFLOW) {
                    querySet->setStatus("query " + String(count) + " buffer underflow");
                    count = querySet->getQueryCount();
                    //Serial.println("7");
                }
            }
        }
    } else {
        querySet->setStatus("header buffer underflow");
        //Serial.println("8");
    }

    querySet->setResponses(responses);

    return responses;
}

void MDNS::writeResponses(uint16_t responses) {
    // Reset output offsets for name compression
    for (uint8_t i = 0; i < NAME_COUNT; i++) {
        labels[i]->reset();
    }

    // Don't send additional responses where we are already sending answers
    responses &= ~(responses << FLAG_COUNT);

    uint8_t answerCount = count(responses);
    uint8_t additionalCount = count(responses >> FLAG_COUNT);

    buffer->writeUInt16(0x0);
    buffer->writeUInt16(0x8400);
    buffer->writeUInt16(0x0);
    buffer->writeUInt16(answerCount);
    buffer->writeUInt16(0x0);
    buffer->writeUInt16(additionalCount);

    while (responses > 0) {
        if ((responses & A_FLAG) != 0) {
            writeARecord();
        }
        if ((responses & PTR_FLAG) != 0) {
            writePTRRecord();
        }
        if ((responses & SRV_FLAG) != 0) {
            writeSRVRecord();
        }
        if ((responses & TXT_FLAG) != 0) {
            writeTXTRecord();
        }
        if ((responses & NSEC_HOST_FLAG) != 0) {
            writeNSECHostRecord();
        }
        if ((responses & NSEC_INSTANCE_FLAG) != 0) {
            writeNSECInstanceRecord();
        }

        responses >>= FLAG_COUNT;
    }
}

void MDNS::writeARecord() {
    writeRecord(HOST_NAME, A_TYPE, TTL_2MIN);
    buffer->writeUInt16(4);
    IPAddress ip = WiFi.localIP();
    for (int i = 0; i < IP_SIZE; i++) {
        buffer->writeUInt8(ip[i]);
    }
}

void MDNS::writePTRRecord() {
    writeRecord(SERVICE_NAME, PTR_TYPE, TTL_75MIN);
    buffer->writeUInt16(labels[INSTANCE_NAME]->getWriteSize());
    labels[INSTANCE_NAME]->write(buffer);
}

void MDNS::writeSRVRecord() {
    writeRecord(INSTANCE_NAME, SRV_TYPE, TTL_2MIN);
    buffer->writeUInt16(6 + labels[HOST_NAME]->getWriteSize());
    buffer->writeUInt16(0);
    buffer->writeUInt16(0);
    buffer->writeUInt16(port);
    labels[HOST_NAME]->write(buffer);
}

void MDNS::writeTXTRecord() {
    writeRecord(INSTANCE_NAME, TXT_TYPE, TTL_75MIN);
    txtData->write(buffer);
}

void MDNS::writeNSECHostRecord() {
    writeRecord(HOST_NAME, NSEC_TYPE, TTL_2MIN);
    buffer->writeUInt16(5);
    labels[HOST_NAME]->write(buffer);
    buffer->writeUInt8(0);
    buffer->writeUInt8(1);
    buffer->writeUInt8(0x40);
}

void MDNS::writeNSECInstanceRecord() {
    writeRecord(INSTANCE_NAME, NSEC_TYPE, TTL_2MIN);
    buffer->writeUInt16(9);
    labels[INSTANCE_NAME]->write(buffer);
    buffer->writeUInt8(0);
    buffer->writeUInt8(5);
    buffer->writeUInt8(0);
    buffer->writeUInt8(0);
    buffer->writeUInt8(0x80);
    buffer->writeUInt8(0);
    buffer->writeUInt8(0x40);
}

void MDNS::writeRecord(uint8_t nameIndex, uint16_t type, uint32_t ttl) {
    labels[nameIndex]->write(buffer);
    buffer->writeUInt16(type);
    buffer->writeUInt16(IN_CLASS);
    buffer->writeUInt32(ttl);
}

bool MDNS::isAlphaDigitHyphen(String string) {
    bool result = true;

    uint8_t idx = 0;

    while (result && idx < string.length()) {
        uint8_t c = string.charAt(idx++);

        result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-';
    }

    return result;
}

bool MDNS::isNetUnicode(String string) {
    bool result = true;

    uint8_t idx = 0;

    while (result && idx < string.length()) {
        uint8_t c = string.charAt(idx++);

        result = c >= 0x1f && c != 0x7f;
    }

    return result;
}

uint8_t MDNS::count(uint16_t bits) {
    uint8_t count = 0;

    for (uint8_t i = 0; i < FLAG_COUNT; i++) {
        count += bits & 1;

        bits >>= 1;
    }

    return count;
}

QuerySet * MDNS::getQuerySet() {
    return querySet;
}
