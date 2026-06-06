#include "m24452/m24452.h"
QVector<double> m24452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
