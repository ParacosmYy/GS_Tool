#include "k9810/m9810.h"
QVector<double> m9810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
