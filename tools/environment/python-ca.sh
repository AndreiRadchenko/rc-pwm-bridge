# Source this file in the shell used for ESP-IDF tooling on macOS.
# Python's bundled OpenSSL was built with a non-existent CA path.
if [ ! -r /etc/ssl/cert.pem ]; then
    printf '%s\n' 'Помилка: системний CA bundle /etc/ssl/cert.pem недоступний.' >&2
    return 1 2>/dev/null || exit 1
fi
export SSL_CERT_FILE=/etc/ssl/cert.pem
