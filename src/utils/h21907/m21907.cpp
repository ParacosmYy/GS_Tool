#include "h21907/m21907.h"
QVector<double> m21907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
