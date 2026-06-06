#include "l27011/m27011.h"
QVector<double> m27011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
