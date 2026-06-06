#include "l9011/m9011.h"
QVector<double> m9011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
