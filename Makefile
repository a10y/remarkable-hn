.PHONY: build build-docker deploy deploy-appload install-xovi-persistence smoke-test

build:
	scripts/build.sh

build-docker:
	scripts/build-docker.sh

deploy:
	scripts/deploy.sh

deploy-appload:
	scripts/deploy-appload.sh

install-xovi-persistence:
	scripts/install-xovi-persistence.sh

smoke-test:
	scripts/smoke-test.sh
