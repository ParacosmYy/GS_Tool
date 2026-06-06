#include "h7907/m7907.h"
QVector<double> m7907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
