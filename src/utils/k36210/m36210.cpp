#include "k36210/m36210.h"
QVector<double> m36210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
