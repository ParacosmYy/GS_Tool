#include "m7972/m7972.h"
QVector<double> m7972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
