src=main.cpp sentiment.cpp similarity.cpp
app=main

all: sim

cygwin: $(src)
	g++ $(src) -g -lpthread -L./lib -lNLPIR -I./include -DOS_LINUX  -o ./app/$(app)

sim : similarity.cpp
	g++ similarity.cpp -D _TEST_SIMILARITY -o ./sim

simv2: similarity_v2.cpp
	g++ similarity_v2.cpp -D _TEST_SIMILARITY -o ./sim2

simv3: syn_dic.cpp
	g++ syn_dic.cpp -D _TEST_SYN_DIC -o ./syn_dic

origin : sim
	sh test/sim_test_miller_origin.sh

v2: simv2
	sh test/sim_test_miller_v2.sh

v3: simv3
	sh test/sim_test_miller_v3.sh

csv_v1: sim
	./sim

csv_v2: simv2
	./sim2

csv_syn_dic: syn_dic
	./syn_dic
