#include "m15592/m15592.h"
QVector<double> m15592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
