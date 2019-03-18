Security Considerations
=======================

Any discussion about connected devices is incomplete without discussion
about the security considerations. Let us look at some of the security
considerations that should be taken into account.

Securing Remote Communication
-----------------------------

All communication with any entity outside of the device must be secured.
Instead of reinventing the wheel, we recommend using the standard TLS
for securing this communication. The ESP8266\_RTOS\_SDK supports
*mbedtls* that implements all the features of the TLS protocol.

All the code in the ESP-Jumpstart already includes this for remote
communication. This section is applicable for any other remote
connections that you wish to make from your firmware. You can skip to
the next section if you are not using any other remote connections.

CA Certificates
~~~~~~~~~~~~~~~

The TLS layer uses trusted CA certificates to validate that the remote
endpoint/server is really who it claims to be.

The *esp\_tls* API accepts a CA certificate for performing server
validation.

.. code:: c

            esp_tls_cfg_t cfg = {
                .cacert_pem_buf  = server_root_cert_pem_start,
                .cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
            };

            struct esp_tls *tls = esp_tls_conn_http_new("https://www.example.com", &cfg);

If this parameter is not present, then the server validation check is
skipped. It is strongly recommended that for all your TLS connections
you specify the trusted CA certificate that can be used for server
validation.

Obtaining CA Certificates
~~~~~~~~~~~~~~~~~~~~~~~~~

As can be seen from the code above, the trusted CA certificate that can
validate your server must be programmed into your firmware. You can
obtain the trusted CA certificates by using the following command:

.. code:: bash

    $ openssl s_client -showcerts -connect www.example.com:443 < /dev/null

This command prints out a list of certificates. The last certificate
from this list can be embedded in your device’s firmware. Please refer
to the Section :ref:`sec_embedding\_files` for embedding files in your
firmware.
