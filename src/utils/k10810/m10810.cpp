#include "k10810/m10810.h"
QVector<double> m10810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
