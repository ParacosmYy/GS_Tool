#include "i7888/m7888.h"
QVector<double> m7888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
