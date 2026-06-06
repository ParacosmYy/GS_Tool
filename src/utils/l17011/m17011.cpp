#include "l17011/m17011.h"
QVector<double> m17011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
