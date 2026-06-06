#include "i26868/m26868.h"
QVector<double> m26868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
