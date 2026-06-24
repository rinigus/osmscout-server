FROM alpine
RUN apk --update add openssl wget && rm -rf /var/cache/apk/*
COPY entrypoint.sh /app/entrypoint.sh
ENTRYPOINT ["/app/entrypoint.sh"]
