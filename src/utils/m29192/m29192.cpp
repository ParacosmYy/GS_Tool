#include "m29192/m29192.h"
QVector<double> m29192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
