#include "a16160/m16160.h"
QVector<double> m16160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
