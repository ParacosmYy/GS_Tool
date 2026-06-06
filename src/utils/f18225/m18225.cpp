#include "f18225/m18225.h"
QVector<double> m18225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
