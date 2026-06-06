#include "k10230/m10230.h"
QVector<double> m10230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
