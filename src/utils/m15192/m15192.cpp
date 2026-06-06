#include "m15192/m15192.h"
QVector<double> m15192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
