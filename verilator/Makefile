run:
	mkdir build && cmake -B build && pip3 install -r requirements.txt
	cd build && make -j`nproc` && ./FionaSim

clean:
	rm -rf build