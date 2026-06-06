#include "d25843/m25843.h"
QVector<double> m25843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
