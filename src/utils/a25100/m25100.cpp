#include "a25100/m25100.h"
QVector<double> m25100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
