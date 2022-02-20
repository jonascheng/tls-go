.DEFAULT_GOAL := help

APPLICATION?=tls-go
COMMIT_SHA?=$(shell git rev-parse --short HEAD)
DOCKER?=docker
REGISTRY?=jonascheng
# is Windows_NT on XP, 2000, 7, Vista, 10...
ifeq ($(OS),Windows_NT)
GOOS?=windows
RACE=""
else
GOOS?=$(shell uname -s | awk '{print tolower($0)}')
GORACE="-race"
endif

.PHONY: setup
setup: ## setup go modules
	go mod tidy

.PHONY: clean
clean: ## cleans the binary
	go clean
	rm -rf ./bin
	rm -rf server.*

.PHONY: run
run: setup server-key ## runs go run the application
	go run ${GORACE} cmd/${APPLICATION}/main.go

.PHONY: test
test:
	## runs test after 'make run'
	curl --cacert server.crt https://localhost:443/hello

.PHONY: build
build: clean ## build the application
	GOOS=${GOOS} GOARCH=amd64 go build ${GORACE} -a -v -ldflags="-w -s" -o bin/${APPLICATION} cmd/${APPLICATION}/main.go

.PHONY: server-key
server-key:
	## Key considerations for algorithm RSA ≥ 1024-bit
	if [ ! -f server.key ]; then openssl genrsa -out server.key 1024; fi;
	## Generation of self-signed(x509) public key (PEM-encodings .pem|.crt) based on the private (.key)
	if [ ! -f server.crt ]; then openssl req -new -x509 -key server.key -out server.crt -days 3650 -subj "/C=TW/ST=Test/L=Test/O=Test/OU=Test/CN=localhost/emailAddress=Test@email"; fi;

.PHONY: help
help: ## prints this help message
	@echo "Usage: \n"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
