#include "a32100/m32100.h"
QVector<double> m32100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
