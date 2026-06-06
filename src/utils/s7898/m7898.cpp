#include "s7898/m7898.h"
QVector<double> m7898::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
