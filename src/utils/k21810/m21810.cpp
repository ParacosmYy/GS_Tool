#include "k21810/m21810.h"
QVector<double> m21810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
