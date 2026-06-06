#include "m29972/m29972.h"
QVector<double> m29972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
