#include "a28100/m28100.h"
QVector<double> m28100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
