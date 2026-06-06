#include "a29100/m29100.h"
QVector<double> m29100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
