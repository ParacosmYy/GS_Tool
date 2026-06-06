#include "s17578/m17578.h"
QVector<double> m17578::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
