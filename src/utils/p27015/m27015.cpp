#include "p27015/m27015.h"
QVector<double> m27015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
