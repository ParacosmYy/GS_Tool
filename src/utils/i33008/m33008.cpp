#include "i33008/m33008.h"
QVector<double> m33008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
