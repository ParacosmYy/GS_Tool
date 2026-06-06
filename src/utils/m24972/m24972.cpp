#include "m24972/m24972.h"
QVector<double> m24972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
