#include "f25825/m25825.h"
QVector<double> m25825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
