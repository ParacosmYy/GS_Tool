#include "a9000/m9000.h"
QVector<double> m9000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
