#include "k25810/m25810.h"
QVector<double> m25810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
