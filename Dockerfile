ARG IDF_IMAGE=espressif/idf:v5.3.5
FROM ${IDF_IMAGE}

WORKDIR /workspace/firmware

ENV IDF_TARGET=esp32s3
ENV HOME=/tmp

CMD ["idf.py", "--version"]
