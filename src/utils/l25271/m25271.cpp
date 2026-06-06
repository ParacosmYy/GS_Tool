#include "l25271/m25271.h"
QVector<double> m25271::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
