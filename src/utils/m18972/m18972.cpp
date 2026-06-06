#include "m18972/m18972.h"
QVector<double> m18972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
