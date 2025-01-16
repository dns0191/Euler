#include "Variable.h"

void update_reactor_state(double time, double* data) {
	double PRE_INSERT_RHO = 0.0;
	double k = 0.0;
	double PGEN = 0.0;
	double DELAY = 0.0;
	double PROMPT = 0.0;
	double deltaPOWER = 0.0;
	double deltaRHO = 0.0;
	std::map<int, double> deltaPRECURSOR;

	// 이전 반응도 저장
	PRE_INSERT_RHO = INSERT_RHO;

	// 제어봉 위치 업데이트
	std::tuple<double, double> POSITION = findRodPosition(time);
	FR_POSITION = std::get<0>(POSITION);
	CR_POSITION = std::get<1>(POSITION);

	// 현재 반응도 계산
	calculateInsertRho(time);
	deltaRHO = INSERT_RHO - PRE_INSERT_RHO;
	RHO += deltaRHO;

	k = 1 / (1 - RHO);
	PGEN = PN_LIFE / k;

	// 출력 변화율 계산
	for (const auto& pair : PRECURSOR) {
		DELAY += pair.second * LAMBDA[pair.first];
	}
	PROMPT = (RHO - BETA_EFF) * POWER / PGEN;
	deltaPOWER = PROMPT + DELAY;

	// 지발중성자군 변화율 계산
	for (const auto& pair : PRECURSOR) {
		deltaPRECURSOR[pair.first] = pair.second * -LAMBDA[pair.first] + BETA[pair.first] * POWER / PGEN;
	}

	// 변화량 적용
	POWER += deltaPOWER * T_INTERVAL;
	for (auto& pair : PRECURSOR) {
		pair.second += deltaPRECURSOR[pair.first] * T_INTERVAL;
	}

	auto it = INTERPOLATED_HISTORY.lower_bound(time);
    if (it != INTERPOLATED_HISTORY.begin() && (it == INTERPOLATED_HISTORY.end() || it->first != time)) {
        --it;
    }
    if (it != INTERPOLATED_HISTORY.end()) {
        data[1] = std::get<2>(it->second);
    } 

	// DATA 배열 업데이트
	data[0] = POWER;
	data[2] = FR_POSITION;
	data[3] = CR_POSITION;
	data[4] = RHO*1e5;
	data[5] = INSERT_RHO*1e5;
}