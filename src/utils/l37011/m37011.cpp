#include "l37011/m37011.h"
QVector<double> m37011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
