#include "k21630/m21630.h"
QVector<double> m21630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
