#include "k10970/m10970.h"
QVector<double> m10970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
