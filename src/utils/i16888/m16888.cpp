#include "i16888/m16888.h"
QVector<double> m16888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
