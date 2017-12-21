src=main.cpp sentiment.cpp similarity.cpp
app=main

all: sim

cygwin: $(src)
	g++ $(src) -g -lpthread -L./lib -lNLPIR -I./include -DOS_LINUX  -o ./app/$(app)

sim : similarity.cpp
	g++ similarity.cpp -D _TEST_SIMILARITY -o ./sim

simv2: similarity_v2.cpp
	g++ similarity_v2.cpp -D _TEST_SIMILARITY -o ./sim2

simv3: similarity_v3.cpp
	g++ similarity_v3.cpp -D _TEST_SIMILARITY -o ./sim3

test : sim
	sh test/sim_test_1.sh; \
	sh test/sim_test_2.sh; \
	sh test/sim_test_3.sh

origin : sim
	sh test/sim_test_miller_origin.sh

v2: simv2
	sh test/sim_test_miller_v2.sh

v3: simv3
	sh test/sim_test_miller_v3.sh
