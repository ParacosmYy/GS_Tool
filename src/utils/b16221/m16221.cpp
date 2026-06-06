#include "b16221/m16221.h"
QVector<double> m16221::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
