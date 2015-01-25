all: hiredis/libhiredis.a

.PHONY: all

hiredis/libhiredis.a: hiredis
	@make -C hiredis static

hiredis:
	git clone https://github.com/redis/hiredis

clean-local:
	@make -C hiredis clean

.PHONY: clean
