#include "m19972/m19972.h"
QVector<double> m19972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
