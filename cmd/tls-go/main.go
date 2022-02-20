package main

import (
	"crypto/tls"
	"log"
	"net/http"
)

func HelloServer(w http.ResponseWriter, req *http.Request) {
	w.Header().Set("Content-Type", "text/plain")
	w.Write([]byte("This is an example server.\n"))
}

func secureTLSConfig() *tls.Config {
	config := new(tls.Config)
	config.MinVersion = tls.VersionTLS11
	config.PreferServerCipherSuites = true
	config.CipherSuites = []uint16{
		tls.TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
		tls.TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
		tls.TLS_RSA_WITH_AES_128_CBC_SHA,
		tls.TLS_RSA_WITH_AES_256_CBC_SHA,
		tls.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
		tls.TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305,
		tls.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,

		tls.TLS_RSA_WITH_AES_128_GCM_SHA256,
		tls.TLS_RSA_WITH_AES_256_GCM_SHA384,

		tls.TLS_AES_128_GCM_SHA256,
		tls.TLS_AES_256_GCM_SHA384,
		tls.TLS_CHACHA20_POLY1305_SHA256,
	}
	return config
}

func main() {
	// mux
	mux := http.NewServeMux()
	mux.HandleFunc("/hello", HelloServer)

	srv := &http.Server{
		Addr:         ":443",
		Handler:      mux,
		TLSConfig:    secureTLSConfig(),
		TLSNextProto: make(map[string]func(*http.Server, *tls.Conn, http.Handler), 0),
	}

	err := srv.ListenAndServeTLS("server.crt", "server.key")
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
