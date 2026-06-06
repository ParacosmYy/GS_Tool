#include "m27012/m27012.h"
QVector<double> m27012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
