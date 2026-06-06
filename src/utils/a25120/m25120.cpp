#include "a25120/m25120.h"
QVector<double> m25120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
