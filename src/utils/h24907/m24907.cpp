#include "h24907/m24907.h"
QVector<double> m24907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
