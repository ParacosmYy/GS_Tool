#include "a17280/m17280.h"
QVector<double> m17280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
