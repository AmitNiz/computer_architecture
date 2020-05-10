

compiler := g++
flags := -std=c++11 -Wall -g
src_dir := src
src_files := $(shell ls $(src_dir))
obj_files := $(patsubst %.cpp,%.o, $(src_files))
bin := chacheSim

$(bin): $(obj_files)
	$(compiler) $(flags) $^ -o $@

$(obj_files): %.o: $(src_dir)/%.cpp
	$(compiler) $(flags) -c $^

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(bin)
