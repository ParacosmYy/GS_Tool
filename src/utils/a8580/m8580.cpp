#include "a8580/m8580.h"
QVector<double> m8580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
