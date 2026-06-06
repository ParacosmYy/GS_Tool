#include "m25592/m25592.h"
QVector<double> m25592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
