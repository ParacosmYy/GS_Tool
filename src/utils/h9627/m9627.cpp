#include "h9627/m9627.h"
QVector<double> m9627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
