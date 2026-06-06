#include "k18810/m18810.h"
QVector<double> m18810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
