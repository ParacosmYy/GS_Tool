#include "h16247/m16247.h"
QVector<double> m16247::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
