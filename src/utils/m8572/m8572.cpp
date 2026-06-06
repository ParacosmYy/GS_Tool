#include "m8572/m8572.h"
QVector<double> m8572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
