#include "h32907/m32907.h"
QVector<double> m32907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
