#include "a25580/m25580.h"
QVector<double> m25580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
