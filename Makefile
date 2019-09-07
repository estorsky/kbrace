all:
	-ctags -R .
	cd server; make all
	cd client; make all

clean:
	rm -f tags
	cd server; make clean
	cd client; make clean

