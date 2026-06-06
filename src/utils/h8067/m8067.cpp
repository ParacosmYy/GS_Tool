#include "h8067/m8067.h"
QVector<double> m8067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
