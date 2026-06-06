#include "d8843/m8843.h"
QVector<double> m8843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
