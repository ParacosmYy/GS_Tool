#include "i8748/m8748.h"
QVector<double> m8748::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
