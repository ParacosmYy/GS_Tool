#include "f29705/m29705.h"
QVector<double> m29705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
