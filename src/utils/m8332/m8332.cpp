#include "m8332/m8332.h"
QVector<double> m8332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
